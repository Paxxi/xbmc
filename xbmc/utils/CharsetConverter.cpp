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
#include "LangInfo.h"
#include "utils/Utf8Utils.h"

#include <unicode/ucnv.h>
#include <unicode/urename.h>
#include <unicode/ubidi.h>
#include <unicode/uniset.h>
#include <unicode/normalizer2.h>

#if !defined(TARGET_WINDOWS) && defined(HAVE_CONFIG_H)
  #include "config.h"
#endif

#define UTF8_CHARSET "UTF-8"
#define UTF16_CHARSET "UTF16_PlatformEndian"
#define UTF16LE_CHARSET "UTF-16LE"
#define UTF16BE_CHARSET "UTF-16BE"
#define UTF32_CHARSET "UTF32_PlatformEndian"
#define UTF32LE_CHARSET "UTF-32LE"
#define UTF32BE_CHARSET "UTF-32BE"

#if defined(TARGET_DARWIN)

#elif defined(TARGET_WINDOWS)
  #define WCHAR_CHARSET UTF16_CHARSET 
#ifdef NDEBUG
  #pragma comment(lib, "icuuc.lib")
#else
  #pragma comment(lib, "icuucd.lib")
#endif
#elif defined(TARGET_ANDROID)

#else

#endif

/* We don't want to pollute header file with many additional includes and definitions, so put 
   here all stuff that require usage of types defined in this file or in additional headers */
class CCharsetConverter::CInnerConverter
{
private:
  static bool InternalBidiHelper(const UChar* srcBuffer, int32_t srcLength, 
                                 UChar** dstBuffer, int32_t& dstLength,
                                 const uint16_t bidiOptions);
public:

  template<class INPUT, class OUTPUT>
  static bool LogicalToVisualBiDi(const std::string& sourceCharset, const std::string& targetCharset,
                                  const INPUT& strSource, OUTPUT& strDest,
                                  const uint16_t bidiOptions, const bool failOnBadString = false);

  template<class INPUT,class OUTPUT>
  static bool Convert(const std::string& sourceCharset, const std::string& targetCharset,
                            const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar = false);

  static bool NormalizeSystemSafe(std::u16string& strSrc);
};


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
bool CCharsetConverter::CInnerConverter::Convert(const std::string& sourceCharset, const std::string& targetCharset, const INPUT& strSource, OUTPUT& strDest, bool failOnInvalidChar /*= false*/)
{
  strDest.clear();
  if (strSource.empty())
    return true;

  UErrorCode err = U_ZERO_ERROR;
  UConverterGuard srcConv;
  UConverterGuard dstConv;

  int32_t srcLength = -1;
  int32_t srcLengthInBytes = -1;
  int32_t dstLength = -1;
  int32_t dstLengthInBytes = -1;

  const char* srcBuffer = NULL;
  char* dstBuffer = NULL;

  //ucnv_ConvertEx moves the pointer to the end of the buffer
  //use this for input to ucnv_ConvertEx
  char* dstBufferWalker = NULL;

  //if any of the charsets are empty we fall back to the default
  //encoder
  if (sourceCharset.empty())
    srcConv.Set(ucnv_open(NULL, &err));
  else
    srcConv.Set(ucnv_open(sourceCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;

  if (targetCharset.empty())
    dstConv.Set(ucnv_open(NULL, &err));
  else
    dstConv.Set(ucnv_open(targetCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;

  if (failOnInvalidChar)
  {
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
  }
  else
  {
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);
  }

  if (U_FAILURE(err))
    return false;

  srcLength = strSource.length();
  srcLengthInBytes = strSource.length() * ucnv_getMinCharSize(srcConv);
  srcBuffer = (const char*)strSource.c_str();

  //Point of no return, remember to free dstBuffer :)
  dstLengthInBytes = UCNV_GET_MAX_BYTES_FOR_STRING(srcLength, ucnv_getMaxCharSize(dstConv));
  dstBuffer = new char[dstLengthInBytes];
  dstBufferWalker = dstBuffer;

  ucnv_convertEx(dstConv, srcConv, &dstBufferWalker, dstBufferWalker + dstLengthInBytes,
    &srcBuffer, srcBuffer + srcLengthInBytes, NULL, NULL, NULL, NULL, FALSE, TRUE, &err);
  
  assert(U_SUCCESS(err));

  if (U_SUCCESS(err))
  {
    ptrdiff_t res = (dstBufferWalker - dstBuffer) / ucnv_getMinCharSize(dstConv);
    dstBufferWalker = dstBuffer;

    uint16_t bom = (uint16_t)dstBuffer[0];
    //check for a bom, ICU writes it when converting to utf16 or 32
    if (bom == 65535 || bom == 65279)
    {
      dstBufferWalker += 2;
      --res;
    }

    strDest.assign(reinterpret_cast<OUTPUT::value_type *>(dstBufferWalker), res);
  }
  
  delete[] dstBuffer;

  return true;
}

template<class INPUT, class OUTPUT>
bool CCharsetConverter::CInnerConverter::LogicalToVisualBiDi(const std::string& sourceCharset, const std::string& targetCharset,
                                                            const INPUT& strSource, OUTPUT& strDest,
                                                            const uint16_t bidiOptions, const bool failOnBadString)
{
  strDest.clear();
  if (strSource.empty())
    return true;

  UErrorCode err = U_ZERO_ERROR;
  UConverterGuard srcConv;
  UConverterGuard dstConv;

  int32_t srcLength = -1;
  int32_t srcLengthInBytes = -1;
  int32_t dstLength = -1;
  int32_t dstLengthInBytes = -1;

  const char* srcBuffer = NULL;
  char* dstBuffer = NULL;
  UChar* conversionBuffer = NULL;
  UChar* bidiResultBuffer = NULL;

  //if any of the charsets are empty we fall back to the default
  //encoder
  if (sourceCharset.empty())
    srcConv.Set(ucnv_open(NULL, &err));
  else
    srcConv.Set(ucnv_open(sourceCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;

  if (targetCharset.empty())
    dstConv.Set(ucnv_open(NULL, &err));
  else
    dstConv.Set(ucnv_open(targetCharset.c_str(), &err));

  if (U_FAILURE(err))
    return false;

  if (failOnBadString)
  {
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_STOP, NULL, NULL, NULL, &err);
  }
  else
  {
    ucnv_setFromUCallBack(dstConv, UCNV_FROM_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);
    ucnv_setToUCallBack(srcConv, UCNV_TO_U_CALLBACK_SKIP, NULL, NULL, NULL, &err);
  }

  if (U_FAILURE(err))
    return false;

  srcLength = strSource.length();
  srcLengthInBytes = strSource.length() * ucnv_getMinCharSize(srcConv);
  srcBuffer = (const char*)strSource.c_str();

  //Point of no return, remember to free dstBuffer :)
  dstLengthInBytes = UCNV_GET_MAX_BYTES_FOR_STRING(srcLength, ucnv_getMaxCharSize(srcConv));
  dstLength = dstLengthInBytes / 2; //we know UChar is always 2 bytes
  conversionBuffer = new UChar[dstLength];

  int res = ucnv_toUChars(srcConv, conversionBuffer, dstLength, srcBuffer, srcLengthInBytes, &err);

  assert(U_SUCCESS(err));

  if (U_FAILURE(err))
  {
    delete[] conversionBuffer;
    return false;
  }

  if (!CInnerConverter::InternalBidiHelper(conversionBuffer, res, &bidiResultBuffer, res, bidiOptions))
  {
    delete[] conversionBuffer;
    return false;
  }

  dstLengthInBytes = UCNV_GET_MAX_BYTES_FOR_STRING(res, ucnv_getMaxCharSize(dstConv));
  dstBuffer = new char[dstLengthInBytes];

  res = ucnv_fromUChars(dstConv, dstBuffer, dstLengthInBytes, bidiResultBuffer, res, &err);

  if (U_FAILURE(err))
  {
    delete[] conversionBuffer;
    delete[] bidiResultBuffer;
    delete[] dstBuffer;
    return false;
  }

  res /= ucnv_getMinCharSize(dstConv);

  if (U_SUCCESS(err))
  {
    uint16_t bom = (uint16_t)dstBuffer[0];
    //check for a utf-16 or utf-32 bom
    if (bom == 65535 || bom == 65279)
      strDest.assign(reinterpret_cast<OUTPUT::value_type *>(dstBuffer + 2), res - 1);
    else
      strDest.assign(reinterpret_cast<OUTPUT::value_type *>(dstBuffer), res);
  }

  delete[] conversionBuffer;
  delete[] bidiResultBuffer;
  delete[] dstBuffer;

  return true;
}

bool CCharsetConverter::CInnerConverter::InternalBidiHelper(const UChar* srcBuffer, int32_t srcLength,
                                                            UChar** dstBuffer, int32_t& dstLength,
                                                            const uint16_t bidiOptions)
{
  bool result = false;
  int32_t outputLength = -1;
  int32_t requiredLength = -1;
  UErrorCode err = U_ZERO_ERROR;
  UBiDiLevel level = 0;
  UBiDi* bidiConv = ubidi_open();
  
  //ubidi_setPara changes the srcBuffer pointer so make it const
  //and get a local copy of it
  UChar* inputBuffer = const_cast<UChar*>(srcBuffer);
  UChar* outputBuffer = NULL;

  if (bidiOptions & BiDiOptions::LTR)
    level = UBIDI_DEFAULT_LTR;
  else if (bidiOptions & BiDiOptions::RTL)
    level = UBIDI_DEFAULT_RTL;
  else
    level = UBIDI_DEFAULT_LTR;

  ubidi_setPara(bidiConv, inputBuffer, srcLength, level, NULL, &err);

  if (U_SUCCESS(err))
  {
    outputLength = ubidi_getProcessedLength(bidiConv) + 1; //allow for null terminator
    outputBuffer = new UChar[outputLength];

    do
    {
      uint16_t options = UBIDI_DO_MIRRORING;
      if (bidiOptions & BiDiOptions::REMOVE_CONTROLS)
        options |= UBIDI_REMOVE_BIDI_CONTROLS;
      if (bidiOptions & BiDiOptions::WRITE_REVERSE)
        options |= UBIDI_OUTPUT_REVERSE;

      requiredLength = ubidi_writeReordered(bidiConv, outputBuffer, outputLength, options, &err);

      if (U_SUCCESS(err))
      {
        outputLength = requiredLength;
        break;
      }

      if (err == U_BUFFER_OVERFLOW_ERROR)
      {
        delete[] outputBuffer;
        outputBuffer = new UChar[requiredLength];
        outputLength = requiredLength;

        //make sure our err is reset, some icu functions fail if it contains
        //a previous bad result
        err = U_ZERO_ERROR;
      }

      //We can't do much for other failures than buffer overflow
      //clean up and bail out
      if (U_FAILURE(err))
      {
        delete[] outputBuffer;
        return false;
      }
    } while (1);

    *dstBuffer = outputBuffer;
    dstLength = outputLength;
    result = true;
  }

  ubidi_close(bidiConv);
  
  //we don't delete outBuffer, it's returned to caller and
  //caller is responsible for freeing it
  return result;
}

bool CCharsetConverter::CInnerConverter::NormalizeSystemSafe(std::u16string& strSrc)
{
  UErrorCode err = U_ZERO_ERROR;
  
  //https://developer.apple.com/library/mac/qa/qa1173/_index.html
  //which U + 2000 through U + 2FFF, U + F900 through U + FAFF, and U + 2F800 through U + 2FAFF
  //are not decomposed(this avoids problems with round trip conversions from old Mac text encodings).
  UnicodeString usetString("[^[\\u2000-\\u2fff][\\uf900-\\ufaff][\\u2f800-\\u2faff]]");
  UnicodeSet uSet(usetString, err);
  
  if (U_FAILURE(err))
    return false;

  UnicodeString src((UChar*)strSrc.c_str());
  UnicodeString dst;

  const Normalizer2* nfd = Normalizer2::getNFDInstance(err);

  if (U_FAILURE(err))
    return false;

  FilteredNormalizer2 filteredNfd(*nfd, uSet);

  filteredNfd.normalize(src, dst, err);

  if (U_FAILURE(err))
    return false;

  strSrc.assign((char16_t*)dst.getBuffer(), dst.length());

  return true;
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

bool CCharsetConverter::utf8ToUtf32(const std::string& utf8StringSrc, std::u32string& utf32StringDst, bool failOnBadChar /*= true*/)
{
  return CInnerConverter::Convert(UTF8_CHARSET, UTF32_CHARSET, utf8StringSrc, utf32StringDst, failOnBadChar);
}

std::u32string CCharsetConverter::utf8ToUtf32(const std::string& utf8StringSrc, bool failOnBadChar /*= true*/)
{
  std::u32string converted;
  utf8ToUtf32(utf8StringSrc, converted, failOnBadChar);
  return converted;
}

bool CCharsetConverter::utf8ToUtf16(const std::string& utf8StringSrc, std::u16string& utf16StringDst, bool failOnBadChar /*= true*/)
{
  return CInnerConverter::Convert(UTF8_CHARSET, UTF16_CHARSET, utf8StringSrc, utf16StringDst, failOnBadChar);
}

std::u16string CCharsetConverter::utf8ToUtf16(const std::string& utf8StringSrc, bool failOnBadChar /*= true*/)
{
  std::u16string converted;
  utf8ToUtf16(utf8StringSrc, converted, failOnBadChar);
  return converted;
}

bool CCharsetConverter::utf8ToUtf16BE(const std::string& utf8StringSrc, std::u16string& utf16StringDst, bool failOnBadChar /* = true */)
{
  return CInnerConverter::Convert(UTF8_CHARSET, UTF16BE_CHARSET, utf8StringSrc, utf16StringDst, failOnBadChar);
}

bool CCharsetConverter::utf8ToUtf16LE(const std::string& utf8StringSrc, std::u16string& utf16StringDst, bool failOnBadChar /* = true */)
{
  return CInnerConverter::Convert(UTF8_CHARSET, UTF16LE_CHARSET, utf8StringSrc, utf16StringDst, failOnBadChar);
}

bool CCharsetConverter::utf8ToUtf32Visual(const std::string& utf8StringSrc, std::u32string& utf32StringDst, bool bVisualBiDiFlip /*= false*/, bool forceLTRReadingOrder /*= false*/, bool failOnBadChar /*= false*/)
{
  if (bVisualBiDiFlip)
  {
    return CInnerConverter::LogicalToVisualBiDi(UTF8_CHARSET, UTF32_CHARSET, utf8StringSrc, utf32StringDst, forceLTRReadingOrder ? BiDiOptions::LTR : BiDiOptions::RTL, failOnBadChar);
  }
  return CInnerConverter::Convert(UTF8_CHARSET, UTF32_CHARSET, utf8StringSrc, utf32StringDst, failOnBadChar);
}

bool CCharsetConverter::utf32ToUtf8(const std::u32string& utf32StringSrc, std::string& utf8StringDst, bool failOnBadChar /*= true*/)
{
  return CInnerConverter::Convert(UTF32_CHARSET, UTF8_CHARSET, utf32StringSrc, utf8StringDst, failOnBadChar);
}

std::string CCharsetConverter::utf32ToUtf8(const std::u32string& utf32StringSrc, bool failOnBadChar /*= false*/)
{
  std::string converted;
  utf32ToUtf8(utf32StringSrc, converted, failOnBadChar);
  return converted;
}

// The bVisualBiDiFlip forces a flip of characters for hebrew/arabic languages, only set to false if the flipping
// of the string is already made or the string is not displayed in the GUI
bool CCharsetConverter::utf8ToWLogicalToVisual(const std::string& utf8StringSrc, std::wstring& wStringDst, bool bVisualBiDiFlip /*= true*/, 
                                bool forceLTRReadingOrder /*= false*/, bool failOnBadChar /*= false*/)
{
  // Try to flip hebrew/arabic characters, if any
  if (bVisualBiDiFlip)
  {
    return CInnerConverter::LogicalToVisualBiDi(UTF8_CHARSET, WCHAR_CHARSET, utf8StringSrc, wStringDst, forceLTRReadingOrder ? BiDiOptions::LTR : BiDiOptions::RTL, failOnBadChar);
  }
  
  return CInnerConverter::Convert(UTF8_CHARSET, UTF16_CHARSET, utf8StringSrc, wStringDst, failOnBadChar);
}

bool CCharsetConverter::utf8ToW(const std::string& utf8StringSrc, std::wstring& wStringDst, bool failOnBadChar /* = false */)
{
  return CInnerConverter::Convert(UTF8_CHARSET, WCHAR_CHARSET, utf8StringSrc, wStringDst, failOnBadChar);
}

bool CCharsetConverter::subtitleCharsetToUtf8(const std::string& stringSrc, std::string& utf8StringDst)
{
  std::string subtitleCharset = g_langInfo.GetSubtitleCharSet();
  return CInnerConverter::Convert(subtitleCharset, UTF8_CHARSET, stringSrc, utf8StringDst, false);
}

bool CCharsetConverter::utf8ToStringCharset(const std::string& utf8StringSrc, std::string& stringDst)
{
  std::string guiCharset = g_langInfo.GetGuiCharSet();
  return CInnerConverter::Convert(UTF8_CHARSET, guiCharset, utf8StringSrc, stringDst, false);
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
  
  return CInnerConverter::Convert(strSourceCharset, UTF8_CHARSET, stringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::string& stringDst)
{
  if (strDestCharset == "UTF-8")
  { // simple case - no conversion necessary
    stringDst = utf8StringSrc;
    return true;
  }

  return CInnerConverter::Convert(UTF8_CHARSET, strDestCharset, utf8StringSrc, stringDst);
}

bool CCharsetConverter::utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::u16string& utf16StringDst)
{
  return CInnerConverter::Convert(UTF8_CHARSET, strDestCharset, utf8StringSrc, utf16StringDst);
}

bool CCharsetConverter::utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::u32string& utf32StringDst)
{
  return CInnerConverter::Convert(UTF8_CHARSET, strDestCharset, utf8StringSrc, utf32StringDst);
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
  return CInnerConverter::Convert(guiCharset, UTF8_CHARSET, stringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::wToUTF8(const std::wstring& wStringSrc, std::string& utf8StringDst, bool failOnBadChar /*= false*/)
{
  return CInnerConverter::Convert(WCHAR_CHARSET, UTF8_CHARSET, wStringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::utf16BEtoUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst)
{
  return CInnerConverter::Convert(UTF16BE_CHARSET, UTF8_CHARSET, utf16StringSrc, utf8StringDst, false);
}

bool CCharsetConverter::utf16LEtoUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst)
{
  return CInnerConverter::Convert(UTF16LE_CHARSET, UTF8_CHARSET, utf16StringSrc, utf8StringDst, false);
}

bool CCharsetConverter::utf16ToUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst)
{
  return CInnerConverter::Convert(UTF16_CHARSET, UTF8_CHARSET, utf16StringSrc, utf8StringDst, false);
}

bool CCharsetConverter::ucs2ToUTF8(const std::u16string& ucs2StringSrc, std::string& utf8StringDst)
{
  //UCS2 is technically only BE and ICU includes no LE version converter
  //Use UTF-16 converter since the only difference are lead bytes that are
  //illegal to use in UCS2 so should not cause any issues
  return CInnerConverter::Convert(UTF16LE_CHARSET, UTF8_CHARSET, ucs2StringSrc, utf8StringDst, false);
}


bool CCharsetConverter::utf8ToSystem(std::string& stringSrcDst, bool failOnBadChar /*= false*/)
{
  std::string strSrc(stringSrcDst);
  return CInnerConverter::Convert(UTF8_CHARSET, "", strSrc, stringSrcDst, failOnBadChar);
}

bool CCharsetConverter::systemToUtf8(const std::string& sysStringSrc, std::string& utf8StringDst, bool failOnBadChar /*= false*/)
{
  return CInnerConverter::Convert("", UTF8_CHARSET, sysStringSrc, utf8StringDst, failOnBadChar);
}

bool CCharsetConverter::logicalToVisualBiDi(const std::string& utf8StringSrc, std::string& utf8StringDst,
                                            uint16_t bidiOptions /* = BiDiOptions::LTR | BiDiOptions::REMOVE_CONTROLS */,
                                            bool failOnBadString /* = false */)
{
  return CInnerConverter::LogicalToVisualBiDi(UTF8_CHARSET, UTF8_CHARSET, utf8StringSrc, utf8StringDst, bidiOptions, failOnBadString);
}

bool CCharsetConverter::logicalToVisualBiDi(const std::u16string& utf16StringSrc, std::u16string& utf16StringDst,
                                            uint16_t bidiOptions /* = BiDiOptions::LTR | BiDiOptions::REMOVE_CONTROLS */,
                                            bool failOnBadString /* = false */)
{
  return CInnerConverter::LogicalToVisualBiDi(UTF16_CHARSET, UTF16_CHARSET, utf16StringSrc, utf16StringDst, bidiOptions, failOnBadString);
}

bool CCharsetConverter::logicalToVisualBiDi(const std::u32string& utf32StringSrc, std::u32string& utf32StringDst,
                                            uint16_t bidiOptions /* = BiDiOptions::LTR | BiDiOptions::REMOVE_CONTROLS */,
                                            bool failOnBadString /* = false */)
{
  return CInnerConverter::LogicalToVisualBiDi(UTF32_CHARSET, UTF32_CHARSET, utf32StringSrc, utf32StringDst, bidiOptions, failOnBadString);
}

bool CCharsetConverter::utf8ToSystemSafe(const std::string& stringSrc, std::string& stringDst)
{
  stringDst.clear();

  if (stringSrc.empty())
    return true;

  std::u16string buffer;
    
  if (!utf8ToUtf16(stringSrc, buffer, true))
    return false;

  if (!CInnerConverter::NormalizeSystemSafe(buffer))
    return false;

  if (!utf16LEtoUTF8(buffer, stringDst))
    return false;

  return true;
}

bool CCharsetConverter::utf8ToWSystemSafe(const std::string& stringSrc, std::wstring& stringDst)
{
  //W should be win32 only and requires no special handling, make sure we fail on bad chars
  //to avoid any weird behaviour
  return CInnerConverter::Convert(UTF8_CHARSET, WCHAR_CHARSET, stringSrc, stringDst, true);
}
