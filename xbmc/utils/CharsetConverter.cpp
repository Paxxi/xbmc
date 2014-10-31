/*
 *      Copyright (C) 2005-2013 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "CharsetConverter.h"
#include "Util.h"
#include "utils/StringUtils.h"
#include <fribidi/fribidi.h>
#include "LangInfo.h"
#include "guilib/LocalizeStrings.h"
#include "settings/lib/Setting.h"
#include "settings/Settings.h"
#include "threads/SingleLock.h"
#include "utils/Utf8Utils.h"
#include "log.h"

#include <errno.h>
#include <iconv.h>
#include <unicode/ucnv.h>
#include <unicode/urename.h>
#include <unicode/ustring.h>
#include <memory>

#if !defined(TARGET_WINDOWS) && defined(HAVE_CONFIG_H)
  #include "config.h"
#endif

#ifdef WORDS_BIGENDIAN
  #define ENDIAN_SUFFIX "BE"
#else
  #define ENDIAN_SUFFIX "LE"
#endif

#if defined(TARGET_DARWIN)
  #define WCHAR_IS_UCS_4 1
  #define UTF16_CHARSET "UTF-16" ENDIAN_SUFFIX
  #define UTF32_CHARSET "UTF-32" ENDIAN_SUFFIX
  #define UTF8_SOURCE "UTF-8-MAC"
  #define WCHAR_CHARSET UTF32_CHARSET
#elif defined(TARGET_WINDOWS)
  #define WCHAR_IS_UTF16 1
  #define UTF16_CHARSET "UTF-16" ENDIAN_SUFFIX
  #define UTF32_CHARSET "UTF-32" ENDIAN_SUFFIX
  #define UTF8_SOURCE "UTF-8"
  #define WCHAR_CHARSET UTF16_CHARSET 
  #pragma comment(lib, "libfribidi.lib")
  #pragma comment(lib, "icuuc.lib")
#elif defined(TARGET_ANDROID)
  #define WCHAR_IS_UCS_4 1
  #define UTF16_CHARSET "UTF-16" ENDIAN_SUFFIX
  #define UTF32_CHARSET "UTF-32" ENDIAN_SUFFIX
  #define UTF8_SOURCE "UTF-8"
  #define WCHAR_CHARSET UTF32_CHARSET 
#else
  #define UTF16_CHARSET "UTF-16" ENDIAN_SUFFIX
  #define UTF32_CHARSET "UTF-32" ENDIAN_SUFFIX
  #define UTF8_SOURCE "UTF-8"
  #define WCHAR_CHARSET "WCHAR_T"
  #if __STDC_ISO_10646__
    #ifdef SIZEOF_WCHAR_T
      #if SIZEOF_WCHAR_T == 4
        #define WCHAR_IS_UCS_4 1
      #elif SIZEOF_WCHAR_T == 2
        #define WCHAR_IS_UCS_2 1
      #endif
    #endif
  #endif
#endif

/* We don't want to pollute header file with many additional includes and definitions, so put 
   here all staff that require usage of types defined in this file or in additional headers */
class CCharsetConverter::CInnerConverter
{
public:
  static bool logicalToVisualBiDi(const std::u32string& stringSrc, std::u32string& stringDst, FriBidiCharType base = FRIBIDI_TYPE_LTR, const bool failOnBadString = false);

  template<class INPUT,class OUTPUT>
  static bool customConvert(const std::string& sourceCharset, const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar = false);

  template<class INPUT, class OUTPUT>
  static bool toUtf16(const std::string& sourceCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/);

  template<class INPUT, class OUTPUT>
  static bool fromUtf16(const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/);
};

/* single symbol sizes in chars */
const int CCharsetConverter::m_Utf8CharMinSize = 1;
const int CCharsetConverter::m_Utf8CharMaxSize = 4;


class UConverterGuard
{
  UConverter* converter;
public:
  UConverterGuard() : converter(nullptr) {};
  UConverterGuard(UConverter* cnv) : converter(cnv) {};
  ~UConverterGuard() 
{
    if (converter)
      ucnv_close(converter);
}
  void Set(UConverter* cnv) { converter = cnv; }
  operator UConverter*() const
{
    return this->converter;
}
};

template<class INPUT,class OUTPUT>
bool CCharsetConverter::CInnerConverter::customConvert(const std::string& sourceCharset, const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/)
{
  strDest.clear();
  if (strSource.empty())
    return true;

  UErrorCode err = U_ZERO_ERROR;
  UConverterGuard srcConv;
  UConverterGuard dstConv;

  int32_t srcLength = -1;
  int32_t tempBufferLength = -1;
  int32_t dstLength = -1;

  const char* srcBuffer = NULL;
  char* dstBuffer = NULL;
  UChar* tempBuffer = NULL;
  /*std::unique_ptr<char[]> dstBuffer;
  std::unique_ptr<UChar[]> tempBuffer;*/

  //if any of the charsets are empty we fall back to the default
  //encoder
  if (sourceCharset.empty())
    srcConv.Set(ucnv_open(NULL, &err));
  else
    srcConv.Set(ucnv_open(sourceCharset.c_str(), &err));
    
  if (U_FAILURE(err))
    return false;

  if (failOnInvalidChar)
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
  else
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);

  if (targetCharset.empty())
    dstConv.Set(ucnv_open(NULL, &err));
  else
    dstConv.Set(ucnv_open(targetCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;

  if (failOnInvalidChar)
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
  else
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);

  //input to ucnv_toUChars is a char* pointer so multiply the length by
  //the minimum size of a char
  srcLength = strSource.length() * ucnv_getMinCharSize(srcConv);
  srcBuffer = (const char*)strSource.c_str();

  tempBufferLength = UCNV_GET_MAX_BYTES_FOR_STRING(srcLength, ucnv_getMaxCharSize(srcConv));
  tempBuffer = new UChar[tempBufferLength];

  int res = ucnv_toUChars(srcConv, tempBuffer, tempBufferLength, srcBuffer, -1, &err);

  if (U_FAILURE(err))
    return false;

  assert(U_SUCCESS(err));

  dstLength = UCNV_GET_MAX_BYTES_FOR_STRING(res, ucnv_getMaxCharSize(dstConv));
  dstBuffer = new char[dstLength];

  res = ucnv_fromUChars(dstConv, dstBuffer, dstLength, tempBuffer, res, &err);

  if (U_FAILURE(err))
    return false;

  res /= ucnv_getMinCharSize(dstConv);

  uint16_t bom = (uint16_t)dstBuffer[0];
  //check for a utf-16 or utf-32 bom
  if (bom == 65535 || bom == 65279)
{
    dstBuffer += 2;
    --res;
  }

  strDest.assign(reinterpret_cast<OUTPUT::value_type *>(dstBuffer), res);

    return true;
}

template<class INPUT,class OUTPUT>
bool CCharsetConverter::CInnerConverter::toUtf16(const std::string& sourceCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/)
{
  strDest.clear();
  if (strSource.empty())
    return true;

  UErrorCode err = U_ZERO_ERROR;
  UConverterGuard srcConv;

  int32_t srcLength = -1;
  int32_t dstLength = -1;

  const char* srcBuffer = NULL;
  std::unique_ptr<UChar[]> dstBuffer;

  //if any of the charsets are empty we fall back to the default
  //encoder
  if (sourceCharset.empty())
    srcConv.Set(ucnv_open(NULL, &err));
  else
    srcConv.Set(ucnv_open(sourceCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;
    
  if (failOnInvalidChar)
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
  else
  ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);

  //input to ucnv_toUChars is a char* pointer so multiply the length by
  //the minimum size of a char
  srcLength = strSource.length() * ucnv_getMinCharSize(srcConv);
  srcBuffer = (const char*)strSource.c_str();

  dstLength = UCNV_GET_MAX_BYTES_FOR_STRING(srcLength, ucnv_getMaxCharSize(srcConv));
  dstBuffer = std::make_unique<UChar[]>(dstLength);

  int res = ucnv_toUChars(srcConv, dstBuffer.get(), dstLength, srcBuffer, srcLength, &err);

  if (U_FAILURE(err))
    return false;

  res /= ucnv_getMinCharSize(srcConv);


  strDest = reinterpret_cast<OUTPUT::value_type *>(dstBuffer.get());

  return true;
}

template<class INPUT,class OUTPUT>
bool CCharsetConverter::CInnerConverter::fromUtf16(const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/)
  {
  strDest.clear();
  if (strSource.empty())
    return true;

  UErrorCode err = U_ZERO_ERROR;
  UConverterGuard dstConv;

  int32_t srcLength = -1;
  int32_t dstLength = -1;

  const UChar* srcBuffer = NULL;
  std::unique_ptr<char[]> dstBuffer;

  //if any of the charsets are empty we fall back to the default
  //encoder
  if (targetCharset.empty())
    dstConv.Set(ucnv_open(NULL, &err));
  else
    dstConv.Set(ucnv_open(targetCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;

        if (failOnInvalidChar)
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
  else
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);

  srcLength = strSource.length();
  srcBuffer = (UChar*)strSource.c_str();

  dstLength = UCNV_GET_MAX_BYTES_FOR_STRING(srcLength, ucnv_getMaxCharSize(dstConv));
  dstBuffer = std::make_unique<char[]>(dstLength);

  int res = ucnv_fromUChars(dstConv, dstBuffer.get(), dstLength, srcBuffer, srcLength, &err);

  if (U_FAILURE(err))
    return false;

  res /= ucnv_getMinCharSize(dstConv);

  strDest = reinterpret_cast<OUTPUT::value_type *>(dstBuffer.get());

  return true;
}

bool CCharsetConverter::CInnerConverter::logicalToVisualBiDi(const std::u32string& stringSrc, std::u32string& stringDst, FriBidiCharType base /*= FRIBIDI_TYPE_LTR*/, const bool failOnBadString /*= false*/)
{
  stringDst.clear();

  const size_t srcLen = stringSrc.length();
  if (srcLen == 0)
    return true;

  stringDst.reserve(srcLen);
  size_t lineStart = 0;

  // libfribidi is not threadsafe, so make sure we make it so
  do
  {
    size_t lineEnd = stringSrc.find('\n', lineStart);
    if (lineEnd >= srcLen) // equal to 'lineEnd == std::string::npos'
      lineEnd = srcLen;
    else
      lineEnd++; // include '\n'
    
    const size_t lineLen = lineEnd - lineStart;

    FriBidiChar* visual = (FriBidiChar*) malloc((lineLen + 1) * sizeof(FriBidiChar));
    if (visual == NULL)
    {
      free(visual);
      CLog::Log(LOGSEVERE, "%s: can't allocate memory", __FUNCTION__);
      return false;
    }

    bool bidiFailed = false;
    FriBidiCharType baseCopy = base; // preserve same value for all lines, required because fribidi_log2vis will modify parameter value
    if (fribidi_log2vis((const FriBidiChar*)(stringSrc.c_str() + lineStart), lineLen, &baseCopy, visual, NULL, NULL, NULL))
    {
      // Removes bidirectional marks
      const int newLen = fribidi_remove_bidi_marks(visual, lineLen, NULL, NULL, NULL);
      if (newLen > 0)
        stringDst.append((const char32_t*)visual, (size_t)newLen);
      else if (newLen < 0)
        bidiFailed = failOnBadString;
    }
    else
      bidiFailed = failOnBadString;

    free(visual);

    if (bidiFailed)
      return false;

    lineStart = lineEnd;
  } while (lineStart < srcLen);

  return !stringDst.empty();
}


static struct SCharsetMapping
{
  const char* charset;
  const char* caption;
} g_charsets[] = {
  { "ISO-8859-1", "Western Europe (ISO)" }
  , { "ISO-8859-2", "Central Europe (ISO)" }
  , { "ISO-8859-3", "South Europe (ISO)" }
  , { "ISO-8859-4", "Baltic (ISO)" }
  , { "ISO-8859-5", "Cyrillic (ISO)" }
  , { "ISO-8859-6", "Arabic (ISO)" }
  , { "ISO-8859-7", "Greek (ISO)" }
  , { "ISO-8859-8", "Hebrew (ISO)" }
  , { "ISO-8859-9", "Turkish (ISO)" }
  , { "CP1250", "Central Europe (Windows)" }
  , { "CP1251", "Cyrillic (Windows)" }
  , { "CP1252", "Western Europe (Windows)" }
  , { "CP1253", "Greek (Windows)" }
  , { "CP1254", "Turkish (Windows)" }
  , { "CP1255", "Hebrew (Windows)" }
  , { "CP1256", "Arabic (Windows)" }
  , { "CP1257", "Baltic (Windows)" }
  , { "CP1258", "Vietnamesse (Windows)" }
  , { "CP874", "Thai (Windows)" }
  , { "BIG5", "Chinese Traditional (Big5)" }
  , { "GBK", "Chinese Simplified (GBK)" }
  , { "SHIFT_JIS", "Japanese (Shift-JIS)" }
  , { "CP949", "Korean" }
  , { "BIG5-HKSCS", "Hong Kong (Big5-HKSCS)" }
  , { NULL, NULL }
};


CCharsetConverter::CCharsetConverter()
{
}

void CCharsetConverter::OnSettingChanged(const CSetting* setting)
{
  if (setting == NULL)
    return;

  const std::string& settingId = setting->GetId();
  if (settingId == "locale.charset")
    resetUserCharset();
  else if (settingId == "subtitles.charset")
    resetSubtitleCharset();
  else if (settingId == "karaoke.charset")
    resetKaraokeCharset();
}

void CCharsetConverter::clear()
{
}

std::vector<std::string> CCharsetConverter::getCharsetLabels()
{
  std::vector<std::string> lab;
  for(SCharsetMapping* c = g_charsets; c->charset; c++)
    lab.push_back(c->caption);

  return lab;
}

std::string CCharsetConverter::getCharsetLabelByName(const std::string& charsetName)
{
  for(SCharsetMapping* c = g_charsets; c->charset; c++)
  {
    if (StringUtils::EqualsNoCase(charsetName,c->charset))
      return c->caption;
  }

  return "";
}

std::string CCharsetConverter::getCharsetNameByLabel(const std::string& charsetLabel)
{
  for(SCharsetMapping* c = g_charsets; c->charset; c++)
  {
    if (StringUtils::EqualsNoCase(charsetLabel, c->caption))
      return c->charset;
  }

  return "";
}

void CCharsetConverter::reset(void)
{
}

void CCharsetConverter::resetSystemCharset(void)
{
}

void CCharsetConverter::resetUserCharset(void)
{
}

void CCharsetConverter::resetSubtitleCharset(void)
{
}

void CCharsetConverter::resetKaraokeCharset(void)
{
}

void CCharsetConverter::reinitCharsetsFromSettings(void)
{
}

bool CCharsetConverter::utf8ToUtf32(const std::string& utf8StringSrc, std::u32string& utf32StringDst, bool failOnBadChar /*= true*/)
{
  return CInnerConverter::customConvert("utf-8", "utf-32", utf8StringSrc, utf32StringDst, failOnBadChar);
}

std::u32string CCharsetConverter::utf8ToUtf32(const std::string& utf8StringSrc, bool failOnBadChar /*= true*/)
{
  std::u32string converted;
  utf8ToUtf32(utf8StringSrc, converted, failOnBadChar);
  return converted;
}

bool CCharsetConverter::utf8ToUtf16(const std::string& utf8StringSrc, std::u16string& utf16StringDst, bool failOnBadChar /*= true*/)
{
  return CInnerConverter::toUtf16("utf-8", utf8StringSrc, utf16StringDst, failOnBadChar);
}

std::u16string CCharsetConverter::utf8ToUtf16(const std::string& utf8StringSrc, bool failOnBadChar /*= true*/)
{
  std::u16string converted;
  utf8ToUtf16(utf8StringSrc, converted, failOnBadChar);
  return converted;
}

bool CCharsetConverter::utf8ToUtf32Visual(const std::string& utf8StringSrc, std::u32string& utf32StringDst, bool bVisualBiDiFlip /*= false*/, bool forceLTRReadingOrder /*= false*/, bool failOnBadChar /*= false*/)
{
 /* if (bVisualBiDiFlip)
  {
    std::u32string converted;
    if (!CInnerConverter::stdConvert(Utf8ToUtf32, utf8StringSrc, converted, failOnBadChar))
      return false;

    return CInnerConverter::logicalToVisualBiDi(converted, utf32StringDst, forceLTRReadingOrder ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_PDF, failOnBadChar);
  }
  return CInnerConverter::stdConvert(Utf8ToUtf32, utf8StringSrc, utf32StringDst, failOnBadChar);*/
  return CInnerConverter::customConvert("utf-8", "utf-32", utf8StringSrc, utf32StringDst, failOnBadChar);
}

bool CCharsetConverter::utf32ToUtf8(const std::u32string& utf32StringSrc, std::string& utf8StringDst, bool failOnBadChar /*= true*/)
{
  return CInnerConverter::customConvert("utf-32", "utf-8", utf32StringSrc, utf8StringDst, failOnBadChar);
}

std::string CCharsetConverter::utf32ToUtf8(const std::u32string& utf32StringSrc, bool failOnBadChar /*= false*/)
{
  std::string converted;
  utf32ToUtf8(utf32StringSrc, converted, failOnBadChar);
  return converted;
}

bool CCharsetConverter::utf32logicalToVisualBiDi(const std::u32string& logicalStringSrc, std::u32string& visualStringDst, bool forceLTRReadingOrder /*= false*/, bool failOnBadString /*= false*/)
{
  return CInnerConverter::logicalToVisualBiDi(logicalStringSrc, visualStringDst, forceLTRReadingOrder ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_PDF, failOnBadString);
}

// The bVisualBiDiFlip forces a flip of characters for hebrew/arabic languages, only set to false if the flipping
// of the string is already made or the string is not displayed in the GUI
bool CCharsetConverter::utf8ToW(const std::string& utf8StringSrc, std::wstring& wStringDst, bool bVisualBiDiFlip /*= true*/, 
                                bool forceLTRReadingOrder /*= false*/, bool failOnBadChar /*= false*/)
{
  // Try to flip hebrew/arabic characters, if any
  if (bVisualBiDiFlip)
  {
    wStringDst.clear();
    std::u32string utf32str;
    if (!CInnerConverter::customConvert(UTF8_SOURCE, "UTF-32", utf8StringSrc, utf32str, failOnBadChar))
      return false;

    std::u32string utf32flipped;
    const bool bidiResult = CInnerConverter::logicalToVisualBiDi(utf32str, utf32flipped, forceLTRReadingOrder ? FRIBIDI_TYPE_LTR : FRIBIDI_TYPE_PDF, failOnBadChar);

    return CInnerConverter::customConvert("UTF-32", WCHAR_CHARSET, utf32flipped, wStringDst, failOnBadChar) && bidiResult;
  }
  
  return CInnerConverter::toUtf16(UTF8_SOURCE, utf8StringSrc, wStringDst, failOnBadChar);
}

bool CCharsetConverter::subtitleCharsetToUtf8(const std::string& stringSrc, std::string& utf8StringDst)
{
  std::string subtitleCharset = g_langInfo.GetSubtitleCharSet();
  return CInnerConverter::customConvert(subtitleCharset, UTF8_SOURCE, stringSrc, utf8StringDst, false);
}

bool CCharsetConverter::utf8ToStringCharset(const std::string& utf8StringSrc, std::string& stringDst)
{
  std::string guiCharset = g_langInfo.GetGuiCharSet();
  return CInnerConverter::customConvert(UTF8_SOURCE, guiCharset, utf8StringSrc, stringDst, false);
}

bool CCharsetConverter::utf8ToStringCharset(std::string& stringSrcDst)
{
  std::string strSrc(stringSrcDst);
  return utf8ToStringCharset(strSrc, stringSrcDst);
}

bool CCharsetConverter::ToUtf8(const std::string& strSourceCharset, const std::string& stringSrc, std::string& utf8StringDst, bool failOnBadChar /*= false*/)
{
  if (strSourceCharset == "UTF-8")
  { // simple case - no conversion necessary
    utf8StringDst = stringSrc;
    return true;
  }
  
  return CInnerConverter::customConvert(strSourceCharset, "UTF-8", stringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::string& stringDst)
{
  if (strDestCharset == "UTF-8")
  { // simple case - no conversion necessary
    stringDst = utf8StringSrc;
    return true;
  }

  return CInnerConverter::customConvert(UTF8_SOURCE, strDestCharset, utf8StringSrc, stringDst);
}

bool CCharsetConverter::utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::u16string& utf16StringDst)
{
  return CInnerConverter::customConvert(UTF8_SOURCE, strDestCharset, utf8StringSrc, utf16StringDst);
}

bool CCharsetConverter::utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::u32string& utf32StringDst)
{
  return CInnerConverter::customConvert(UTF8_SOURCE, strDestCharset, utf8StringSrc, utf32StringDst);
}

bool CCharsetConverter::unknownToUTF8(std::string& stringSrcDst)
{
  std::string source(stringSrcDst);
  return unknownToUTF8(source, stringSrcDst);
}

bool CCharsetConverter::unknownToUTF8(const std::string& stringSrc, std::string& utf8StringDst, bool failOnBadChar /*= false*/)
{
  // checks whether it's utf8 already, and if not converts using the sourceCharset if given, else the string charset
  if (CUtf8Utils::isValidUtf8(stringSrc))
  {
    utf8StringDst = stringSrc;
    return true;
  }
  std::string guiCharset = g_langInfo.GetGuiCharSet();
  return CInnerConverter::customConvert(guiCharset, UTF8_SOURCE, stringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::wToUTF8(const std::wstring& wStringSrc, std::string& utf8StringDst, bool failOnBadChar /*= false*/)
{
  //return CInnerConverter::fromUtf16("utf-8", wStringSrc, utf8StringDst, failOnBadChar);
  return CInnerConverter::customConvert(UTF16_CHARSET, UTF8_SOURCE, wStringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::utf16BEtoUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst)
{
  return CInnerConverter::customConvert("utf-16be", "utf-8", utf16StringSrc, utf8StringDst, false);
}

bool CCharsetConverter::ucs2ToUTF8(const std::u16string& ucs2StringSrc, std::string& utf8StringDst)
{
  //UCS2 is technically only BE and ICU includes no LE version converter
  //Use UTF-16 converter since the only difference are lead bytes that are
  //illegal to use in UCS2 so should not cause any issues
  return CInnerConverter::customConvert("UTF-16LE", UTF8_SOURCE, ucs2StringSrc, utf8StringDst, false);
}


bool CCharsetConverter::utf8ToSystem(std::string& stringSrcDst, bool failOnBadChar /*= false*/)
{
  std::string strSrc(stringSrcDst);
  return CInnerConverter::customConvert("utf-8", "", strSrc, stringSrcDst, failOnBadChar);
}

bool CCharsetConverter::systemToUtf8(const std::string& sysStringSrc, std::string& utf8StringDst, bool failOnBadChar /*= false*/)
{
  return CInnerConverter::customConvert("", "utf-8", sysStringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::utf8logicalToVisualBiDi(const std::string& utf8StringSrc, std::string& utf8StringDst, bool failOnBadString /*= false*/)
{
  utf8StringDst.clear();
  std::u32string utf32flipped;
  if (!utf8ToUtf32Visual(utf8StringSrc, utf32flipped, true, true, failOnBadString))
    return false;

  return CInnerConverter::customConvert("UTF-32", UTF8_SOURCE, utf32flipped, utf8StringDst, failOnBadString);
}

void CCharsetConverter::SettingOptionsCharsetsFiller(const CSetting* setting, std::vector< std::pair<std::string, std::string> >& list, std::string& current, void *data)
{
  std::vector<std::string> vecCharsets = g_charsetConverter.getCharsetLabels();
  sort(vecCharsets.begin(), vecCharsets.end(), sortstringbyname());

  list.push_back(make_pair(g_localizeStrings.Get(13278), "DEFAULT")); // "Default"
  for (int i = 0; i < (int) vecCharsets.size(); ++i)
    list.push_back(make_pair(vecCharsets[i], g_charsetConverter.getCharsetNameByLabel(vecCharsets[i])));
}
