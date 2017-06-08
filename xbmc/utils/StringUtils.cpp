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
//-----------------------------------------------------------------------
//
//  File:      StringUtils.cpp
//
//  Purpose:   ATL split string utility
//  Author:    Paul J. Weiss
//
//  Modified to use J O'Leary's std::string class by kraqh3d
//
//------------------------------------------------------------------------

#include <guid.h>

#if defined(TARGET_ANDROID)
#include <androidjni/JNIThreading.h>
#endif

#include "StringUtils.h"
#include "CharsetConverter.h"
#include "utils/fstrcmp.h"
#include "Util.h"
#include <functional>
#include <array>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <algorithm>
#include "utils/RegExp.h" // don't move or std functions end up in PCRE namespace

#define FORMAT_BLOCK_SIZE 512 // # of bytes for initial allocation for printf

const char* ADDON_GUID_RE = "^(\\{){0,1}[0-9a-fA-F]{8}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{4}\\-[0-9a-fA-F]{12}(\\}){0,1}$";

/* empty string for use in returns by ref */
const std::string StringUtils::Empty = "";

//	Copyright (c) Leigh Brasington 2012.  All rights reserved.
//  This code may be used and reproduced without written permission.
//  http://www.leighb.com/tounicupper.htm
//
//	The tables were constructed from
//	http://publib.boulder.ibm.com/infocenter/iseries/v7r1m0/index.jsp?topic=%2Fnls%2Frbagslowtoupmaptable.htm

static wchar_t unicode_lowers[] = {
  static_cast<wchar_t>(0x0061), static_cast<wchar_t>(0x0062), static_cast<wchar_t>(0x0063), static_cast<wchar_t>(0x0064), static_cast<wchar_t>(0x0065), static_cast<wchar_t>(0x0066), static_cast<wchar_t>(0x0067), static_cast<wchar_t>(0x0068), static_cast<wchar_t>(0x0069),
  static_cast<wchar_t>(0x006A), static_cast<wchar_t>(0x006B), static_cast<wchar_t>(0x006C), static_cast<wchar_t>(0x006D), static_cast<wchar_t>(0x006E), static_cast<wchar_t>(0x006F), static_cast<wchar_t>(0x0070), static_cast<wchar_t>(0x0071), static_cast<wchar_t>(0x0072),
  static_cast<wchar_t>(0x0073), static_cast<wchar_t>(0x0074), static_cast<wchar_t>(0x0075), static_cast<wchar_t>(0x0076), static_cast<wchar_t>(0x0077), static_cast<wchar_t>(0x0078), static_cast<wchar_t>(0x0079), static_cast<wchar_t>(0x007A), static_cast<wchar_t>(0x00E0),
  static_cast<wchar_t>(0x00E1), static_cast<wchar_t>(0x00E2), static_cast<wchar_t>(0x00E3), static_cast<wchar_t>(0x00E4), static_cast<wchar_t>(0x00E5), static_cast<wchar_t>(0x00E6), static_cast<wchar_t>(0x00E7), static_cast<wchar_t>(0x00E8), static_cast<wchar_t>(0x00E9),
  static_cast<wchar_t>(0x00EA), static_cast<wchar_t>(0x00EB), static_cast<wchar_t>(0x00EC), static_cast<wchar_t>(0x00ED), static_cast<wchar_t>(0x00EE), static_cast<wchar_t>(0x00EF), static_cast<wchar_t>(0x00F0), static_cast<wchar_t>(0x00F1), static_cast<wchar_t>(0x00F2),
  static_cast<wchar_t>(0x00F3), static_cast<wchar_t>(0x00F4), static_cast<wchar_t>(0x00F5), static_cast<wchar_t>(0x00F6), static_cast<wchar_t>(0x00F8), static_cast<wchar_t>(0x00F9), static_cast<wchar_t>(0x00FA), static_cast<wchar_t>(0x00FB), static_cast<wchar_t>(0x00FC),
  static_cast<wchar_t>(0x00FD), static_cast<wchar_t>(0x00FE), static_cast<wchar_t>(0x00FF), static_cast<wchar_t>(0x0101), static_cast<wchar_t>(0x0103), static_cast<wchar_t>(0x0105), static_cast<wchar_t>(0x0107), static_cast<wchar_t>(0x0109), static_cast<wchar_t>(0x010B),
  static_cast<wchar_t>(0x010D), static_cast<wchar_t>(0x010F), static_cast<wchar_t>(0x0111), static_cast<wchar_t>(0x0113), static_cast<wchar_t>(0x0115), static_cast<wchar_t>(0x0117), static_cast<wchar_t>(0x0119), static_cast<wchar_t>(0x011B), static_cast<wchar_t>(0x011D),
  static_cast<wchar_t>(0x011F), static_cast<wchar_t>(0x0121), static_cast<wchar_t>(0x0123), static_cast<wchar_t>(0x0125), static_cast<wchar_t>(0x0127), static_cast<wchar_t>(0x0129), static_cast<wchar_t>(0x012B), static_cast<wchar_t>(0x012D), static_cast<wchar_t>(0x012F),
  static_cast<wchar_t>(0x0131), static_cast<wchar_t>(0x0133), static_cast<wchar_t>(0x0135), static_cast<wchar_t>(0x0137), static_cast<wchar_t>(0x013A), static_cast<wchar_t>(0x013C), static_cast<wchar_t>(0x013E), static_cast<wchar_t>(0x0140), static_cast<wchar_t>(0x0142),
  static_cast<wchar_t>(0x0144), static_cast<wchar_t>(0x0146), static_cast<wchar_t>(0x0148), static_cast<wchar_t>(0x014B), static_cast<wchar_t>(0x014D), static_cast<wchar_t>(0x014F), static_cast<wchar_t>(0x0151), static_cast<wchar_t>(0x0153), static_cast<wchar_t>(0x0155),
  static_cast<wchar_t>(0x0157), static_cast<wchar_t>(0x0159), static_cast<wchar_t>(0x015B), static_cast<wchar_t>(0x015D), static_cast<wchar_t>(0x015F), static_cast<wchar_t>(0x0161), static_cast<wchar_t>(0x0163), static_cast<wchar_t>(0x0165), static_cast<wchar_t>(0x0167),
  static_cast<wchar_t>(0x0169), static_cast<wchar_t>(0x016B), static_cast<wchar_t>(0x016D), static_cast<wchar_t>(0x016F), static_cast<wchar_t>(0x0171), static_cast<wchar_t>(0x0173), static_cast<wchar_t>(0x0175), static_cast<wchar_t>(0x0177), static_cast<wchar_t>(0x017A),
  static_cast<wchar_t>(0x017C), static_cast<wchar_t>(0x017E), static_cast<wchar_t>(0x0183), static_cast<wchar_t>(0x0185), static_cast<wchar_t>(0x0188), static_cast<wchar_t>(0x018C), static_cast<wchar_t>(0x0192), static_cast<wchar_t>(0x0199), static_cast<wchar_t>(0x01A1),
  static_cast<wchar_t>(0x01A3), static_cast<wchar_t>(0x01A5), static_cast<wchar_t>(0x01A8), static_cast<wchar_t>(0x01AD), static_cast<wchar_t>(0x01B0), static_cast<wchar_t>(0x01B4), static_cast<wchar_t>(0x01B6), static_cast<wchar_t>(0x01B9), static_cast<wchar_t>(0x01BD),
  static_cast<wchar_t>(0x01C6), static_cast<wchar_t>(0x01C9), static_cast<wchar_t>(0x01CC), static_cast<wchar_t>(0x01CE), static_cast<wchar_t>(0x01D0), static_cast<wchar_t>(0x01D2), static_cast<wchar_t>(0x01D4), static_cast<wchar_t>(0x01D6), static_cast<wchar_t>(0x01D8),
  static_cast<wchar_t>(0x01DA), static_cast<wchar_t>(0x01DC), static_cast<wchar_t>(0x01DF), static_cast<wchar_t>(0x01E1), static_cast<wchar_t>(0x01E3), static_cast<wchar_t>(0x01E5), static_cast<wchar_t>(0x01E7), static_cast<wchar_t>(0x01E9), static_cast<wchar_t>(0x01EB),
  static_cast<wchar_t>(0x01ED), static_cast<wchar_t>(0x01EF), static_cast<wchar_t>(0x01F3), static_cast<wchar_t>(0x01F5), static_cast<wchar_t>(0x01FB), static_cast<wchar_t>(0x01FD), static_cast<wchar_t>(0x01FF), static_cast<wchar_t>(0x0201), static_cast<wchar_t>(0x0203),
  static_cast<wchar_t>(0x0205), static_cast<wchar_t>(0x0207), static_cast<wchar_t>(0x0209), static_cast<wchar_t>(0x020B), static_cast<wchar_t>(0x020D), static_cast<wchar_t>(0x020F), static_cast<wchar_t>(0x0211), static_cast<wchar_t>(0x0213), static_cast<wchar_t>(0x0215),
  static_cast<wchar_t>(0x0217), static_cast<wchar_t>(0x0253), static_cast<wchar_t>(0x0254), static_cast<wchar_t>(0x0257), static_cast<wchar_t>(0x0258), static_cast<wchar_t>(0x0259), static_cast<wchar_t>(0x025B), static_cast<wchar_t>(0x0260), static_cast<wchar_t>(0x0263),
  static_cast<wchar_t>(0x0268), static_cast<wchar_t>(0x0269), static_cast<wchar_t>(0x026F), static_cast<wchar_t>(0x0272), static_cast<wchar_t>(0x0275), static_cast<wchar_t>(0x0283), static_cast<wchar_t>(0x0288), static_cast<wchar_t>(0x028A), static_cast<wchar_t>(0x028B),
  static_cast<wchar_t>(0x0292), static_cast<wchar_t>(0x03AC), static_cast<wchar_t>(0x03AD), static_cast<wchar_t>(0x03AE), static_cast<wchar_t>(0x03AF), static_cast<wchar_t>(0x03B1), static_cast<wchar_t>(0x03B2), static_cast<wchar_t>(0x03B3), static_cast<wchar_t>(0x03B4),
  static_cast<wchar_t>(0x03B5), static_cast<wchar_t>(0x03B6), static_cast<wchar_t>(0x03B7), static_cast<wchar_t>(0x03B8), static_cast<wchar_t>(0x03B9), static_cast<wchar_t>(0x03BA), static_cast<wchar_t>(0x03BB), static_cast<wchar_t>(0x03BC), static_cast<wchar_t>(0x03BD),
  static_cast<wchar_t>(0x03BE), static_cast<wchar_t>(0x03BF), static_cast<wchar_t>(0x03C0), static_cast<wchar_t>(0x03C1), static_cast<wchar_t>(0x03C3), static_cast<wchar_t>(0x03C4), static_cast<wchar_t>(0x03C5), static_cast<wchar_t>(0x03C6), static_cast<wchar_t>(0x03C7),
  static_cast<wchar_t>(0x03C8), static_cast<wchar_t>(0x03C9), static_cast<wchar_t>(0x03CA), static_cast<wchar_t>(0x03CB), static_cast<wchar_t>(0x03CC), static_cast<wchar_t>(0x03CD), static_cast<wchar_t>(0x03CE), static_cast<wchar_t>(0x03E3), static_cast<wchar_t>(0x03E5),
  static_cast<wchar_t>(0x03E7), static_cast<wchar_t>(0x03E9), static_cast<wchar_t>(0x03EB), static_cast<wchar_t>(0x03ED), static_cast<wchar_t>(0x03EF), static_cast<wchar_t>(0x0430), static_cast<wchar_t>(0x0431), static_cast<wchar_t>(0x0432), static_cast<wchar_t>(0x0433),
  static_cast<wchar_t>(0x0434), static_cast<wchar_t>(0x0435), static_cast<wchar_t>(0x0436), static_cast<wchar_t>(0x0437), static_cast<wchar_t>(0x0438), static_cast<wchar_t>(0x0439), static_cast<wchar_t>(0x043A), static_cast<wchar_t>(0x043B), static_cast<wchar_t>(0x043C),
  static_cast<wchar_t>(0x043D), static_cast<wchar_t>(0x043E), static_cast<wchar_t>(0x043F), static_cast<wchar_t>(0x0440), static_cast<wchar_t>(0x0441), static_cast<wchar_t>(0x0442), static_cast<wchar_t>(0x0443), static_cast<wchar_t>(0x0444), static_cast<wchar_t>(0x0445),
  static_cast<wchar_t>(0x0446), static_cast<wchar_t>(0x0447), static_cast<wchar_t>(0x0448), static_cast<wchar_t>(0x0449), static_cast<wchar_t>(0x044A), static_cast<wchar_t>(0x044B), static_cast<wchar_t>(0x044C), static_cast<wchar_t>(0x044D), static_cast<wchar_t>(0x044E),
  static_cast<wchar_t>(0x044F), static_cast<wchar_t>(0x0451), static_cast<wchar_t>(0x0452), static_cast<wchar_t>(0x0453), static_cast<wchar_t>(0x0454), static_cast<wchar_t>(0x0455), static_cast<wchar_t>(0x0456), static_cast<wchar_t>(0x0457), static_cast<wchar_t>(0x0458),
  static_cast<wchar_t>(0x0459), static_cast<wchar_t>(0x045A), static_cast<wchar_t>(0x045B), static_cast<wchar_t>(0x045C), static_cast<wchar_t>(0x045E), static_cast<wchar_t>(0x045F), static_cast<wchar_t>(0x0461), static_cast<wchar_t>(0x0463), static_cast<wchar_t>(0x0465),
  static_cast<wchar_t>(0x0467), static_cast<wchar_t>(0x0469), static_cast<wchar_t>(0x046B), static_cast<wchar_t>(0x046D), static_cast<wchar_t>(0x046F), static_cast<wchar_t>(0x0471), static_cast<wchar_t>(0x0473), static_cast<wchar_t>(0x0475), static_cast<wchar_t>(0x0477),
  static_cast<wchar_t>(0x0479), static_cast<wchar_t>(0x047B), static_cast<wchar_t>(0x047D), static_cast<wchar_t>(0x047F), static_cast<wchar_t>(0x0481), static_cast<wchar_t>(0x0491), static_cast<wchar_t>(0x0493), static_cast<wchar_t>(0x0495), static_cast<wchar_t>(0x0497),
  static_cast<wchar_t>(0x0499), static_cast<wchar_t>(0x049B), static_cast<wchar_t>(0x049D), static_cast<wchar_t>(0x049F), static_cast<wchar_t>(0x04A1), static_cast<wchar_t>(0x04A3), static_cast<wchar_t>(0x04A5), static_cast<wchar_t>(0x04A7), static_cast<wchar_t>(0x04A9),
  static_cast<wchar_t>(0x04AB), static_cast<wchar_t>(0x04AD), static_cast<wchar_t>(0x04AF), static_cast<wchar_t>(0x04B1), static_cast<wchar_t>(0x04B3), static_cast<wchar_t>(0x04B5), static_cast<wchar_t>(0x04B7), static_cast<wchar_t>(0x04B9), static_cast<wchar_t>(0x04BB),
  static_cast<wchar_t>(0x04BD), static_cast<wchar_t>(0x04BF), static_cast<wchar_t>(0x04C2), static_cast<wchar_t>(0x04C4), static_cast<wchar_t>(0x04C8), static_cast<wchar_t>(0x04CC), static_cast<wchar_t>(0x04D1), static_cast<wchar_t>(0x04D3), static_cast<wchar_t>(0x04D5),
  static_cast<wchar_t>(0x04D7), static_cast<wchar_t>(0x04D9), static_cast<wchar_t>(0x04DB), static_cast<wchar_t>(0x04DD), static_cast<wchar_t>(0x04DF), static_cast<wchar_t>(0x04E1), static_cast<wchar_t>(0x04E3), static_cast<wchar_t>(0x04E5), static_cast<wchar_t>(0x04E7),
  static_cast<wchar_t>(0x04E9), static_cast<wchar_t>(0x04EB), static_cast<wchar_t>(0x04EF), static_cast<wchar_t>(0x04F1), static_cast<wchar_t>(0x04F3), static_cast<wchar_t>(0x04F5), static_cast<wchar_t>(0x04F9), static_cast<wchar_t>(0x0561), static_cast<wchar_t>(0x0562),
  static_cast<wchar_t>(0x0563), static_cast<wchar_t>(0x0564), static_cast<wchar_t>(0x0565), static_cast<wchar_t>(0x0566), static_cast<wchar_t>(0x0567), static_cast<wchar_t>(0x0568), static_cast<wchar_t>(0x0569), static_cast<wchar_t>(0x056A), static_cast<wchar_t>(0x056B),
  static_cast<wchar_t>(0x056C), static_cast<wchar_t>(0x056D), static_cast<wchar_t>(0x056E), static_cast<wchar_t>(0x056F), static_cast<wchar_t>(0x0570), static_cast<wchar_t>(0x0571), static_cast<wchar_t>(0x0572), static_cast<wchar_t>(0x0573), static_cast<wchar_t>(0x0574),
  static_cast<wchar_t>(0x0575), static_cast<wchar_t>(0x0576), static_cast<wchar_t>(0x0577), static_cast<wchar_t>(0x0578), static_cast<wchar_t>(0x0579), static_cast<wchar_t>(0x057A), static_cast<wchar_t>(0x057B), static_cast<wchar_t>(0x057C), static_cast<wchar_t>(0x057D),
  static_cast<wchar_t>(0x057E), static_cast<wchar_t>(0x057F), static_cast<wchar_t>(0x0580), static_cast<wchar_t>(0x0581), static_cast<wchar_t>(0x0582), static_cast<wchar_t>(0x0583), static_cast<wchar_t>(0x0584), static_cast<wchar_t>(0x0585), static_cast<wchar_t>(0x0586),
  static_cast<wchar_t>(0x10D0), static_cast<wchar_t>(0x10D1), static_cast<wchar_t>(0x10D2), static_cast<wchar_t>(0x10D3), static_cast<wchar_t>(0x10D4), static_cast<wchar_t>(0x10D5), static_cast<wchar_t>(0x10D6), static_cast<wchar_t>(0x10D7), static_cast<wchar_t>(0x10D8),
  static_cast<wchar_t>(0x10D9), static_cast<wchar_t>(0x10DA), static_cast<wchar_t>(0x10DB), static_cast<wchar_t>(0x10DC), static_cast<wchar_t>(0x10DD), static_cast<wchar_t>(0x10DE), static_cast<wchar_t>(0x10DF), static_cast<wchar_t>(0x10E0), static_cast<wchar_t>(0x10E1),
  static_cast<wchar_t>(0x10E2), static_cast<wchar_t>(0x10E3), static_cast<wchar_t>(0x10E4), static_cast<wchar_t>(0x10E5), static_cast<wchar_t>(0x10E6), static_cast<wchar_t>(0x10E7), static_cast<wchar_t>(0x10E8), static_cast<wchar_t>(0x10E9), static_cast<wchar_t>(0x10EA),
  static_cast<wchar_t>(0x10EB), static_cast<wchar_t>(0x10EC), static_cast<wchar_t>(0x10ED), static_cast<wchar_t>(0x10EE), static_cast<wchar_t>(0x10EF), static_cast<wchar_t>(0x10F0), static_cast<wchar_t>(0x10F1), static_cast<wchar_t>(0x10F2), static_cast<wchar_t>(0x10F3),
  static_cast<wchar_t>(0x10F4), static_cast<wchar_t>(0x10F5), static_cast<wchar_t>(0x1E01), static_cast<wchar_t>(0x1E03), static_cast<wchar_t>(0x1E05), static_cast<wchar_t>(0x1E07), static_cast<wchar_t>(0x1E09), static_cast<wchar_t>(0x1E0B), static_cast<wchar_t>(0x1E0D),
  static_cast<wchar_t>(0x1E0F), static_cast<wchar_t>(0x1E11), static_cast<wchar_t>(0x1E13), static_cast<wchar_t>(0x1E15), static_cast<wchar_t>(0x1E17), static_cast<wchar_t>(0x1E19), static_cast<wchar_t>(0x1E1B), static_cast<wchar_t>(0x1E1D), static_cast<wchar_t>(0x1E1F),
  static_cast<wchar_t>(0x1E21), static_cast<wchar_t>(0x1E23), static_cast<wchar_t>(0x1E25), static_cast<wchar_t>(0x1E27), static_cast<wchar_t>(0x1E29), static_cast<wchar_t>(0x1E2B), static_cast<wchar_t>(0x1E2D), static_cast<wchar_t>(0x1E2F), static_cast<wchar_t>(0x1E31),
  static_cast<wchar_t>(0x1E33), static_cast<wchar_t>(0x1E35), static_cast<wchar_t>(0x1E37), static_cast<wchar_t>(0x1E39), static_cast<wchar_t>(0x1E3B), static_cast<wchar_t>(0x1E3D), static_cast<wchar_t>(0x1E3F), static_cast<wchar_t>(0x1E41), static_cast<wchar_t>(0x1E43),
  static_cast<wchar_t>(0x1E45), static_cast<wchar_t>(0x1E47), static_cast<wchar_t>(0x1E49), static_cast<wchar_t>(0x1E4B), static_cast<wchar_t>(0x1E4D), static_cast<wchar_t>(0x1E4F), static_cast<wchar_t>(0x1E51), static_cast<wchar_t>(0x1E53), static_cast<wchar_t>(0x1E55),
  static_cast<wchar_t>(0x1E57), static_cast<wchar_t>(0x1E59), static_cast<wchar_t>(0x1E5B), static_cast<wchar_t>(0x1E5D), static_cast<wchar_t>(0x1E5F), static_cast<wchar_t>(0x1E61), static_cast<wchar_t>(0x1E63), static_cast<wchar_t>(0x1E65), static_cast<wchar_t>(0x1E67),
  static_cast<wchar_t>(0x1E69), static_cast<wchar_t>(0x1E6B), static_cast<wchar_t>(0x1E6D), static_cast<wchar_t>(0x1E6F), static_cast<wchar_t>(0x1E71), static_cast<wchar_t>(0x1E73), static_cast<wchar_t>(0x1E75), static_cast<wchar_t>(0x1E77), static_cast<wchar_t>(0x1E79),
  static_cast<wchar_t>(0x1E7B), static_cast<wchar_t>(0x1E7D), static_cast<wchar_t>(0x1E7F), static_cast<wchar_t>(0x1E81), static_cast<wchar_t>(0x1E83), static_cast<wchar_t>(0x1E85), static_cast<wchar_t>(0x1E87), static_cast<wchar_t>(0x1E89), static_cast<wchar_t>(0x1E8B),
  static_cast<wchar_t>(0x1E8D), static_cast<wchar_t>(0x1E8F), static_cast<wchar_t>(0x1E91), static_cast<wchar_t>(0x1E93), static_cast<wchar_t>(0x1E95), static_cast<wchar_t>(0x1EA1), static_cast<wchar_t>(0x1EA3), static_cast<wchar_t>(0x1EA5), static_cast<wchar_t>(0x1EA7),
  static_cast<wchar_t>(0x1EA9), static_cast<wchar_t>(0x1EAB), static_cast<wchar_t>(0x1EAD), static_cast<wchar_t>(0x1EAF), static_cast<wchar_t>(0x1EB1), static_cast<wchar_t>(0x1EB3), static_cast<wchar_t>(0x1EB5), static_cast<wchar_t>(0x1EB7), static_cast<wchar_t>(0x1EB9),
  static_cast<wchar_t>(0x1EBB), static_cast<wchar_t>(0x1EBD), static_cast<wchar_t>(0x1EBF), static_cast<wchar_t>(0x1EC1), static_cast<wchar_t>(0x1EC3), static_cast<wchar_t>(0x1EC5), static_cast<wchar_t>(0x1EC7), static_cast<wchar_t>(0x1EC9), static_cast<wchar_t>(0x1ECB),
  static_cast<wchar_t>(0x1ECD), static_cast<wchar_t>(0x1ECF), static_cast<wchar_t>(0x1ED1), static_cast<wchar_t>(0x1ED3), static_cast<wchar_t>(0x1ED5), static_cast<wchar_t>(0x1ED7), static_cast<wchar_t>(0x1ED9), static_cast<wchar_t>(0x1EDB), static_cast<wchar_t>(0x1EDD),
  static_cast<wchar_t>(0x1EDF), static_cast<wchar_t>(0x1EE1), static_cast<wchar_t>(0x1EE3), static_cast<wchar_t>(0x1EE5), static_cast<wchar_t>(0x1EE7), static_cast<wchar_t>(0x1EE9), static_cast<wchar_t>(0x1EEB), static_cast<wchar_t>(0x1EED), static_cast<wchar_t>(0x1EEF),
  static_cast<wchar_t>(0x1EF1), static_cast<wchar_t>(0x1EF3), static_cast<wchar_t>(0x1EF5), static_cast<wchar_t>(0x1EF7), static_cast<wchar_t>(0x1EF9), static_cast<wchar_t>(0x1F00), static_cast<wchar_t>(0x1F01), static_cast<wchar_t>(0x1F02), static_cast<wchar_t>(0x1F03),
  static_cast<wchar_t>(0x1F04), static_cast<wchar_t>(0x1F05), static_cast<wchar_t>(0x1F06), static_cast<wchar_t>(0x1F07), static_cast<wchar_t>(0x1F10), static_cast<wchar_t>(0x1F11), static_cast<wchar_t>(0x1F12), static_cast<wchar_t>(0x1F13), static_cast<wchar_t>(0x1F14),
  static_cast<wchar_t>(0x1F15), static_cast<wchar_t>(0x1F20), static_cast<wchar_t>(0x1F21), static_cast<wchar_t>(0x1F22), static_cast<wchar_t>(0x1F23), static_cast<wchar_t>(0x1F24), static_cast<wchar_t>(0x1F25), static_cast<wchar_t>(0x1F26), static_cast<wchar_t>(0x1F27),
  static_cast<wchar_t>(0x1F30), static_cast<wchar_t>(0x1F31), static_cast<wchar_t>(0x1F32), static_cast<wchar_t>(0x1F33), static_cast<wchar_t>(0x1F34), static_cast<wchar_t>(0x1F35), static_cast<wchar_t>(0x1F36), static_cast<wchar_t>(0x1F37), static_cast<wchar_t>(0x1F40),
  static_cast<wchar_t>(0x1F41), static_cast<wchar_t>(0x1F42), static_cast<wchar_t>(0x1F43), static_cast<wchar_t>(0x1F44), static_cast<wchar_t>(0x1F45), static_cast<wchar_t>(0x1F51), static_cast<wchar_t>(0x1F53), static_cast<wchar_t>(0x1F55), static_cast<wchar_t>(0x1F57),
  static_cast<wchar_t>(0x1F60), static_cast<wchar_t>(0x1F61), static_cast<wchar_t>(0x1F62), static_cast<wchar_t>(0x1F63), static_cast<wchar_t>(0x1F64), static_cast<wchar_t>(0x1F65), static_cast<wchar_t>(0x1F66), static_cast<wchar_t>(0x1F67), static_cast<wchar_t>(0x1F80),
  static_cast<wchar_t>(0x1F81), static_cast<wchar_t>(0x1F82), static_cast<wchar_t>(0x1F83), static_cast<wchar_t>(0x1F84), static_cast<wchar_t>(0x1F85), static_cast<wchar_t>(0x1F86), static_cast<wchar_t>(0x1F87), static_cast<wchar_t>(0x1F90), static_cast<wchar_t>(0x1F91),
  static_cast<wchar_t>(0x1F92), static_cast<wchar_t>(0x1F93), static_cast<wchar_t>(0x1F94), static_cast<wchar_t>(0x1F95), static_cast<wchar_t>(0x1F96), static_cast<wchar_t>(0x1F97), static_cast<wchar_t>(0x1FA0), static_cast<wchar_t>(0x1FA1), static_cast<wchar_t>(0x1FA2),
  static_cast<wchar_t>(0x1FA3), static_cast<wchar_t>(0x1FA4), static_cast<wchar_t>(0x1FA5), static_cast<wchar_t>(0x1FA6), static_cast<wchar_t>(0x1FA7), static_cast<wchar_t>(0x1FB0), static_cast<wchar_t>(0x1FB1), static_cast<wchar_t>(0x1FD0), static_cast<wchar_t>(0x1FD1),
  static_cast<wchar_t>(0x1FE0), static_cast<wchar_t>(0x1FE1), static_cast<wchar_t>(0x24D0), static_cast<wchar_t>(0x24D1), static_cast<wchar_t>(0x24D2), static_cast<wchar_t>(0x24D3), static_cast<wchar_t>(0x24D4), static_cast<wchar_t>(0x24D5), static_cast<wchar_t>(0x24D6),
  static_cast<wchar_t>(0x24D7), static_cast<wchar_t>(0x24D8), static_cast<wchar_t>(0x24D9), static_cast<wchar_t>(0x24DA), static_cast<wchar_t>(0x24DB), static_cast<wchar_t>(0x24DC), static_cast<wchar_t>(0x24DD), static_cast<wchar_t>(0x24DE), static_cast<wchar_t>(0x24DF),
  static_cast<wchar_t>(0x24E0), static_cast<wchar_t>(0x24E1), static_cast<wchar_t>(0x24E2), static_cast<wchar_t>(0x24E3), static_cast<wchar_t>(0x24E4), static_cast<wchar_t>(0x24E5), static_cast<wchar_t>(0x24E6), static_cast<wchar_t>(0x24E7), static_cast<wchar_t>(0x24E8),
  static_cast<wchar_t>(0x24E9), static_cast<wchar_t>(0xFF41), static_cast<wchar_t>(0xFF42), static_cast<wchar_t>(0xFF43), static_cast<wchar_t>(0xFF44), static_cast<wchar_t>(0xFF45), static_cast<wchar_t>(0xFF46), static_cast<wchar_t>(0xFF47), static_cast<wchar_t>(0xFF48),
  static_cast<wchar_t>(0xFF49), static_cast<wchar_t>(0xFF4A), static_cast<wchar_t>(0xFF4B), static_cast<wchar_t>(0xFF4C), static_cast<wchar_t>(0xFF4D), static_cast<wchar_t>(0xFF4E), static_cast<wchar_t>(0xFF4F), static_cast<wchar_t>(0xFF50), static_cast<wchar_t>(0xFF51),
  static_cast<wchar_t>(0xFF52), static_cast<wchar_t>(0xFF53), static_cast<wchar_t>(0xFF54), static_cast<wchar_t>(0xFF55), static_cast<wchar_t>(0xFF56), static_cast<wchar_t>(0xFF57), static_cast<wchar_t>(0xFF58), static_cast<wchar_t>(0xFF59), static_cast<wchar_t>(0xFF5A)
};

static const wchar_t unicode_uppers[] = {
  static_cast<wchar_t>(0x0041), static_cast<wchar_t>(0x0042), static_cast<wchar_t>(0x0043), static_cast<wchar_t>(0x0044), static_cast<wchar_t>(0x0045), static_cast<wchar_t>(0x0046), static_cast<wchar_t>(0x0047), static_cast<wchar_t>(0x0048), static_cast<wchar_t>(0x0049),
  static_cast<wchar_t>(0x004A), static_cast<wchar_t>(0x004B), static_cast<wchar_t>(0x004C), static_cast<wchar_t>(0x004D), static_cast<wchar_t>(0x004E), static_cast<wchar_t>(0x004F), static_cast<wchar_t>(0x0050), static_cast<wchar_t>(0x0051), static_cast<wchar_t>(0x0052),
  static_cast<wchar_t>(0x0053), static_cast<wchar_t>(0x0054), static_cast<wchar_t>(0x0055), static_cast<wchar_t>(0x0056), static_cast<wchar_t>(0x0057), static_cast<wchar_t>(0x0058), static_cast<wchar_t>(0x0059), static_cast<wchar_t>(0x005A), static_cast<wchar_t>(0x00C0),
  static_cast<wchar_t>(0x00C1), static_cast<wchar_t>(0x00C2), static_cast<wchar_t>(0x00C3), static_cast<wchar_t>(0x00C4), static_cast<wchar_t>(0x00C5), static_cast<wchar_t>(0x00C6), static_cast<wchar_t>(0x00C7), static_cast<wchar_t>(0x00C8), static_cast<wchar_t>(0x00C9),
  static_cast<wchar_t>(0x00CA), static_cast<wchar_t>(0x00CB), static_cast<wchar_t>(0x00CC), static_cast<wchar_t>(0x00CD), static_cast<wchar_t>(0x00CE), static_cast<wchar_t>(0x00CF), static_cast<wchar_t>(0x00D0), static_cast<wchar_t>(0x00D1), static_cast<wchar_t>(0x00D2),
  static_cast<wchar_t>(0x00D3), static_cast<wchar_t>(0x00D4), static_cast<wchar_t>(0x00D5), static_cast<wchar_t>(0x00D6), static_cast<wchar_t>(0x00D8), static_cast<wchar_t>(0x00D9), static_cast<wchar_t>(0x00DA), static_cast<wchar_t>(0x00DB), static_cast<wchar_t>(0x00DC),
  static_cast<wchar_t>(0x00DD), static_cast<wchar_t>(0x00DE), static_cast<wchar_t>(0x0178), static_cast<wchar_t>(0x0100), static_cast<wchar_t>(0x0102), static_cast<wchar_t>(0x0104), static_cast<wchar_t>(0x0106), static_cast<wchar_t>(0x0108), static_cast<wchar_t>(0x010A),
  static_cast<wchar_t>(0x010C), static_cast<wchar_t>(0x010E), static_cast<wchar_t>(0x0110), static_cast<wchar_t>(0x0112), static_cast<wchar_t>(0x0114), static_cast<wchar_t>(0x0116), static_cast<wchar_t>(0x0118), static_cast<wchar_t>(0x011A), static_cast<wchar_t>(0x011C),
  static_cast<wchar_t>(0x011E), static_cast<wchar_t>(0x0120), static_cast<wchar_t>(0x0122), static_cast<wchar_t>(0x0124), static_cast<wchar_t>(0x0126), static_cast<wchar_t>(0x0128), static_cast<wchar_t>(0x012A), static_cast<wchar_t>(0x012C), static_cast<wchar_t>(0x012E),
  static_cast<wchar_t>(0x0049), static_cast<wchar_t>(0x0132), static_cast<wchar_t>(0x0134), static_cast<wchar_t>(0x0136), static_cast<wchar_t>(0x0139), static_cast<wchar_t>(0x013B), static_cast<wchar_t>(0x013D), static_cast<wchar_t>(0x013F), static_cast<wchar_t>(0x0141),
  static_cast<wchar_t>(0x0143), static_cast<wchar_t>(0x0145), static_cast<wchar_t>(0x0147), static_cast<wchar_t>(0x014A), static_cast<wchar_t>(0x014C), static_cast<wchar_t>(0x014E), static_cast<wchar_t>(0x0150), static_cast<wchar_t>(0x0152), static_cast<wchar_t>(0x0154),
  static_cast<wchar_t>(0x0156), static_cast<wchar_t>(0x0158), static_cast<wchar_t>(0x015A), static_cast<wchar_t>(0x015C), static_cast<wchar_t>(0x015E), static_cast<wchar_t>(0x0160), static_cast<wchar_t>(0x0162), static_cast<wchar_t>(0x0164), static_cast<wchar_t>(0x0166),
  static_cast<wchar_t>(0x0168), static_cast<wchar_t>(0x016A), static_cast<wchar_t>(0x016C), static_cast<wchar_t>(0x016E), static_cast<wchar_t>(0x0170), static_cast<wchar_t>(0x0172), static_cast<wchar_t>(0x0174), static_cast<wchar_t>(0x0176), static_cast<wchar_t>(0x0179),
  static_cast<wchar_t>(0x017B), static_cast<wchar_t>(0x017D), static_cast<wchar_t>(0x0182), static_cast<wchar_t>(0x0184), static_cast<wchar_t>(0x0187), static_cast<wchar_t>(0x018B), static_cast<wchar_t>(0x0191), static_cast<wchar_t>(0x0198), static_cast<wchar_t>(0x01A0),
  static_cast<wchar_t>(0x01A2), static_cast<wchar_t>(0x01A4), static_cast<wchar_t>(0x01A7), static_cast<wchar_t>(0x01AC), static_cast<wchar_t>(0x01AF), static_cast<wchar_t>(0x01B3), static_cast<wchar_t>(0x01B5), static_cast<wchar_t>(0x01B8), static_cast<wchar_t>(0x01BC),
  static_cast<wchar_t>(0x01C4), static_cast<wchar_t>(0x01C7), static_cast<wchar_t>(0x01CA), static_cast<wchar_t>(0x01CD), static_cast<wchar_t>(0x01CF), static_cast<wchar_t>(0x01D1), static_cast<wchar_t>(0x01D3), static_cast<wchar_t>(0x01D5), static_cast<wchar_t>(0x01D7),
  static_cast<wchar_t>(0x01D9), static_cast<wchar_t>(0x01DB), static_cast<wchar_t>(0x01DE), static_cast<wchar_t>(0x01E0), static_cast<wchar_t>(0x01E2), static_cast<wchar_t>(0x01E4), static_cast<wchar_t>(0x01E6), static_cast<wchar_t>(0x01E8), static_cast<wchar_t>(0x01EA),
  static_cast<wchar_t>(0x01EC), static_cast<wchar_t>(0x01EE), static_cast<wchar_t>(0x01F1), static_cast<wchar_t>(0x01F4), static_cast<wchar_t>(0x01FA), static_cast<wchar_t>(0x01FC), static_cast<wchar_t>(0x01FE), static_cast<wchar_t>(0x0200), static_cast<wchar_t>(0x0202),
  static_cast<wchar_t>(0x0204), static_cast<wchar_t>(0x0206), static_cast<wchar_t>(0x0208), static_cast<wchar_t>(0x020A), static_cast<wchar_t>(0x020C), static_cast<wchar_t>(0x020E), static_cast<wchar_t>(0x0210), static_cast<wchar_t>(0x0212), static_cast<wchar_t>(0x0214),
  static_cast<wchar_t>(0x0216), static_cast<wchar_t>(0x0181), static_cast<wchar_t>(0x0186), static_cast<wchar_t>(0x018A), static_cast<wchar_t>(0x018E), static_cast<wchar_t>(0x018F), static_cast<wchar_t>(0x0190), static_cast<wchar_t>(0x0193), static_cast<wchar_t>(0x0194),
  static_cast<wchar_t>(0x0197), static_cast<wchar_t>(0x0196), static_cast<wchar_t>(0x019C), static_cast<wchar_t>(0x019D), static_cast<wchar_t>(0x019F), static_cast<wchar_t>(0x01A9), static_cast<wchar_t>(0x01AE), static_cast<wchar_t>(0x01B1), static_cast<wchar_t>(0x01B2),
  static_cast<wchar_t>(0x01B7), static_cast<wchar_t>(0x0386), static_cast<wchar_t>(0x0388), static_cast<wchar_t>(0x0389), static_cast<wchar_t>(0x038A), static_cast<wchar_t>(0x0391), static_cast<wchar_t>(0x0392), static_cast<wchar_t>(0x0393), static_cast<wchar_t>(0x0394),
  static_cast<wchar_t>(0x0395), static_cast<wchar_t>(0x0396), static_cast<wchar_t>(0x0397), static_cast<wchar_t>(0x0398), static_cast<wchar_t>(0x0399), static_cast<wchar_t>(0x039A), static_cast<wchar_t>(0x039B), static_cast<wchar_t>(0x039C), static_cast<wchar_t>(0x039D),
  static_cast<wchar_t>(0x039E), static_cast<wchar_t>(0x039F), static_cast<wchar_t>(0x03A0), static_cast<wchar_t>(0x03A1), static_cast<wchar_t>(0x03A3), static_cast<wchar_t>(0x03A4), static_cast<wchar_t>(0x03A5), static_cast<wchar_t>(0x03A6), static_cast<wchar_t>(0x03A7),
  static_cast<wchar_t>(0x03A8), static_cast<wchar_t>(0x03A9), static_cast<wchar_t>(0x03AA), static_cast<wchar_t>(0x03AB), static_cast<wchar_t>(0x038C), static_cast<wchar_t>(0x038E), static_cast<wchar_t>(0x038F), static_cast<wchar_t>(0x03E2), static_cast<wchar_t>(0x03E4),
  static_cast<wchar_t>(0x03E6), static_cast<wchar_t>(0x03E8), static_cast<wchar_t>(0x03EA), static_cast<wchar_t>(0x03EC), static_cast<wchar_t>(0x03EE), static_cast<wchar_t>(0x0410), static_cast<wchar_t>(0x0411), static_cast<wchar_t>(0x0412), static_cast<wchar_t>(0x0413),
  static_cast<wchar_t>(0x0414), static_cast<wchar_t>(0x0415), static_cast<wchar_t>(0x0416), static_cast<wchar_t>(0x0417), static_cast<wchar_t>(0x0418), static_cast<wchar_t>(0x0419), static_cast<wchar_t>(0x041A), static_cast<wchar_t>(0x041B), static_cast<wchar_t>(0x041C),
  static_cast<wchar_t>(0x041D), static_cast<wchar_t>(0x041E), static_cast<wchar_t>(0x041F), static_cast<wchar_t>(0x0420), static_cast<wchar_t>(0x0421), static_cast<wchar_t>(0x0422), static_cast<wchar_t>(0x0423), static_cast<wchar_t>(0x0424), static_cast<wchar_t>(0x0425),
  static_cast<wchar_t>(0x0426), static_cast<wchar_t>(0x0427), static_cast<wchar_t>(0x0428), static_cast<wchar_t>(0x0429), static_cast<wchar_t>(0x042A), static_cast<wchar_t>(0x042B), static_cast<wchar_t>(0x042C), static_cast<wchar_t>(0x042D), static_cast<wchar_t>(0x042E),
  static_cast<wchar_t>(0x042F), static_cast<wchar_t>(0x0401), static_cast<wchar_t>(0x0402), static_cast<wchar_t>(0x0403), static_cast<wchar_t>(0x0404), static_cast<wchar_t>(0x0405), static_cast<wchar_t>(0x0406), static_cast<wchar_t>(0x0407), static_cast<wchar_t>(0x0408),
  static_cast<wchar_t>(0x0409), static_cast<wchar_t>(0x040A), static_cast<wchar_t>(0x040B), static_cast<wchar_t>(0x040C), static_cast<wchar_t>(0x040E), static_cast<wchar_t>(0x040F), static_cast<wchar_t>(0x0460), static_cast<wchar_t>(0x0462), static_cast<wchar_t>(0x0464),
  static_cast<wchar_t>(0x0466), static_cast<wchar_t>(0x0468), static_cast<wchar_t>(0x046A), static_cast<wchar_t>(0x046C), static_cast<wchar_t>(0x046E), static_cast<wchar_t>(0x0470), static_cast<wchar_t>(0x0472), static_cast<wchar_t>(0x0474), static_cast<wchar_t>(0x0476),
  static_cast<wchar_t>(0x0478), static_cast<wchar_t>(0x047A), static_cast<wchar_t>(0x047C), static_cast<wchar_t>(0x047E), static_cast<wchar_t>(0x0480), static_cast<wchar_t>(0x0490), static_cast<wchar_t>(0x0492), static_cast<wchar_t>(0x0494), static_cast<wchar_t>(0x0496),
  static_cast<wchar_t>(0x0498), static_cast<wchar_t>(0x049A), static_cast<wchar_t>(0x049C), static_cast<wchar_t>(0x049E), static_cast<wchar_t>(0x04A0), static_cast<wchar_t>(0x04A2), static_cast<wchar_t>(0x04A4), static_cast<wchar_t>(0x04A6), static_cast<wchar_t>(0x04A8),
  static_cast<wchar_t>(0x04AA), static_cast<wchar_t>(0x04AC), static_cast<wchar_t>(0x04AE), static_cast<wchar_t>(0x04B0), static_cast<wchar_t>(0x04B2), static_cast<wchar_t>(0x04B4), static_cast<wchar_t>(0x04B6), static_cast<wchar_t>(0x04B8), static_cast<wchar_t>(0x04BA),
  static_cast<wchar_t>(0x04BC), static_cast<wchar_t>(0x04BE), static_cast<wchar_t>(0x04C1), static_cast<wchar_t>(0x04C3), static_cast<wchar_t>(0x04C7), static_cast<wchar_t>(0x04CB), static_cast<wchar_t>(0x04D0), static_cast<wchar_t>(0x04D2), static_cast<wchar_t>(0x04D4),
  static_cast<wchar_t>(0x04D6), static_cast<wchar_t>(0x04D8), static_cast<wchar_t>(0x04DA), static_cast<wchar_t>(0x04DC), static_cast<wchar_t>(0x04DE), static_cast<wchar_t>(0x04E0), static_cast<wchar_t>(0x04E2), static_cast<wchar_t>(0x04E4), static_cast<wchar_t>(0x04E6),
  static_cast<wchar_t>(0x04E8), static_cast<wchar_t>(0x04EA), static_cast<wchar_t>(0x04EE), static_cast<wchar_t>(0x04F0), static_cast<wchar_t>(0x04F2), static_cast<wchar_t>(0x04F4), static_cast<wchar_t>(0x04F8), static_cast<wchar_t>(0x0531), static_cast<wchar_t>(0x0532),
  static_cast<wchar_t>(0x0533), static_cast<wchar_t>(0x0534), static_cast<wchar_t>(0x0535), static_cast<wchar_t>(0x0536), static_cast<wchar_t>(0x0537), static_cast<wchar_t>(0x0538), static_cast<wchar_t>(0x0539), static_cast<wchar_t>(0x053A), static_cast<wchar_t>(0x053B),
  static_cast<wchar_t>(0x053C), static_cast<wchar_t>(0x053D), static_cast<wchar_t>(0x053E), static_cast<wchar_t>(0x053F), static_cast<wchar_t>(0x0540), static_cast<wchar_t>(0x0541), static_cast<wchar_t>(0x0542), static_cast<wchar_t>(0x0543), static_cast<wchar_t>(0x0544),
  static_cast<wchar_t>(0x0545), static_cast<wchar_t>(0x0546), static_cast<wchar_t>(0x0547), static_cast<wchar_t>(0x0548), static_cast<wchar_t>(0x0549), static_cast<wchar_t>(0x054A), static_cast<wchar_t>(0x054B), static_cast<wchar_t>(0x054C), static_cast<wchar_t>(0x054D),
  static_cast<wchar_t>(0x054E), static_cast<wchar_t>(0x054F), static_cast<wchar_t>(0x0550), static_cast<wchar_t>(0x0551), static_cast<wchar_t>(0x0552), static_cast<wchar_t>(0x0553), static_cast<wchar_t>(0x0554), static_cast<wchar_t>(0x0555), static_cast<wchar_t>(0x0556),
  static_cast<wchar_t>(0x10A0), static_cast<wchar_t>(0x10A1), static_cast<wchar_t>(0x10A2), static_cast<wchar_t>(0x10A3), static_cast<wchar_t>(0x10A4), static_cast<wchar_t>(0x10A5), static_cast<wchar_t>(0x10A6), static_cast<wchar_t>(0x10A7), static_cast<wchar_t>(0x10A8),
  static_cast<wchar_t>(0x10A9), static_cast<wchar_t>(0x10AA), static_cast<wchar_t>(0x10AB), static_cast<wchar_t>(0x10AC), static_cast<wchar_t>(0x10AD), static_cast<wchar_t>(0x10AE), static_cast<wchar_t>(0x10AF), static_cast<wchar_t>(0x10B0), static_cast<wchar_t>(0x10B1),
  static_cast<wchar_t>(0x10B2), static_cast<wchar_t>(0x10B3), static_cast<wchar_t>(0x10B4), static_cast<wchar_t>(0x10B5), static_cast<wchar_t>(0x10B6), static_cast<wchar_t>(0x10B7), static_cast<wchar_t>(0x10B8), static_cast<wchar_t>(0x10B9), static_cast<wchar_t>(0x10BA),
  static_cast<wchar_t>(0x10BB), static_cast<wchar_t>(0x10BC), static_cast<wchar_t>(0x10BD), static_cast<wchar_t>(0x10BE), static_cast<wchar_t>(0x10BF), static_cast<wchar_t>(0x10C0), static_cast<wchar_t>(0x10C1), static_cast<wchar_t>(0x10C2), static_cast<wchar_t>(0x10C3),
  static_cast<wchar_t>(0x10C4), static_cast<wchar_t>(0x10C5), static_cast<wchar_t>(0x1E00), static_cast<wchar_t>(0x1E02), static_cast<wchar_t>(0x1E04), static_cast<wchar_t>(0x1E06), static_cast<wchar_t>(0x1E08), static_cast<wchar_t>(0x1E0A), static_cast<wchar_t>(0x1E0C),
  static_cast<wchar_t>(0x1E0E), static_cast<wchar_t>(0x1E10), static_cast<wchar_t>(0x1E12), static_cast<wchar_t>(0x1E14), static_cast<wchar_t>(0x1E16), static_cast<wchar_t>(0x1E18), static_cast<wchar_t>(0x1E1A), static_cast<wchar_t>(0x1E1C), static_cast<wchar_t>(0x1E1E),
  static_cast<wchar_t>(0x1E20), static_cast<wchar_t>(0x1E22), static_cast<wchar_t>(0x1E24), static_cast<wchar_t>(0x1E26), static_cast<wchar_t>(0x1E28), static_cast<wchar_t>(0x1E2A), static_cast<wchar_t>(0x1E2C), static_cast<wchar_t>(0x1E2E), static_cast<wchar_t>(0x1E30),
  static_cast<wchar_t>(0x1E32), static_cast<wchar_t>(0x1E34), static_cast<wchar_t>(0x1E36), static_cast<wchar_t>(0x1E38), static_cast<wchar_t>(0x1E3A), static_cast<wchar_t>(0x1E3C), static_cast<wchar_t>(0x1E3E), static_cast<wchar_t>(0x1E40), static_cast<wchar_t>(0x1E42),
  static_cast<wchar_t>(0x1E44), static_cast<wchar_t>(0x1E46), static_cast<wchar_t>(0x1E48), static_cast<wchar_t>(0x1E4A), static_cast<wchar_t>(0x1E4C), static_cast<wchar_t>(0x1E4E), static_cast<wchar_t>(0x1E50), static_cast<wchar_t>(0x1E52), static_cast<wchar_t>(0x1E54),
  static_cast<wchar_t>(0x1E56), static_cast<wchar_t>(0x1E58), static_cast<wchar_t>(0x1E5A), static_cast<wchar_t>(0x1E5C), static_cast<wchar_t>(0x1E5E), static_cast<wchar_t>(0x1E60), static_cast<wchar_t>(0x1E62), static_cast<wchar_t>(0x1E64), static_cast<wchar_t>(0x1E66),
  static_cast<wchar_t>(0x1E68), static_cast<wchar_t>(0x1E6A), static_cast<wchar_t>(0x1E6C), static_cast<wchar_t>(0x1E6E), static_cast<wchar_t>(0x1E70), static_cast<wchar_t>(0x1E72), static_cast<wchar_t>(0x1E74), static_cast<wchar_t>(0x1E76), static_cast<wchar_t>(0x1E78),
  static_cast<wchar_t>(0x1E7A), static_cast<wchar_t>(0x1E7C), static_cast<wchar_t>(0x1E7E), static_cast<wchar_t>(0x1E80), static_cast<wchar_t>(0x1E82), static_cast<wchar_t>(0x1E84), static_cast<wchar_t>(0x1E86), static_cast<wchar_t>(0x1E88), static_cast<wchar_t>(0x1E8A),
  static_cast<wchar_t>(0x1E8C), static_cast<wchar_t>(0x1E8E), static_cast<wchar_t>(0x1E90), static_cast<wchar_t>(0x1E92), static_cast<wchar_t>(0x1E94), static_cast<wchar_t>(0x1EA0), static_cast<wchar_t>(0x1EA2), static_cast<wchar_t>(0x1EA4), static_cast<wchar_t>(0x1EA6),
  static_cast<wchar_t>(0x1EA8), static_cast<wchar_t>(0x1EAA), static_cast<wchar_t>(0x1EAC), static_cast<wchar_t>(0x1EAE), static_cast<wchar_t>(0x1EB0), static_cast<wchar_t>(0x1EB2), static_cast<wchar_t>(0x1EB4), static_cast<wchar_t>(0x1EB6), static_cast<wchar_t>(0x1EB8),
  static_cast<wchar_t>(0x1EBA), static_cast<wchar_t>(0x1EBC), static_cast<wchar_t>(0x1EBE), static_cast<wchar_t>(0x1EC0), static_cast<wchar_t>(0x1EC2), static_cast<wchar_t>(0x1EC4), static_cast<wchar_t>(0x1EC6), static_cast<wchar_t>(0x1EC8), static_cast<wchar_t>(0x1ECA),
  static_cast<wchar_t>(0x1ECC), static_cast<wchar_t>(0x1ECE), static_cast<wchar_t>(0x1ED0), static_cast<wchar_t>(0x1ED2), static_cast<wchar_t>(0x1ED4), static_cast<wchar_t>(0x1ED6), static_cast<wchar_t>(0x1ED8), static_cast<wchar_t>(0x1EDA), static_cast<wchar_t>(0x1EDC),
  static_cast<wchar_t>(0x1EDE), static_cast<wchar_t>(0x1EE0), static_cast<wchar_t>(0x1EE2), static_cast<wchar_t>(0x1EE4), static_cast<wchar_t>(0x1EE6), static_cast<wchar_t>(0x1EE8), static_cast<wchar_t>(0x1EEA), static_cast<wchar_t>(0x1EEC), static_cast<wchar_t>(0x1EEE),
  static_cast<wchar_t>(0x1EF0), static_cast<wchar_t>(0x1EF2), static_cast<wchar_t>(0x1EF4), static_cast<wchar_t>(0x1EF6), static_cast<wchar_t>(0x1EF8), static_cast<wchar_t>(0x1F08), static_cast<wchar_t>(0x1F09), static_cast<wchar_t>(0x1F0A), static_cast<wchar_t>(0x1F0B),
  static_cast<wchar_t>(0x1F0C), static_cast<wchar_t>(0x1F0D), static_cast<wchar_t>(0x1F0E), static_cast<wchar_t>(0x1F0F), static_cast<wchar_t>(0x1F18), static_cast<wchar_t>(0x1F19), static_cast<wchar_t>(0x1F1A), static_cast<wchar_t>(0x1F1B), static_cast<wchar_t>(0x1F1C),
  static_cast<wchar_t>(0x1F1D), static_cast<wchar_t>(0x1F28), static_cast<wchar_t>(0x1F29), static_cast<wchar_t>(0x1F2A), static_cast<wchar_t>(0x1F2B), static_cast<wchar_t>(0x1F2C), static_cast<wchar_t>(0x1F2D), static_cast<wchar_t>(0x1F2E), static_cast<wchar_t>(0x1F2F),
  static_cast<wchar_t>(0x1F38), static_cast<wchar_t>(0x1F39), static_cast<wchar_t>(0x1F3A), static_cast<wchar_t>(0x1F3B), static_cast<wchar_t>(0x1F3C), static_cast<wchar_t>(0x1F3D), static_cast<wchar_t>(0x1F3E), static_cast<wchar_t>(0x1F3F), static_cast<wchar_t>(0x1F48),
  static_cast<wchar_t>(0x1F49), static_cast<wchar_t>(0x1F4A), static_cast<wchar_t>(0x1F4B), static_cast<wchar_t>(0x1F4C), static_cast<wchar_t>(0x1F4D), static_cast<wchar_t>(0x1F59), static_cast<wchar_t>(0x1F5B), static_cast<wchar_t>(0x1F5D), static_cast<wchar_t>(0x1F5F),
  static_cast<wchar_t>(0x1F68), static_cast<wchar_t>(0x1F69), static_cast<wchar_t>(0x1F6A), static_cast<wchar_t>(0x1F6B), static_cast<wchar_t>(0x1F6C), static_cast<wchar_t>(0x1F6D), static_cast<wchar_t>(0x1F6E), static_cast<wchar_t>(0x1F6F), static_cast<wchar_t>(0x1F88),
  static_cast<wchar_t>(0x1F89), static_cast<wchar_t>(0x1F8A), static_cast<wchar_t>(0x1F8B), static_cast<wchar_t>(0x1F8C), static_cast<wchar_t>(0x1F8D), static_cast<wchar_t>(0x1F8E), static_cast<wchar_t>(0x1F8F), static_cast<wchar_t>(0x1F98), static_cast<wchar_t>(0x1F99),
  static_cast<wchar_t>(0x1F9A), static_cast<wchar_t>(0x1F9B), static_cast<wchar_t>(0x1F9C), static_cast<wchar_t>(0x1F9D), static_cast<wchar_t>(0x1F9E), static_cast<wchar_t>(0x1F9F), static_cast<wchar_t>(0x1FA8), static_cast<wchar_t>(0x1FA9), static_cast<wchar_t>(0x1FAA),
  static_cast<wchar_t>(0x1FAB), static_cast<wchar_t>(0x1FAC), static_cast<wchar_t>(0x1FAD), static_cast<wchar_t>(0x1FAE), static_cast<wchar_t>(0x1FAF), static_cast<wchar_t>(0x1FB8), static_cast<wchar_t>(0x1FB9), static_cast<wchar_t>(0x1FD8), static_cast<wchar_t>(0x1FD9),
  static_cast<wchar_t>(0x1FE8), static_cast<wchar_t>(0x1FE9), static_cast<wchar_t>(0x24B6), static_cast<wchar_t>(0x24B7), static_cast<wchar_t>(0x24B8), static_cast<wchar_t>(0x24B9), static_cast<wchar_t>(0x24BA), static_cast<wchar_t>(0x24BB), static_cast<wchar_t>(0x24BC),
  static_cast<wchar_t>(0x24BD), static_cast<wchar_t>(0x24BE), static_cast<wchar_t>(0x24BF), static_cast<wchar_t>(0x24C0), static_cast<wchar_t>(0x24C1), static_cast<wchar_t>(0x24C2), static_cast<wchar_t>(0x24C3), static_cast<wchar_t>(0x24C4), static_cast<wchar_t>(0x24C5),
  static_cast<wchar_t>(0x24C6), static_cast<wchar_t>(0x24C7), static_cast<wchar_t>(0x24C8), static_cast<wchar_t>(0x24C9), static_cast<wchar_t>(0x24CA), static_cast<wchar_t>(0x24CB), static_cast<wchar_t>(0x24CC), static_cast<wchar_t>(0x24CD), static_cast<wchar_t>(0x24CE),
  static_cast<wchar_t>(0x24CF), static_cast<wchar_t>(0xFF21), static_cast<wchar_t>(0xFF22), static_cast<wchar_t>(0xFF23), static_cast<wchar_t>(0xFF24), static_cast<wchar_t>(0xFF25), static_cast<wchar_t>(0xFF26), static_cast<wchar_t>(0xFF27), static_cast<wchar_t>(0xFF28),
  static_cast<wchar_t>(0xFF29), static_cast<wchar_t>(0xFF2A), static_cast<wchar_t>(0xFF2B), static_cast<wchar_t>(0xFF2C), static_cast<wchar_t>(0xFF2D), static_cast<wchar_t>(0xFF2E), static_cast<wchar_t>(0xFF2F), static_cast<wchar_t>(0xFF30), static_cast<wchar_t>(0xFF31),
  static_cast<wchar_t>(0xFF32), static_cast<wchar_t>(0xFF33), static_cast<wchar_t>(0xFF34), static_cast<wchar_t>(0xFF35), static_cast<wchar_t>(0xFF36), static_cast<wchar_t>(0xFF37), static_cast<wchar_t>(0xFF38), static_cast<wchar_t>(0xFF39), static_cast<wchar_t>(0xFF3A)
};


std::string StringUtils::FormatV(const char *fmt, va_list args)
{
  if (!fmt || !fmt[0])
    return "";

  int size = FORMAT_BLOCK_SIZE;
  va_list argCopy;

  while (1) 
  {
    char *cstr = reinterpret_cast<char*>(malloc(sizeof(char) * size));
    if (!cstr)
      return "";

    va_copy(argCopy, args);
    int nActual = vsnprintf(cstr, size, fmt, argCopy);
    va_end(argCopy);

    if (nActual > -1 && nActual < size) // We got a valid result
    {
      std::string str(cstr, nActual);
      free(cstr);
      return str;
    }
    free(cstr);
#ifndef TARGET_WINDOWS
    if (nActual > -1) {                   // Exactly what we will need (glibc 2.1)
      size = nActual + 1;
    } else {                                // Let's try to double the size (glibc 2.0)
      size *= 2;
}
#else  // TARGET_WINDOWS
    va_copy(argCopy, args);
    size = _vscprintf(fmt, argCopy);
    va_end(argCopy);
    if (size < 0)
      return "";
    else
      size++; // increment for null-termination
#endif // TARGET_WINDOWS
  }

  return ""; // unreachable
}

std::wstring StringUtils::FormatV(const wchar_t *fmt, va_list args)
{
  if (!fmt || !fmt[0])
    return L"";

  int size = FORMAT_BLOCK_SIZE;
  va_list argCopy;
  
  while (1)
  {
    wchar_t *cstr = reinterpret_cast<wchar_t*>(malloc(sizeof(wchar_t) * size));
    if (!cstr)
      return L"";

    va_copy(argCopy, args);
    int nActual = vswprintf(cstr, size, fmt, argCopy);
    va_end(argCopy);
    
    if (nActual > -1 && nActual < size) // We got a valid result
    {
      std::wstring str(cstr, nActual);
      free(cstr);
      return str;
    }
    free(cstr);

#ifndef TARGET_WINDOWS
    if (nActual > -1) {                   // Exactly what we will need (glibc 2.1)
      size = nActual + 1;
    } else {                                // Let's try to double the size (glibc 2.0)
      size *= 2;
}
#else  // TARGET_WINDOWS
    va_copy(argCopy, args);
    size = _vscwprintf(fmt, argCopy);
    va_end(argCopy);
    if (size < 0)
      return L"";
    else
      size++; // increment for null-termination
#endif // TARGET_WINDOWS
  }
  
  return L"";
}

int compareWchar (const void* a, const void* b)
{
  if (*(wchar_t*)a <  *(wchar_t*)b) {
    return -1;
  } if (*(wchar_t*)a >  *(wchar_t*)b) {
    return 1;
}
  return 0;
}

wchar_t tolowerUnicode(const wchar_t& c)
{
  wchar_t* p = reinterpret_cast<wchar_t*>( bsearch (&c, unicode_uppers, sizeof(unicode_uppers) / sizeof(wchar_t), sizeof(wchar_t), compareWchar));
  if (p) {
    return *(unicode_lowers + (p - unicode_uppers));
}

  return c;
}

wchar_t toupperUnicode(const wchar_t& c)
{
  wchar_t* p = reinterpret_cast<wchar_t*>( bsearch (&c, unicode_lowers, sizeof(unicode_lowers) / sizeof(wchar_t), sizeof(wchar_t), compareWchar));
  if (p) {
    return *(unicode_uppers + (p - unicode_lowers));
}

  return c;
}

void StringUtils::ToUpper(std::string &str)
{
  std::transform(str.begin(), str.end(), str.begin(), ::toupper);
}

void StringUtils::ToUpper(std::wstring &str)
{
  transform(str.begin(), str.end(), str.begin(), toupperUnicode);
}

void StringUtils::ToLower(std::string &str)
{
  transform(str.begin(), str.end(), str.begin(), ::tolower);
}

void StringUtils::ToLower(std::wstring &str)
{
  transform(str.begin(), str.end(), str.begin(), tolowerUnicode);
}

void StringUtils::ToCapitalize(std::string &str)
{
  std::wstring wstr;
  g_charsetConverter.utf8ToW(str, wstr);
  ToCapitalize(wstr);
  g_charsetConverter.wToUTF8(wstr, str);
}

void StringUtils::ToCapitalize(std::wstring &str)
{
  const std::locale& loc = g_langInfo.GetSystemLocale();
  bool isFirstLetter = true;
  for (std::wstring::iterator it = str.begin(); it < str.end(); ++it)
  {
    // capitalize after spaces and punctuation characters (except apostrophes)
    if (std::isspace(*it, loc) || (std::ispunct(*it, loc) && *it != '\''))
      isFirstLetter = true;
    else if (isFirstLetter)
    {
      *it = std::toupper(*it, loc);
      isFirstLetter = false;
    }
  }
}

bool StringUtils::EqualsNoCase(const std::string &str1, const std::string &str2)
{
  // before we do the char-by-char comparison, first compare sizes of both strings.
  // This led to a 33% improvement in benchmarking on average. (size() just returns a member of std::string)
  if (str1.size() != str2.size())
    return false;
  return EqualsNoCase(str1.c_str(), str2.c_str());
}

bool StringUtils::EqualsNoCase(const std::string &str1, const char *s2)
{
  return EqualsNoCase(str1.c_str(), s2);
}

bool StringUtils::EqualsNoCase(const char *s1, const char *s2)
{
  char c2; // we need only one char outside the loop
  do
  {
    const char c1 = *s1++; // const local variable should help compiler to optimize
    c2 = *s2++;
    if (c1 != c2 && ::tolower(c1) != ::tolower(c2)) { // This includes the possibility that one of the characters is the null-terminator, which implies a string mismatch.
      return false;
}
  } while (c2 != '\0'); // At this point, we know c1 == c2, so there's no need to test them both.
  return true;
}

int StringUtils::CompareNoCase(const std::string &str1, const std::string &str2)
{
  return CompareNoCase(str1.c_str(), str2.c_str());
}

int StringUtils::CompareNoCase(const char *s1, const char *s2)
{
  char c2; // we need only one char outside the loop
  do
  {
    const char c1 = *s1++; // const local variable should help compiler to optimize
    c2 = *s2++;
    if (c1 != c2 && ::tolower(c1) != ::tolower(c2)) { // This includes the possibility that one of the characters is the null-terminator, which implies a string mismatch.
      return ::tolower(c1) - ::tolower(c2);
}
  } while (c2 != '\0'); // At this point, we know c1 == c2, so there's no need to test them both.
  return 0;
}

std::string StringUtils::Left(const std::string &str, size_t count)
{
  count = std::max((size_t)0, std::min(count, str.size()));
  return str.substr(0, count);
}

std::string StringUtils::Mid(const std::string &str, size_t first, size_t count /* = string::npos */)
{
  if (first + count > str.size())
    count = str.size() - first;
  
  if (first > str.size())
    return std::string();
  
  assert(first + count <= str.size());
  
  return str.substr(first, count);
}

std::string StringUtils::Right(const std::string &str, size_t count)
{
  count = std::max((size_t)0, std::min(count, str.size()));
  return str.substr(str.size() - count);
}

std::string& StringUtils::Trim(std::string &str)
{
  TrimLeft(str);
  return TrimRight(str);
}

std::string& StringUtils::Trim(std::string &str, const char* const chars)
{
  TrimLeft(str, chars);
  return TrimRight(str, chars);
}

// hack to check only first byte of UTF-8 character
// without this hack "TrimX" functions failed on Win32 and OS X with UTF-8 strings
static int isspace_c(char c)
{
  return (c & 0x80) == 0 && ::isspace(c);
}

std::string& StringUtils::TrimLeft(std::string &str)
{
  str.erase(str.begin(), std::find_if(str.begin(), str.end(), std::not1(std::ptr_fun(isspace_c))));
  return str;
}

std::string& StringUtils::TrimLeft(std::string &str, const char* const chars)
{
  size_t nidx = str.find_first_not_of(chars);
  str.erase(0, nidx);
  return str;
}

std::string& StringUtils::TrimRight(std::string &str)
{
  str.erase(std::find_if(str.rbegin(), str.rend(), std::not1(std::ptr_fun(isspace_c))).base(), str.end());
  return str;
}

std::string& StringUtils::TrimRight(std::string &str, const char* const chars)
{
  size_t nidx = str.find_last_not_of(chars);
  str.erase(str.npos == nidx ? 0 : ++nidx);
  return str;
}

int StringUtils::ReturnDigits(const std::string& str)
{
  std::stringstream ss;
  for (const auto& character : str)
  {
    if (isdigit(character))
      ss << character;
  }
  return atoi(ss.str().c_str());
}

std::string& StringUtils::RemoveDuplicatedSpacesAndTabs(std::string& str)
{
  std::string::iterator it = str.begin();
  bool onSpace = false;
  while(it != str.end())
  {
    if (*it == '\t')
      *it = ' ';

    if (*it == ' ')
    {
      if (onSpace)
      {
        it = str.erase(it);
        continue;
      }
      else
        onSpace = true;
    }
    else
      onSpace = false;

    ++it;
  }
  return str;
}

int StringUtils::Replace(std::string &str, char oldChar, char newChar)
{
  int replacedChars = 0;
  for (std::string::iterator it = str.begin(); it != str.end(); ++it)
  {
    if (*it == oldChar)
    {
      *it = newChar;
      replacedChars++;
    }
  }
  
  return replacedChars;
}

int StringUtils::Replace(std::string &str, const std::string &oldStr, const std::string &newStr)
{
  if (oldStr.empty())
    return 0;

  int replacedChars = 0;
  size_t index = 0;
  
  while (index < str.size() && (index = str.find(oldStr, index)) != std::string::npos)
  {
    str.replace(index, oldStr.size(), newStr);
    index += newStr.size();
    replacedChars++;
  }
  
  return replacedChars;
}

int StringUtils::Replace(std::wstring &str, const std::wstring &oldStr, const std::wstring &newStr)
{
  if (oldStr.empty())
    return 0;

  int replacedChars = 0;
  size_t index = 0;

  while (index < str.size() && (index = str.find(oldStr, index)) != std::string::npos)
  {
    str.replace(index, oldStr.size(), newStr);
    index += newStr.size();
    replacedChars++;
  }

  return replacedChars;
}

bool StringUtils::StartsWith(const std::string &str1, const std::string &str2)
{
  return str1.compare(0, str2.size(), str2) == 0;
}

bool StringUtils::StartsWith(const std::string &str1, const char *s2)
{
  return StartsWith(str1.c_str(), s2);
}

bool StringUtils::StartsWith(const char *s1, const char *s2)
{
  while (*s2 != '\0')
  {
    if (*s1 != *s2) {
      return false;
}
    s1++;
    s2++;
  }
  return true;
}

bool StringUtils::StartsWithNoCase(const std::string &str1, const std::string &str2)
{
  return StartsWithNoCase(str1.c_str(), str2.c_str());
}

bool StringUtils::StartsWithNoCase(const std::string &str1, const char *s2)
{
  return StartsWithNoCase(str1.c_str(), s2);
}

bool StringUtils::StartsWithNoCase(const char *s1, const char *s2)
{
  while (*s2 != '\0')
  {
    if (::tolower(*s1) != ::tolower(*s2)) {
      return false;
}
    s1++;
    s2++;
  }
  return true;
}

bool StringUtils::EndsWith(const std::string &str1, const std::string &str2)
{
  if (str1.size() < str2.size())
    return false;
  return str1.compare(str1.size() - str2.size(), str2.size(), str2) == 0;
}

bool StringUtils::EndsWith(const std::string &str1, const char *s2)
{
  size_t len2 = strlen(s2);
  if (str1.size() < len2)
    return false;
  return str1.compare(str1.size() - len2, len2, s2) == 0;
}

bool StringUtils::EndsWithNoCase(const std::string &str1, const std::string &str2)
{
  if (str1.size() < str2.size())
    return false;
  const char *s1 = str1.c_str() + str1.size() - str2.size();
  const char *s2 = str2.c_str();
  while (*s2 != '\0')
  {
    if (::tolower(*s1) != ::tolower(*s2)) {
      return false;
}
    s1++;
    s2++;
  }
  return true;
}

bool StringUtils::EndsWithNoCase(const std::string &str1, const char *s2)
{
  size_t len2 = strlen(s2);
  if (str1.size() < len2)
    return false;
  const char *s1 = str1.c_str() + str1.size() - len2;
  while (*s2 != '\0')
  {
    if (::tolower(*s1) != ::tolower(*s2)) {
      return false;
}
    s1++;
    s2++;
  }
  return true;
}

std::vector<std::string> StringUtils::Split(const std::string& input, const std::string& delimiter, unsigned int iMaxStrings /* = 0 */)
{
  std::vector<std::string> results;
  if (input.empty())
    return results;
  if (delimiter.empty())
  {
    results.push_back(input);
    return results;
  }

  const size_t delimLen = delimiter.length();
  size_t nextDelim;
  size_t textPos = 0;
  do
  {
    if (--iMaxStrings == 0)
    {
      results.push_back(input.substr(textPos));
      break;
    }
    nextDelim = input.find(delimiter, textPos);
    results.push_back(input.substr(textPos, nextDelim - textPos));
    textPos = nextDelim + delimLen;
  } while (nextDelim != std::string::npos);

  return results;
}

std::vector<std::string> StringUtils::Split(const std::string& input, const char delimiter, size_t iMaxStrings /*= 0*/)
{
  std::vector<std::string> results;
  if (input.empty())
    return results;

  size_t nextDelim;
  size_t textPos = 0;
  do
  {
    if (--iMaxStrings == 0)
    {
      results.push_back(input.substr(textPos));
      break;
    }
    nextDelim = input.find(delimiter, textPos);
    results.push_back(input.substr(textPos, nextDelim - textPos));
    textPos = nextDelim + 1;
  } while (nextDelim != std::string::npos);

  return results;
}

std::vector<std::string> StringUtils::Split(const std::string& input, const std::vector<std::string> &delimiters)
{
  std::vector<std::string> results;
  if (input.empty())
    return results;

  if (delimiters.empty())
  {
    results.push_back(input);
    return results;
  }
  std::string str = input;
  for (size_t di = 1; di < delimiters.size(); di++)
    StringUtils::Replace(str, delimiters[di], delimiters[0]);
  results = Split(str, delimiters[0]);

  return results;
}

std::vector<std::string> StringUtils::SplitMulti(const std::vector<std::string> &input, const std::vector<std::string> &delimiters, unsigned int iMaxStrings /* = 0 */)
{
  if (input.empty())
    return std::vector<std::string>(); 

  std::vector<std::string> results(input);

  if (delimiters.empty() || (iMaxStrings > 0 && iMaxStrings <= input.size()))
    return results;

  std::vector<std::string> strings1;
  if (iMaxStrings == 0)
  {
    for (size_t di = 0; di < delimiters.size(); di++)
    {
      for (size_t i = 0; i < results.size(); i++)
      {
        std::vector<std::string> substrings = StringUtils::Split(results[i], delimiters[di]);
        for (size_t j = 0; j < substrings.size(); j++)
          strings1.push_back(substrings[j]);
      }
      results = strings1;
      strings1.clear();
    }
    return results;
  }

  // Control the number of strings input is split into, keeping the original strings. 
  // Note iMaxStrings > input.size() 
  int iNew = iMaxStrings - results.size();
  for (size_t di = 0; di < delimiters.size(); di++)
  {
    for (size_t i = 0; i < results.size(); i++)
    {
      if (iNew > 0)
      {
        std::vector<std::string> substrings = StringUtils::Split(results[i], delimiters[di], iNew + 1);
        iNew = iNew - substrings.size() + 1;
        for (size_t j = 0; j < substrings.size(); j++)
          strings1.push_back(substrings[j]);
      }
      else
        strings1.push_back(results[i]);
    }
    results = strings1;
    iNew = iMaxStrings - results.size();
    strings1.clear();
    if ((iNew <= 0))
      break;  //Stop trying any more delimiters
  }
  return results;
}

// returns the number of occurrences of strFind in strInput.
int StringUtils::FindNumber(const std::string& strInput, const std::string &strFind)
{
  size_t pos = strInput.find(strFind, 0);
  int numfound = 0;
  while (pos != std::string::npos)
  {
    numfound++;
    pos = strInput.find(strFind, pos + 1);
  }
  return numfound;
}

// Compares separately the numeric and alphabetic parts of a string.
// returns negative if left < right, positive if left > right
// and 0 if they are identical (essentially calculates left - right)
int64_t StringUtils::AlphaNumericCompare(const wchar_t *left, const wchar_t *right)
{
  wchar_t *l = const_cast<wchar_t *>(left);
  wchar_t *r = const_cast<wchar_t *>(right);
  wchar_t *ld, *rd;
  wchar_t lc, rc;
  int64_t lnum, rnum;
  const std::collate<wchar_t>& coll = std::use_facet<std::collate<wchar_t> >(g_langInfo.GetSystemLocale());
  int cmp_res = 0;
  while (*l != 0 && *r != 0)
  {
    // check if we have a numerical value
    if (*l >= L'0' && *l <= L'9' && *r >= L'0' && *r <= L'9')
    {
      ld = l;
      lnum = 0;
      while (*ld >= L'0' && *ld <= L'9' && ld < l + 15)
      { // compare only up to 15 digits
        lnum *= 10;
        lnum += *ld++ - '0';
      }
      rd = r;
      rnum = 0;
      while (*rd >= L'0' && *rd <= L'9' && rd < r + 15)
      { // compare only up to 15 digits
        rnum *= 10;
        rnum += *rd++ - L'0';
      }
      // do we have numbers?
      if (lnum != rnum)
      { // yes - and they're different!
        return lnum - rnum;
      }
      l = ld;
      r = rd;
      continue;
    }
    // do case less comparison
    lc = *l;
    if (lc >= L'A' && lc <= L'Z') {
      lc += L'a'-L'A';
    
}rc = *r;
    if (rc >= L'A' && rc <= L'Z') {
      rc += L'a'- L'A';

    // ok, do a normal comparison, taking current locale into account. Add special case stuff (eg '(' characters)) in here later
    
}if ((cmp_res = coll.compare(&lc, &lc + 1, &rc, &rc + 1)) != 0)
    {
      return cmp_res;
    }
    l++; r++;
  }
  if (*r)
  { // r is longer
    return -1;
  }
  if (*l)
  { // l is longer
    return 1;
  }
  return 0; // files are the same
}

int StringUtils::DateStringToYYYYMMDD(const std::string &dateString)
{
  std::vector<std::string> days = StringUtils::Split(dateString, '-');
  if (days.size() == 1)
    return atoi(days[0].c_str());
  else if (days.size() == 2)
    return atoi(days[0].c_str())*100+atoi(days[1].c_str());
  else if (days.size() == 3)
    return atoi(days[0].c_str())*10000+atoi(days[1].c_str())*100+atoi(days[2].c_str());
  else
    return -1;
}

long StringUtils::TimeStringToSeconds(const std::string &timeString)
{
  std::string strCopy(timeString);
  StringUtils::Trim(strCopy);
  if(StringUtils::EndsWithNoCase(strCopy, " min"))
  {
    // this is imdb format of "XXX min"
    return 60 * atoi(strCopy.c_str());
  }
  else
  {
    std::vector<std::string> secs = StringUtils::Split(strCopy, ':');
    int timeInSecs = 0;
    for (unsigned int i = 0; i < 3 && i < secs.size(); i++)
    {
      timeInSecs *= 60;
      timeInSecs += atoi(secs[i].c_str());
    }
    return timeInSecs;
  }
}

std::string StringUtils::SecondsToTimeString(long lSeconds, TIME_FORMAT format)
{
  bool isNegative = lSeconds < 0;
  lSeconds = std::abs(lSeconds);
  int hh = lSeconds / 3600;
  lSeconds = lSeconds % 3600;
  int mm = lSeconds / 60;
  int ss = lSeconds % 60;

  if (format == TIME_FORMAT_GUESS) {
    format = (hh >= 1) ? TIME_FORMAT_HH_MM_SS : TIME_FORMAT_MM_SS;
}
  std::string strHMS;
  if (format & TIME_FORMAT_HH)
    strHMS += StringUtils::Format("%2.2i", hh);
  else if (format & TIME_FORMAT_H)
    strHMS += StringUtils::Format("%i", hh);
  if (format & TIME_FORMAT_MM)
    strHMS += StringUtils::Format(strHMS.empty() ? "%2.2i" : ":%2.2i", mm);
  if (format & TIME_FORMAT_SS)
    strHMS += StringUtils::Format(strHMS.empty() ? "%2.2i" : ":%2.2i", ss);
  if (isNegative)
    strHMS = "-" + strHMS;
  return strHMS;
}

bool StringUtils::IsNaturalNumber(const std::string& str)
{
  size_t i = 0, n = 0;
  // allow whitespace,digits,whitespace
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  while (i < str.size() && isdigit((unsigned char) str[i]))
  {
    i++; n++;
  }
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  return i == str.size() && n > 0;
}

bool StringUtils::IsInteger(const std::string& str)
{
  size_t i = 0, n = 0;
  // allow whitespace,-,digits,whitespace
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  if (i < str.size() && str[i] == '-')
    i++;
  while (i < str.size() && isdigit((unsigned char) str[i]))
  {
    i++; n++;
  }
  while (i < str.size() && isspace((unsigned char) str[i]))
    i++;
  return i == str.size() && n > 0;
}

int StringUtils::asciidigitvalue(char chr)
{
  if (!isasciidigit(chr)) {
    return -1;
}

  return chr - '0';
}

int StringUtils::asciixdigitvalue(char chr)
{
  int v = asciidigitvalue(chr);
  if (v >= 0) {
    return v;
}
  if (chr >= 'a' && chr <= 'f') {
    return chr - 'a' + 10;
}
  if (chr >= 'A' && chr <= 'F') {
    return chr - 'A' + 10;
}

  return -1;
}


void StringUtils::RemoveCRLF(std::string& strLine)
{
  StringUtils::TrimRight(strLine, "\n\r");
}

std::string StringUtils::SizeToString(int64_t size)
{
  std::string strLabel;
  const char prefixes[] = {' ', 'k', 'M', 'G', 'T', 'P', 'E', 'Z', 'Y'};
  unsigned int i = 0;
  double s = static_cast<double>(size);
  while (i < ARRAY_SIZE(prefixes) && s >= 1000.0)
  {
    s /= 1024.0;
    i++;
  }

  if (!i) {
    strLabel = StringUtils::Format("%.0lf B", s);
  } else if (i == ARRAY_SIZE(prefixes))
  {
    if (s >= 1000.0)
      strLabel = StringUtils::Format(">999.99 %cB", prefixes[i - 1]);
    else
      strLabel = StringUtils::Format("%.2lf %cB", s, prefixes[i - 1]);
  }
  else { if 
}(s >= 100.0)
    strLabel = StringUtils::Format("%.1lf %cB", s, prefixes[i]);
  else
    strLabel = StringUtils::Format("%.2lf %cB", s, prefixes[i]);

  return strLabel;
}

std::string StringUtils::BinaryStringToString(const std::string& in)
{
  std::string out;
  out.reserve(in.size() / 2);
  for (const char *cur = in.c_str(), *end = cur + in.size(); cur != end; ++cur) {
    if (*cur == '\\') {
      ++cur;                                                                             
      if (cur == end) {
        break;
      }
      if (isdigit(*cur)) {                                                             
        char* end;
        unsigned long num = strtol(cur, &end, 10);
        cur = end - 1;
        out.push_back(num);
        continue;
      }
    }
    out.push_back(*cur);
  }
  return out;
}

// return -1 if not, else return the utf8 char length.
int IsUTF8Letter(const unsigned char *str)
{
  // reference:
  // unicode -> utf8 table: http://www.utf8-chartable.de/
  // latin characters in unicode: http://en.wikipedia.org/wiki/Latin_characters_in_Unicode
  unsigned char ch = str[0];
  if (!ch) {
    return -1;
}
  if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
    return 1;
}
  if (!(ch & 0x80)) {
    return -1;
}
  unsigned char ch2 = str[1];
  if (!ch2) {
    return -1;
}
  // check latin 1 letter table: http://en.wikipedia.org/wiki/C1_Controls_and_Latin-1_Supplement
  if (ch == 0xC3 && ch2 >= 0x80 && ch2 <= 0xBF && ch2 != 0x97 && ch2 != 0xB7) {
    return 2;
}
  // check latin extended A table: http://en.wikipedia.org/wiki/Latin_Extended-A
  if (ch >= 0xC4 && ch <= 0xC7 && ch2 >= 0x80 && ch2 <= 0xBF) {
    return 2;
}
  // check latin extended B table: http://en.wikipedia.org/wiki/Latin_Extended-B
  // and International Phonetic Alphabet: http://en.wikipedia.org/wiki/IPA_Extensions_(Unicode_block)
  if (((ch == 0xC8 || ch == 0xC9) && ch2 >= 0x80 && ch2 <= 0xBF)
      || (ch == 0xCA && ch2 >= 0x80 && ch2 <= 0xAF)) {
    return 2;
}
  return -1;
}

size_t StringUtils::FindWords(const char *str, const char *wordLowerCase)
{
  // NOTE: This assumes word is lowercase!
  unsigned char *s = (unsigned char *)str;
  do
  {
    // start with a compare
    unsigned char *c = s;
    unsigned char *w = (unsigned char *)wordLowerCase;
    bool same = true;
    while (same && *c && *w)
    {
      unsigned char lc = *c++;
      if (lc >= 'A' && lc <= 'Z')
        lc += 'a'-'A';

      if (lc != *w++) // different
        same = false;
    }
    if (same && *w == 0)  // only the same if word has been exhausted
      return (const char *)s - str;

    // otherwise, skip current word (composed by latin letters) or number
    int l;
    if (*s >= '0' && *s <= '9')
    {
      ++s;
      while (*s >= '0' && *s <= '9') ++s;
    }
    else if ((l = IsUTF8Letter(s)) > 0)
    {
      s += l;
      while ((l = IsUTF8Letter(s)) > 0) s += l;
    }
    else
      ++s;
    while (*s && *s == ' ') s++;

    // and repeat until we're done
  } while (*s);

  return std::string::npos;
}

// assumes it is called from after the first open bracket is found
int StringUtils::FindEndBracket(const std::string &str, char opener, char closer, int startPos)
{
  int blocks = 1;
  for (unsigned int i = startPos; i < str.size(); i++)
  {
    if (str[i] == opener)
      blocks++;
    else if (str[i] == closer)
    {
      blocks--;
      if (!blocks) {
        return i;
}
    }
  }

  return (int)std::string::npos;
}

void StringUtils::WordToDigits(std::string &word)
{
  static const char word_to_letter[] = "22233344455566677778889999";
  StringUtils::ToLower(word);
  for (unsigned int i = 0; i < word.size(); ++i)
  { // NB: This assumes ascii, which probably needs extending at some  point.
    char letter = word[i];
    if ((letter >= 'a' && letter <= 'z')) // assume contiguous letter range
    {
      word[i] = word_to_letter[letter-'a'];
    }
    else if (letter < '0' || letter > '9') // We want to keep 0-9!
    {
      word[i] = ' ';  // replace everything else with a space
    }
  }
}

std::string StringUtils::CreateUUID()
{
  static GuidGenerator guidGenerator;
  auto guid = guidGenerator.newGuid();

  std::stringstream strGuid; strGuid << guid;
  return strGuid.str();
}

bool StringUtils::ValidateUUID(const std::string &uuid)
{
  CRegExp guidRE;
  guidRE.RegComp(ADDON_GUID_RE);
  return (guidRE.RegFind(uuid.c_str()) == 0);
}

double StringUtils::CompareFuzzy(const std::string &left, const std::string &right)
{
  return (0.5 + fstrcmp(left.c_str(), right.c_str(), 0.0) * (left.length() + right.length())) / 2.0;
}

int StringUtils::FindBestMatch(const std::string &str, const std::vector<std::string> &strings, double &matchscore)
{
  int best = -1;
  matchscore = 0;

  int i = 0;
  for (std::vector<std::string>::const_iterator it = strings.begin(); it != strings.end(); ++it, i++)
  {
    int maxlength = std::max(str.length(), it->length());
    double score = StringUtils::CompareFuzzy(str, *it) / maxlength;
    if (score > matchscore)
    {
      matchscore = score;
      best = i;
    }
  }
  return best;
}

bool StringUtils::ContainsKeyword(const std::string &str, const std::vector<std::string> &keywords)
{
  for (std::vector<std::string>::const_iterator it = keywords.begin(); it != keywords.end(); ++it)
  {
    if (str.find(*it) != str.npos)
      return true;
  }
  return false;
}

size_t StringUtils::utf8_strlen(const char *s)
{
  size_t length = 0;
  while (*s)
  {
    if ((*s++ & 0xC0) != 0x80)
      length++;
  }
  return length;
}

std::string StringUtils::Paramify(const std::string &param)
{
  std::string result = param;
  // escape backspaces
  StringUtils::Replace(result, "\\", "\\\\");
  // escape double quotes
  StringUtils::Replace(result, "\"", "\\\"");

  // add double quotes around the whole string
  return "\"" + result + "\"";
}

std::vector<std::string> StringUtils::Tokenize(const std::string &input, const std::string &delimiters)
{
  std::vector<std::string> tokens;
  Tokenize(input, tokens, delimiters);
  return tokens;
}

void StringUtils::Tokenize(const std::string& input, std::vector<std::string>& tokens, const std::string& delimiters)
{
  tokens.clear();
  // Skip delimiters at beginning.
  std::string::size_type dataPos = input.find_first_not_of(delimiters);
  while (dataPos != std::string::npos)
  {
    // Find next delimiter
    const std::string::size_type nextDelimPos = input.find_first_of(delimiters, dataPos);
    // Found a token, add it to the vector.
    tokens.push_back(input.substr(dataPos, nextDelimPos - dataPos));
    // Skip delimiters.  Note the "not_of"
    dataPos = input.find_first_not_of(delimiters, nextDelimPos);
  }
}

std::vector<std::string> StringUtils::Tokenize(const std::string &input, const char delimiter)
{
  std::vector<std::string> tokens;
  Tokenize(input, tokens, delimiter);
  return tokens;
}

void StringUtils::Tokenize(const std::string& input, std::vector<std::string>& tokens, const char delimiter)
{
  tokens.clear();
  // Skip delimiters at beginning.
  std::string::size_type dataPos = input.find_first_not_of(delimiter);
  while (dataPos != std::string::npos)
  {
    // Find next delimiter
    const std::string::size_type nextDelimPos = input.find(delimiter, dataPos);
    // Found a token, add it to the vector.
    tokens.push_back(input.substr(dataPos, nextDelimPos - dataPos));
    // Skip delimiters.  Note the "not_of"
    dataPos = input.find_first_not_of(delimiter, nextDelimPos);
  }
}

uint64_t StringUtils::ToUint64(std::string str, uint64_t fallback) noexcept
{
  std::istringstream iss(str);
  uint64_t result(fallback);
  iss >> result;
  return result;
}

std::string StringUtils::FormatFileSize(uint64_t bytes)
{
  const std::array<std::string, 6> units{{"B", "kB", "MB", "GB", "TB", "PB"}};
  if (bytes < 1000)
    return Format("%" PRIu64 "B", bytes);

  unsigned int i = 0;
  double value = static_cast<double>(bytes);
  while (i < static_cast<int>(units.size()) - 1 && value >= 999.5)
  {
    ++i;
    value /= 1024.0;
  }
  unsigned int decimals = value < 9.995 ? 2 : (value < 99.95 ? 1 : 0);
  auto frmt = "%.0" + Format("%u", decimals) + "f%s";
  return Format(frmt.c_str(), value, units[i].c_str());
}
