#pragma once
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

#include "utils/GlobalsHandling.h"
#include "utils/uXstrings.h"

#include <string>
#include <vector>

/**
 * \class CCharsetConverter
 *
 * Methods for converting text between different encodings
 *
 * There are a few conventions used in the method names that may be
 * beneficial to know about
 * Methods starting with Try* will try to convert the text and fail on any invalid byte sequence
 *
 * Regular methods will silently ignore invalid byte sequences and skip them,
 * should only fail on errors like out of memory or completely invalid input
 *
 * Methods ending with SystemSafe are meant to be used for file system access,
 * they provide additional checks to make sure that output is in a valid state
 * for the platform. They also fail on invalid byte sequences to avoid passing
 * an unkown filename to the system.
 * Currently the only platform with special needs are OSX and iOS which require
 * a special normalization form for it's file names.
 */
class CCharsetConverter
{
public:

  /** Options to specify BiDi text handling */
  enum BiDiOptions
  {
    LTR = 1,            /**< specifies that the text is mainly left-to-right */
    RTL = 2,            /**< specifies that the text is mainly right-to-left */
    WRITE_REVERSE = 4,  /**< specifies that the text should be reversed, e.g flip rtl text to ltr */
    REMOVE_CONTROLS = 8 /**< specifies that bidi control characters such as LRE, RLE, PDF should be removed from the output*/
  };

  /**
   * Convert UTF-8 string to UTF-32 string.
   *
   * \param[in]  utf8StringSrc       is source UTF-8 string to convert
   * \param[out] utf32StringDst      is output UTF-32 string, empty on any error
   *
   * \return true on successful conversion, false on any error
   */
  static bool utf8ToUtf32(const std::string& utf8StringSrc, std::u32string& utf32StringDst);
  
  /**
   * Convert UTF-8 string to UTF-32 string.
   * 
   * \param[in] utf8StringSrc       is source UTF-8 string to convert
   *
   * \return converted string on successful conversion, empty string on any error
   */
  static std::u32string utf8ToUtf32(const std::string& utf8StringSrc);

  /**
  * Convert UTF-8 string to UTF-16 string.
  * 
  * \param[in]  utf8StringSrc       is source UTF-8 string to convert
  * \param[out] utf16StringDst      is output UTF-16 string, empty on any error
  *
  * \return true on successful conversion, false on any error
  */
  static bool utf8ToUtf16(const std::string& utf8StringSrc, std::u16string& utf16StringDst);
  
  /**
  * Convert UTF-8 string to UTF-16 string.
  * 
  * \param[in] utf8StringSrc       is source UTF-8 string to convert
  *
  * \return converted string on successful conversion, empty string on any error
  */
  static std::u16string utf8ToUtf16(const std::string& utf8StringSrc);
  
  /**
   * Convert UTF-8 string to UTF-32 string and perform logical to visual processing
   * on the string.
   * Use it for readable text, GUI strings etc.
   *
   * \param[in]  utf8StringSrc        is source UTF-8 string to convert
   * \param[out] utf32StringDst       is output UTF-32 string, empty on any error
   * \param[in]  bidiOptions          options for logical to visual transformation
   *                                  default is LTR | REMOVE_CONTROLS which should be fine
   *                                  in most cases
   *
   * \return true on successful conversion, false on any error
   * \sa CCharsetConverter::BiDiOptions
   * \sa utf8ToUtf32
   */
  static bool utf8ToUtf32LogicalToVisual(const std::string& utf8StringSrc, std::u32string& utf32StringDst, 
                                         uint16_t bidiOptions = LTR | REMOVE_CONTROLS);
  
  /**
   * Convert UTF-32 string to UTF-8 string.
   * 
   * \param[in]  utf32StringSrc      is source UTF-32 string to convert
   * \param[out] utf8StringDst       is output UTF-8 string, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa utf32ToUtf8(std::u32string&)
   */
  static bool utf32ToUtf8(const std::u32string& utf32StringSrc, std::string& utf8StringDst);
  
  /**
   * Convert UTF-32 string to UTF-8 string.
   * 
   * \param[in] utf32StringSrc      is source UTF-32 string to convert
   *
   * \return converted string on successful conversion, empty string on any error
   * \sa utf32ToUtf8(std::u32string&, std::string&)
   */
  static std::string utf32ToUtf8(const std::u32string& utf32StringSrc);
  
  /**
   * Convert UTF-8 string to wide string and perform logical to visual processing
   * on the string.
   * Use it for readable text, GUI strings etc.
   *
   * \param[in]  utf8StringSrc        is source UTF-8 string to convert
   * \param[out] wStringDst           is output wide string, empty on any error
   * \param[in]  bidiOptions          options for logical to visual transformation
   *                                  default is LTR | REMOVE_CONTROLS which should be fine
   *                                  in most cases
   *
   * \return true on successful conversion, false on any error
   * \sa CCharsetConverter::BiDiOptions
   * \sa utf8ToW
   * \sa utf8ToUtf32LogicalToVisual
   */
  static bool utf8ToWLogicalToVisual(const std::string& utf8StringSrc, std::wstring& wStringDst,
                                     uint16_t bidiOptions = LTR | REMOVE_CONTROLS);

  /**
   * Convert UTF-8 string to wide string.
   *
   * \param[in]  utf8StringSrc       is source UTF-8 string to convert
   * \param[out] wStringDst          is output wide string, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa utf8ToWLogicalToVisual
   * \sa utf8ToWSystemSafe
   */
  static bool utf8ToW(const std::string& utf8StringSrc, std::wstring& wStringDst);

  /**
   * Convert UTF-8 string to wide string and perform extra processing
   * to ensure that the string is valid for file system operations.
   * Fails on invalid byte sequences.
   *
   * \param[in]  utf8StringSrc       is source UTF-8 string to convert
   * \param[out] wStringDst          is output wide string, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa utf8ToWLogicalToVisual
   * \sa utf8ToW
   */
  static bool utf8ToWSystemSafe(const std::string& stringSrc, std::wstring& stringDst);

  /**
   * Convert wide string to UTF-8 string.
   *
   * \param[in]  wStringSrc             is source UTF-8 string to convert
   * \param[out] utf8StringDst          is output wide string, empty on any error
   *
   * \return true on successful conversion, false on any error
   */
  static bool wToUTF8(const std::wstring& wStringSrc, std::string& utf8StringDst);

  /**
   * Convert from the user selected subtitle encoding to UTF-8
   *
   * \param[in]  stringSrc             is source UTF-8 string to convert
   * \param[out] utf8StringDst         is output wide string, empty on any error
   *
   * \return true on successful conversion, false on any error
   */
  static bool subtitleCharsetToUtf8(const std::string& stringSrc, std::string& utf8StringDst);

  /**
   * Convert UTF-8 string to the user selected GUI character set
   *
   * \param[in]  utf8StringSrc          is source UTF-8 string to convert
   * \param[out] stringDst              is output wide string, empty on any error
   *
   * \return true on successful conversion, false on any error
   */
  static bool utf8ToStringCharset(const std::string& utf8StringSrc, std::string& stringDst);

  /**
   * Convert UTF-8 string to the user selected GUI character set
   *
   * \param[in,out]  stringSrcDst       is source UTF-8 string to convert
   *                                    Undefined value of stringSrcDst if conversion fails
   *
   * \return true on successful conversion, false on any error
   */
  static bool utf8ToStringCharset(std::string& stringSrcDst);

  /**
   * Convert UTF-8 string to UTF-16 big endian string.
   *
   * \param[in]  utf8StringSrc          is source UTF-8 string to convert
   * \param[out] utf16StringDst         is output UTF-16 big endian, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa utf8ToUtf16LE
   * \sa utf8ToUtf16
   */
  static bool utf8ToUtf16BE(const std::string& utf8StringSrc, std::u16string& utf16StringDst);

  /**
   * Convert UTF-8 string to UTF-16 little endian string.
   * Only for special cases where endianness matters, prefer
   * utf8ToUtf16 for general use.
   *
   * \param[in]  utf8StringSrc          is source UTF-8 string to convert
   * \param[out] utf16StringDst         is output UTF-16 little endian, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa utf8ToUtf16BE
   * \sa utf8ToUtf16
   */
  static bool utf8ToUtf16LE(const std::string& utf8StringSrc, std::u16string& utf16StringDst);

  /**
   * Convert UTF-8 string to system encoding, likely UTF-8 on Linux
   * and Mac but can be just about anything
   *
   * \param[in,out]  utf8StringSrc          is source UTF-8 string to convert
   *                                        Undefined value on errors
   *
   * \return true on successful conversion, false on any error
   * \sa systemToUtf8
   * \sa utf8To
   */
  static bool utf8ToSystem(std::string& stringSrcDst);

  /**
   * Convert string in system encoding to UTF-8
   *
   * \param[in]  sysStringSrc          is source string in system encoding to convert
   * \param[out] utf8StringDst         is output UTF-8 string, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa utf8ToSystem
   * \sa utf8To
   * \sa TrySystemToUtf8
   */
  static bool systemToUtf8(const std::string& sysStringSrc, std::string& utf8StringDst);

  /**
   * Try to convert string in system encoding to UTF-8.
   * Will fail on invalid byte sequences.
   *
   * \param[in]  sysStringSrc         is source string in system encoding to convert
   * \param[out] utf8StringDst        is output UTF-8 string, empty on any error
   *
   * \return true on successful conversion, false on any error
   * \sa systemToUtf8
   * \sa utf8To
   */
  static bool TrySystemToUtf8(const std::string& sysStringSrc, std::string& utf8StringDst);

  /**
   * Convert UTF-8 string to specified 8-bit encoding
   *
   * \param[in]  strDestCharset         specify destination encoding
   *                                    e.g. US-ASCII or CP-1252
   * \param[in]  utf8StringSrc          is source UTF-8 string to convert
   * \param[out] stringDst              is output string in specified encoding
   *
   * \return true on successful conversion, false on any error
   * \sa utf8To(const std::string&, const std::string&, std::u16string&)
   * \sa utf8To(const std::string&, const std::string&, std::u32string&)
   */
  static bool utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::string& stringDst);

  /**
   * Convert UTF-8 string to specified 16-bit encoding
   *
   * \param[in]  strDestCharset         specify destination encoding
   *                                    e.g. UTF-16 or UTF-16LE
   * \param[in]  utf8StringSrc          is source UTF-8 string to convert
   * \param[out] utf16StringDst         is output string in specified encoding
   *
   * \return true on successful conversion, false on any error
   * \sa utf8To(const std::string&, const std::string&, std::string&)
   * \sa utf8To(const std::string&, const std::string&, std::u32string&)
   */
  static bool utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::u16string& utf16StringDst);

  /**
   * Convert UTF-8 string to specified 32-bit encoding
   *
   * \param[in]  strDestCharset         specify destination encoding
   *                                    e.g. UTF-32 or UTF-32LE
   * \param[in]  utf8StringSrc          is source UTF-8 string to convert
   * \param[out] utf32StringDst         is output string in specified encoding
   *
   * \return true on successful conversion, false on any error
   * \sa utf8To(const std::string&, const std::string&, std::string&)
   * \sa utf8To(const std::string&, const std::string&, std::u16string&)
   */
  static bool utf8To(const std::string& strDestCharset, const std::string& utf8StringSrc, std::u32string& utf32StringDst);

  /**
   * Convert specified 8-bit encoding to UTF-8
   *
   * \param[in]  strSourceCharset         specify source encoding
   *                                      e.g. US-ASCII or CP-1252
   * \param[in]  stringSrc                is source 8-bit string to convert
   * \param[out] utf8StringDst            is output string in UTF-8
   *
   * \return true on successful conversion, false on any error
   * \sa TryToUtf8
   */
  static bool ToUtf8(const std::string& strSourceCharset, const std::string& stringSrc, std::string& utf8StringDst);

  /**
   * Try to convert specified 8-bit encoding to UTF-8.
   * Fails on any bad byte sequence
   *
   * \param[in]  strSourceCharset         specify source encoding
   *                                      e.g. US-ASCII or CP-1252
   * \param[in]  stringSrc                is source 8-bit string to convert
   * \param[out] utf8StringDst            is output string in UTF-8
   *
   * \return true on successful conversion, false on any error
   * \sa ToUtf8
   */
  static bool TryToUtf8(const std::string& strSourceCharset, const std::string& stringSrc, std::string& utf8StringDst);

  /**
   * Convert UTF-16 big endian to UTF-8
   *
   * \param[in]  utf16StringSrc           is source UTF-16 big endian string to convert
   * \param[out] utf8StringDst            is output string in UTF-8
   *
   * \return true on successful conversion, false on any error
   * \sa utf16ToUTF8
   * \sa utf16LEtoUTF8
   */
  static bool utf16BEtoUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst);

  /**
  * Convert UTF-16 little endian to UTF-8
  *
  * \param[in]  utf16StringSrc           is source UTF-16 little endian string to convert
  * \param[out] utf8StringDst            is output string in UTF-8
  *
  * \return true on successful conversion, false on any error
  * \sa utf16ToUTF8
  * \sa utf16BEtoUTF8
  */
  static bool utf16LEtoUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst);

  /**
  * Convert UTF-16 to UTF-8
  *
  * \param[in]  utf16StringSrc           is source UTF-16 string to convert
  * \param[out] utf8StringDst            is output string in UTF-8
  *
  * \return true on successful conversion, false on any error
  * \sa utf16ToUTF8
  * \sa utf16LEtoUTF8
  */
  static bool utf16ToUTF8(const std::u16string& utf16StringSrc, std::string& utf8StringDst);

  /**
   * Convert UCS-2 to UTF-8
   * This is really another name for utf16LEtoUTF8, technically
   * UCS-2 is only big endian but our use case requires little endian
   * conversion
   *
   * \param[in]  utf16StringSrc           is source UCS-2 little endian string to convert
   * \param[out] utf8StringDst            is output string in UTF-8
   *
   * \return true on successful conversion, false on any error
   * \sa utf16ToUTF8
   * \sa utf16LEtoUTF8
   * \sa utf16BEtoUTF8
   */
  static bool ucs2ToUTF8(const std::u16string& ucs2StringSrc, std::string& utf8StringDst);

  /**
   * Perform logical to visual processing on the string.
   * Use it for readable text, GUI strings etc.
   *
   * \param[in]  utf8StringSrc        is source UTF-8 string to process
   * \param[out] utf8StringDst        is output UTF-8 string, empty on any error
   * \param[in]  bidiOptions          options for logical to visual transformation
   *                                  default is LTR | REMOVE_CONTROLS which should be fine
   *                                  in most cases
   *
   * \return true on successful conversion, false on any error
   * \sa CCharsetConverter::BiDiOptions
   * \sa logicalToVisualBiDi(const std::u16string&, std::u16string&, uint16_t)
   * \sa logicalToVisualBiDi(const std::wstring&, std::wstring&, uint16_t)
   * \sa logicalToVisualBiDi(const std::u32string, std::u32string&, uint16_t)
   */
  static bool logicalToVisualBiDi(const std::string& utf8StringSrc, std::string& utf8StringDst, 
                                  uint16_t bidiOptions = LTR | REMOVE_CONTROLS);

  /**
   * Perform logical to visual processing on the string.
   * Use it for readable text, GUI strings etc.
   *
   * \param[in]  utf16StringSrc       is source UTF-16 string to process
   * \param[out] utf16StringDst       is output UTF-16 string, empty on any error
   * \param[in]  bidiOptions          options for logical to visual transformation
   *                                  default is LTR | REMOVE_CONTROLS which should be fine
   *                                  in most cases
   *
   * \return true on successful conversion, false on any error
   * \sa CCharsetConverter::BiDiOptions
   * \sa logicalToVisualBiDi(const std::string&, std::string&, uint16_t)
   * \sa logicalToVisualBiDi(const std::wstring&, std::wstring&, uint16_t)
   * \sa logicalToVisualBiDi(const std::u32string, std::u32string&, uint16_t)
   */
  static bool logicalToVisualBiDi(const std::u16string& utf16StringSrc, std::u16string& utf16StringDst,
                                  uint16_t bidiOptions = LTR | REMOVE_CONTROLS);

  /**
   * Perform logical to visual processing on the string.
   * Use it for readable text, GUI strings etc.
   *
   * \param[in]  wStringSrc           is source wide string to process
   * \param[out] wStringDst           is output wide string, empty on any error
   * \param[in]  bidiOptions          options for logical to visual transformation
   *                                  default is LTR | REMOVE_CONTROLS which should be fine
   *                                  in most cases
   *
   * \return true on successful conversion, false on any error
   * \sa CCharsetConverter::BiDiOptions
   * \sa logicalToVisualBiDi(const std::string&, std::string&, uint16_t)
   * \sa logicalToVisualBiDi(const std::u16string&, std::u16string&, uint16_t)
   * \sa logicalToVisualBiDi(const std::u32string, std::u32string&, uint16_t)
   */
  static bool logicalToVisualBiDi(const std::wstring& wStringSrc, std::wstring& wStringDst,
                                  uint16_t bidiOptions = LTR | REMOVE_CONTROLS);

  /**
   * Perform logical to visual processing on the string.
   * Use it for readable text, GUI strings etc.
   *
   * \param[in]  utf32StringSrc       is source UTF-32 string to process
   * \param[out] utf32StringDst       is output UTF-32 string, empty on any error
   * \param[in]  bidiOptions          options for logical to visual transformation
   *                                  default is LTR | REMOVE_CONTROLS which should be fine
   *                                  in most cases
   *
   * \return true on successful conversion, false on any error
   * \sa CCharsetConverter::BiDiOptions
   * \sa logicalToVisualBiDi(const std::string&, std::string&, uint16_t)
   * \sa logicalToVisualBiDi(const std::u16string&, std::u16string&, uint16_t)
   * \sa logicalToVisualBiDi(const std::wstring&, std::wstring&, uint16_t)
   */
  static bool logicalToVisualBiDi(const std::u32string& utf32StringSrc, std::u32string& utf32StringDst,
                                  uint16_t bidiOptions = LTR | REMOVE_CONTROLS);

  /**
   * Reverse an RTL string, taking into account encoding
   *
   * \param[in]  utf8StringSrc     RTL string to be reversed in utf8 encoding
   * \param[out] utf8StringDst     Destination for the reversed string
   *
   * \return true in success, false otherwise
   */
  static bool reverseRTL(const std::string& utf8StringSrc, std::string& utf8StringDst);

  /**
   * Convert from unkown encoding to UTF-8
   *
   * \param[in,out]  stringSrcDst        is source string to convert and destination string
   *                                     Undefined value on error
   *
   * \return true on successful conversion, false on any error
   * \sa unkownToUTF8(const std::string&, std::string&)
   */
  static bool unknownToUTF8(std::string& stringSrcDst);

  /**
   * Convert from unkown encoding to UTF-8
   *
   * \param[in]  stringSrcDst       is source string to convert and destination string
   * \param[out] utf8StringDst      is output UTF-8 string
   *
   * \return true on successful conversion, false on any error
   * \sa unkownToUTF8(std::string&)
   */
  static bool unknownToUTF8(const std::string& stringSrc, std::string& utf8StringDst);

  /**
   * Convert UTF-8 string to system encoding and perform extra processing
   * to ensure that the string is valid for file system operations.
   * Fails on invalid byte sequences.
   *
   * \param[in]  stringSrc        is source UTF-8 string to convert
   * \param[out] stringDst        is output string in system encoding
   *
   * \return true on successful conversion, false on any error
   */
  static bool utf8ToSystemSafe(const std::string& stringSrc, std::string& stringDst);

  /**
   * Convert wide string to UTF-8 string and perform extra processing
   * to ensure that the string is valid for file system operations.
   * Fails on invalid byte sequences.
   *
   * \param[in]  wStringSrc          is source wide string to convert
   * \param[out] utf8StringDst       is output UTF-8 string
   *
   * \return true on successful conversion, false on any error
   */
  static bool wToUTF8SystemSafe(const std::wstring& wStringSrc, std::string& utf8StringDst);

private:

  class CInnerConverter;
};

XBMC_GLOBAL(CCharsetConverter,g_charsetConverter);