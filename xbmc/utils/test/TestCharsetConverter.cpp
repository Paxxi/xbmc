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

#include "settings/Settings.h"
#include "utils/CharsetConverter.h"
#include "utils/Utf8Utils.h"
#include "system.h"

#include <string>
#include <unicode/ucnv.h>

#include "gtest/gtest.h"

static const uint16_t refutf16LE1[] = { 0xff54, 0xff45, 0xff53, 0xff54,
                                        0xff3f, 0xff55, 0xff54, 0xff46,
                                        0xff11, 0xff16, 0xff2c, 0xff25,
                                        0xff54, 0xff4f, 0xff57, 0x0 };

static const uint16_t refutf16LE2[] = { 0xff54, 0xff45, 0xff53, 0xff54,
                                        0xff3f, 0xff55, 0xff54, 0xff46,
                                        0xff18, 0xff34, 0xff4f, 0xff1a,
                                        0xff3f, 0xff43, 0xff48, 0xff41,
                                        0xff52, 0xff53, 0xff45, 0xff54,
                                        0xff3f, 0xff35, 0xff34, 0xff26,
                                        0xff0d, 0xff11, 0xff16, 0xff2c,
                                        0xff25, 0xff0c, 0xff3f, 0xff23,
                                        0xff33, 0xff54, 0xff44, 0xff33,
                                        0xff54, 0xff52, 0xff49, 0xff4e,
                                        0xff47, 0xff11, 0xff16, 0x0 };

static const char refutf16LE3[] = "T\377E\377S\377T\377?\377S\377T\377"
                                  "R\377I\377N\377G\377#\377H\377A\377"
                                  "R\377S\377E\377T\377\064\377O\377\065"
                                  "\377T\377F\377\030\377\000";

static const uint16_t refutf16LE4[] = { 0xff54, 0xff45, 0xff53, 0xff54,
                                        0xff3f, 0xff55, 0xff54, 0xff46,
                                        0xff11, 0xff16, 0xff2c, 0xff25,
                                        0xff54, 0xff4f, 0xff35, 0xff34,
                                        0xff26, 0xff18, 0x0 };

static const uint32_t refutf32LE1[] = { 0xff54, 0xff45, 0xff53, 0xff54,
                                       0xff3f, 0xff55, 0xff54, 0xff46,
                                       0xff18, 0xff34, 0xff4f, 0xff1a,
                                       0xff3f, 0xff43, 0xff48, 0xff41,
                                       0xff52, 0xff53, 0xff45, 0xff54,
                                       0xff3f, 0xff35, 0xff34, 0xff26,
                                       0xff0d, 0xff13, 0xff12, 0xff2c,
                                       0xff25, 0xff0c, 0xff3f, 0xff23,
                                       0xff33, 0xff54, 0xff44, 0xff33,
                                       0xff54, 0xff52, 0xff49, 0xff4e,
                                       0xff47, 0xff13, 0xff12, 0xff3f,
#ifdef TARGET_DARWIN
                                       0x0 };
#else
                                       0x1f42d, 0x1f42e, 0x0 };
#endif

static const uint16_t refutf16BE[] = { 0x54ff, 0x45ff, 0x53ff, 0x54ff,
                                       0x3fff, 0x55ff, 0x54ff, 0x46ff,
                                       0x11ff, 0x16ff, 0x22ff, 0x25ff,
                                       0x54ff, 0x4fff, 0x35ff, 0x34ff,
                                       0x26ff, 0x18ff, 0x0};

static const uint16_t refucs2[] = { 0xff54, 0xff45, 0xff53, 0xff54,
                                    0xff3f, 0xff55, 0xff43, 0xff53,
                                    0xff12, 0xff54, 0xff4f, 0xff35,
                                    0xff34, 0xff26, 0xff18, 0x0 };

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1251.TXT
static const uint8_t refCP1251[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
  0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
  0xEA, 0xEB, 0xEC, 0xED, 0xEE,
  0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
  0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
  0xF9, 0xFA, 0xFB, 0xFC, 0xFD,
  0xFE, 0xFF,
  0x00
};

static const uint8_t CP1251asUTF8[] = {
  0xD0, 0xB0, 0xD0, 0xB1, 0xD0, 0xB2, 0xD0, 0xB3, 0xD0, 0xB4,
  0xD0, 0xB5, 0xD0, 0xB6, 0xD0, 0xB7, 0xD0, 0xB8, 0xD0, 0xB9,
  0xD0, 0xBA, 0xD0, 0xBB, 0xD0, 0xBC, 0xD0, 0xBD, 0xD0, 0xBE,
  0xD0, 0xBF, 0xD1, 0x80, 0xD1, 0x81, 0xD1, 0x82, 0xD1, 0x83,
  0xD1, 0x84, 0xD1, 0x85, 0xD1, 0x86, 0xD1, 0x87, 0xD1, 0x88,
  0xD1, 0x89, 0xD1, 0x8A, 0xD1, 0x8B, 0xD1, 0x8C, 0xD1, 0x8D,
  0xD1, 0x8E, 0xD1, 0x8F,
  0x00
};

static const uint16_t CP1251asUTF16[] = {
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434,
  0x0435, 0x0436, 0x0437, 0x0438, 0x0439,
  0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
  0x043F, 0x0440, 0x0441, 0x0442, 0x0443,
  0x0444, 0x0445, 0x0446, 0x0447, 0x0448,
  0x0449, 0x044A, 0x044B, 0x044C, 0x044D,
  0x044E, 0x044F,
  0x0000
};

static const uint32_t CP1251asUTF32[] = {
  0x04300000, 0x04310000, 0x04320000, 0x04330000, 0x04340000,
  0x04350000, 0x04360000, 0x04370000, 0x04380000, 0x04390000,
  0x043A0000, 0x043B0000, 0x043C0000, 0x043D0000, 0x043E0000,
  0x043F0000, 0x04400000, 0x04410000, 0x04420000, 0x04430000,
  0x04440000, 0x04450000, 0x04460000, 0x04470000, 0x04480000,
  0x04490000, 0x044A0000, 0x044B0000, 0x044C0000, 0x044D0000,
  0x044E0000, 0x044F0000,
  0x0000
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1252.TXT
static const uint8_t refCP1252[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
  0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
  0xEA, 0xEB, 0xEC, 0xED, 0xEE,
  0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
  0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
  0xF9, 0xFA, 0xFB, 0xFC, 0xFD,
  0xFE, 0xFF, 0x00
};

static const uint8_t CP1252asUTF8[] = {
  0xC3, 0xA0, 0xC3, 0xA1, 0xC3, 0xA2, 0xC3, 0xA3, 0xC3, 0xA4,
  0xC3, 0xA5, 0xC3, 0xA6, 0xC3, 0xA7, 0xC3, 0xA8, 0xC3, 0xA9,
  0xC3, 0xAA, 0xC3, 0xAB, 0xC3, 0xAC, 0xC3, 0xAD, 0xC3, 0xAE,
  0xC3, 0xAF, 0xC3, 0xB0, 0xC3, 0xB1, 0xC3, 0xB2, 0xC3, 0xB3,
  0xC3, 0xB4, 0xC3, 0xB5, 0xC3, 0xB6, 0xC3, 0xB7, 0xC3, 0xB8,
  0xC3, 0xB9, 0xC3, 0xBA, 0xC3, 0xBB, 0xC3, 0xBC, 0xC3, 0xBD,
  0xC3, 0xBE, 0xC3, 0xBF, 0x00
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1253.TXT
static const uint8_t refCP1253[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
  0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
  0xEA, 0xEB, 0xEC, 0xED, 0xEE,
  0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
  0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
  0xF9, 0xFA, 0xFB, 0xFC, 0xFD,
  0xFE, 0x00
};

static const uint8_t CP1253asUTF8[] = {
  0xCE, 0xB0, 0xCE, 0xB1, 0xCE, 0xB2, 0xCE, 0xB3, 0xCE, 0xB4,
  0xCE, 0xB5, 0xCE, 0xB6, 0xCE, 0xB7, 0xCE, 0xB8, 0xCE, 0xB9,
  0xCE, 0xBA, 0xCE, 0xBB, 0xCE, 0xBC, 0xCE, 0xBD, 0xCE, 0xBE,
  0xCE, 0xBF, 0xCF, 0x80, 0xCF, 0x81, 0xCF, 0x82, 0xCF, 0x83,
  0xCF, 0x84, 0xCF, 0x85, 0xCF, 0x86, 0xCF, 0x87, 0xCF, 0x88,
  0xCF, 0x89, 0xCF, 0x8A, 0xCF, 0x8B, 0xCF, 0x8C, 0xCF, 0x8D,
  0xCF, 0x8E, 0x00
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1255.TXT
static const uint8_t refCP1255[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4,
  0xE5, 0xE6, 0xE7, 0xE8, 0xE9,
  0xEA, 0xEB, 0xEC, 0xED, 0xEE,
  0xEF, 0xF0, 0xF1, 0xF2, 0xF3,
  0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
  0xF9, 0xFA, 0x00
};

static const uint8_t CP1255asUTF8[] = {
  0xD7, 0x90, 0xD7, 0x91, 0xD7, 0x92, 0xD7, 0x93, 0xD7, 0x94,
  0xD7, 0x95, 0xD7, 0x96, 0xD7, 0x97, 0xD7, 0x98, 0xD7, 0x99,
  0xD7, 0x9A, 0xD7, 0x9B, 0xD7, 0x9C, 0xD7, 0x9D, 0xD7, 0x9E,
  0xD7, 0x9F, 0xD7, 0xA0, 0xD7, 0xA1, 0xD7, 0xA2, 0xD7, 0xA3,
  0xD7, 0xA4, 0xD7, 0xA5, 0xD7, 0xA6, 0xD7, 0xA7, 0xD7, 0xA8,
  0xD7, 0xA9, 0xD7, 0xAA, 0x00
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP1256.TXT
static const uint8_t refCP1256[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xC0, 0xC1, 0xC2, 0xC3, 0xC4,
  0xC5, 0xC6, 0xC7, 0xC8, 0xC9,
  0xCA, 0xCB, 0xCC, 0xCD, 0xCE,
  0xCF, 0xD0, 0xD1, 0xD2, 0xD3,
  0xD4, 0xD5, 0xD6, 0xD8, 0xD9,
  0xDA, 0xDB, 0xDC, 0xDD, 0xDE,
  0xDF, 0x00
};

static const uint8_t CP1256asUTF8[] = {
  0xDB, 0x81, 0xD8, 0xA1, 0xD8, 0xA2, 0xD8, 0xA3, 0xD8, 0xA4,
  0xD8, 0xA5, 0xD8, 0xA6, 0xD8, 0xA7, 0xD8, 0xA8, 0xD8, 0xA9,
  0xD8, 0xAA, 0xD8, 0xAB, 0xD8, 0xAC, 0xD8, 0xAD, 0xD8, 0xAE,
  0xD8, 0xAF, 0xD8, 0xB0, 0xD8, 0xB1, 0xD8, 0xB2, 0xD8, 0xB3,
  0xD8, 0xB4, 0xD8, 0xB5, 0xD8, 0xB6, 0xD8, 0xB7, 0xD8, 0xB8,
  0xD8, 0xB9, 0xD8, 0xBA, 0xD9, 0x80, 0xD9, 0x81, 0xD9, 0x82,
  0xD9, 0x83, 0x00
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP874.TXT
static const uint8_t refCP874[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xE0, 0xE1, 0xE2, 0xE3,
  0xE4, 0xE5, 0xE6, 0xE7,
  0xE8, 0xE9, 0xEA, 0xEB,
  0xEC, 0xED, 0xEE, 0xEF,
  0xF0, 0xF1, 0xF2, 0xF3,
  0xF4, 0xF5, 0xF6, 0xF7,
  0xF8, 0xF9, 0xFA, 0xFB,
  0x00
};

static const uint8_t CP874asUTF8[] = {
  //uses 3 bytes
  0xE0, 0xB9, 0x80, 0xE0, 0xB9, 0x81, 0xE0, 0xB9, 0x82, 0xE0, 0xB9, 0x83,
  0xE0, 0xB9, 0x84, 0xE0, 0xB9, 0x85, 0xE0, 0xB9, 0x86, 0xE0, 0xB9, 0x87,
  0xE0, 0xB9, 0x88, 0xE0, 0xB9, 0x89, 0xE0, 0xB9, 0x8A, 0xE0, 0xB9, 0x8B,
  0xE0, 0xB9, 0x8C, 0xE0, 0xB9, 0x8D, 0xE0, 0xB9, 0x8E, 0xE0, 0xB9, 0x8F,
  0xE0, 0xB9, 0x90, 0xE0, 0xB9, 0x91, 0xE0, 0xB9, 0x92, 0xE0, 0xB9, 0x93,
  0xE0, 0xB9, 0x94, 0xE0, 0xB9, 0x95, 0xE0, 0xB9, 0x96, 0xE0, 0xB9, 0x97,
  0xE0, 0xB9, 0x98, 0xE0, 0xB9, 0x99, 0xE0, 0xB9, 0x9A, 0xE0, 0xB9, 0x9B,
  0x00
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP932.TXT
static const uint8_t refCP932[] = {
  //match column width with utf8 version to make it easier to spot any errors
  //or make changes
  0xC0, 0xC1, 0xC2, 0xC3,
  0xC4, 0xC5, 0xC6, 0xC7,
  0xC8, 0xC9, 0xCA, 0xCB,
  0xCC, 0xCD, 0xCE, 0xCF,
  0x00
};

static const uint8_t CP932asUTF8[] = {
  //uses 3 bytes
  0xEF, 0xBE, 0x80, 0xEF, 0xBE, 0x81, 0xEF, 0xBE, 0x82, 0xEF, 0xBE, 0x83,
  0xEF, 0xBE, 0x84, 0xEF, 0xBE, 0x85, 0xEF, 0xBE, 0x86, 0xEF, 0xBE, 0x87,
  0xEF, 0xBE, 0x88, 0xEF, 0xBE, 0x89, 0xEF, 0xBE, 0x8A, 0xEF, 0xBE, 0x8B,
  0xEF, 0xBE, 0x8C, 0xEF, 0xBE, 0x8D, 0xEF, 0xBE, 0x8E, 0xEF, 0xBE, 0x8F,
  0x00
};

// ftp://ftp.unicode.org/Public/MAPPINGS/VENDORS/MICSFT/WINDOWS/CP932.TXT
static const uint8_t refCP932Hiranga[] = {
  //uses 2 bytes
  0x82, 0xB0, 0x82, 0xB1, 0x82, 0xB2, 0x82, 0xB3,
  0x82, 0xB4, 0x82, 0xB5, 0x82, 0xB6, 0x82, 0xB7,
  0x82, 0xB8, 0x82, 0xB9, 0x82, 0xBA, 0x82, 0xBB,
  0x82, 0xBC, 0x82, 0xBD, 0x82, 0xBE, 0x82, 0xBF,
  0x00
};

static const uint8_t CP932HirangaAsUTF8[] = {
  //uses 3 bytes
  0xE3, 0x81, 0x92, 0xE3, 0x81, 0x93, 0xE3, 0x81, 0x94, 0xE3, 0x81, 0x95,
  0xE3, 0x81, 0x96, 0xE3, 0x81, 0x97, 0xE3, 0x81, 0x98, 0xE3, 0x81, 0x99,
  0xE3, 0x81, 0x9A, 0xE3, 0x81, 0x9B, 0xE3, 0x81, 0x9C, 0xE3, 0x81, 0x9D,
  0xE3, 0x81, 0x9E, 0xE3, 0x81, 0x9F, 0xE3, 0x81, 0xA0, 0xE3, 0x81, 0xA1,
  0x00
};

//http://ftp.unicode.org/Public/UNIDATA/BidiCharacterTest.txt
//05D0 05D1 0028 05D2 05D3 005B 0026 0065 0066 005D 002E 0029 0067 0068; 0; 0; 1 1 0 1 1 0 0 0 0 0 0 0 0 0; 1 0 2 4 3 5 6 7 8 9 10 11 12 13
//LTR
static const uint16_t bidiReorder_1_LTR_utf16BE[] = {
  0x05D0, 0x05D1, 0x0028, 0x05D2, 0x05D3,
  0x005B, 0x0026, 0x0065, 0x0066, 0x005D,
  0x002E, 0x0029, 0x0067, 0x0068,
  0x0000
};

static const uint16_t bidiReorder_1_LTR_utf16LE[] = {
  0xD005, 0xD105, 0x2800, 0xD205, 0xD305,
  0x5B00, 0x2600, 0x6500, 0x6600, 0x5D00,
  0x2E00, 0x2900, 0x6700, 0x6800,
  0x0000
};

static const uint16_t bidiReorder_1_LTR_utf16LE_Expected[] = {
  0xD105, 0xD005, 0x2800, 0xD305, 0xD205,
  0x5B00, 0x2600, 0x6500, 0x6600, 0x5D00,
  0x2E00, 0x2900, 0x6700, 0x6800,
  0x0000
};

static const uint8_t bidiReorder_1_utf8[] = {
  0x00
};

//05D0 05D1 0028 05D2 05D3 005B 0026 0065 0066 005D 002E 0029 0067 0068; 1; 1; 1 1 1 1 1 1 1 2 2 1 1 1 2 2; 12 13 11 10 9 7 8 6 5 4 3 2 1 0
//0061 0062 0063 0020 0028 0064 0065 0066 0020 0627 0628 062C 0029 0020 05D0 05D1 05D2; 0; 0; 0 0 0 0 0 0 0 0 0 1 1 1 0 0 1 1 1; 0 1 2 3 4 5 6 7 8 11 10 9 12 13 16 15 14
//0061 0062 0063 0020 0028 0064 0065 0066 0020 0627 0628 062C 0029 0020 05D0 05D1 05D2; 1; 1; 2 2 2 1 1 2 2 2 1 1 1 1 1 1 1 1 1; 16 15 14 13 12 11 10 9 8 5 6 7 4 3 0 1 2
//05D0 05D1 05D2 0020 0028 0064 0065 0066 0020 0627 0628 062C 0029 0020 0061 0062 0063; 0; 0; 1 1 1 0 0 0 0 0 0 1 1 1 0 0 0 0 0; 2 1 0 3 4 5 6 7 8 11 10 9 12 13 14 15 16
//05D0 05D1 05D2 0020 0028 0064 0065 0066 0020 0627 0628 062C 0029 0020 0061 0062 0063; 1; 1; 1 1 1 1 1 2 2 2 1 1 1 1 1 1 2 2 2; 14 15 16 13 12 11 10 9 8 5 6 7 4 3 2 1 0
//0061 0062 0063 0020 0028 0627 0628 062C 0020 0064 0065 0066 0029 0020 05D0 05D1 05D2; 0; 0; 0 0 0 0 0 1 1 1 0 0 0 0 0 0 1 1 1; 0 1 2 3 4 7 6 5 8 9 10 11 12 13 16 15 14
//0061 0062 0063 0020 0028 0627 0628 062C 0020 0064 0065 0066 0029 0020 05D0 05D1 05D2; 1; 1; 2 2 2 1 1 1 1 1 1 2 2 2 1 1 1 1 1; 16 15 14 13 12 9 10 11 8 7 6 5 4 3 0 1 2
//05D0 05D1 05D2 0020 0028 0627 0628 062C 0020 0064 0065 0066 0029 0020 0061 0062 0063; 0; 0; 1 1 1 0 0 1 1 1 0 0 0 0 0 0 0 0 0; 2 1 0 3 4 7 6 5 8 9 10 11 12 13 14 15 16
//05D0 05D1 05D2 0020 0028 0627 0628 062C 0020 0064 0065 0066 0029 0020 0061 0062 0063; 1; 1; 1 1 1 1 1 1 1 1 1 2 2 2 1 1 2 2 2; 14 15 16 13 12 9 10 11 8 7 6 5 4 3 2 1 0
//0627 0628 062C 0020 0062 006F 006F 006B 0028 0073 0029; 0; 0; 1 1 1 0 0 0 0 0 0 0 0; 2 1 0 3 4 5 6 7 8 9 10
//0627 0628 062C 0020 0062 006F 006F 006B 0028 0073 0029; 1; 1; 1 1 1 1 2 2 2 2 2 2 2; 4 5 6 7 8 9 10 3 2 1 0

class TestCharsetConverter : public testing::Test
{
protected:
  TestCharsetConverter()
  {
    /* Add default settings for locale.
     * Settings here are taken from CGUISettings::Initialize()
     */
    /* TODO
    CSettingsCategory *loc = CSettings::Get().AddCategory(7, "locale", 14090);
    CSettings::Get().AddString(loc, "locale.language",248,"english",
                            SPIN_CONTROL_TEXT);
    CSettings::Get().AddString(loc, "locale.country", 20026, "USA",
                            SPIN_CONTROL_TEXT);
    CSettings::Get().AddString(loc, "locale.charset", 14091, "DEFAULT",
                            SPIN_CONTROL_TEXT); // charset is set by the
                                                // language file

    // Add default settings for subtitles
    CSettingsCategory *sub = CSettings::Get().AddCategory(5, "subtitles", 287);
    CSettings::Get().AddString(sub, "subtitles.charset", 735, "DEFAULT",
                            SPIN_CONTROL_TEXT);
    */

    g_charsetConverter.reset();
    g_charsetConverter.clear();
  }

  ~TestCharsetConverter()
  {
    CSettings::Get().Unload();
  }

  std::string refstra1, refstra2, varstra1;
  std::wstring refstrw1, varstrw1;
  std::u16string refstr16_1, varstr16_1;
  std::u32string refstr32_1, varstr32_1;
  std::string refstr1;
};

TEST_F(TestCharsetConverter, systemToUtf8_CP1251)
{
  std::string data((char*)&refCP1251);
  std::string expected((char*)&CP1251asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-1251"); //simulate CP1251 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP1252)
{
  std::string data((char*)&refCP1252);
  std::string expected((char*)&CP1252asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-1252"); //simulate CP1252 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP1253)
{
  std::string data((char*)&refCP1253);
  std::string expected((char*)&CP1253asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-1253"); //simulate CP1253 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP1255)
{
  std::string data((char*)&refCP1255);
  std::string expected((char*)&CP1255asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-1255"); //simulate CP1255 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP1256)
{
  std::string data((char*)&refCP1256);
  std::string expected((char*)&CP1256asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-1256"); //simulate CP1256 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP874)
{
  std::string data((char*)&refCP874);
  std::string expected((char*)&CP874asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-874"); //simulate CP874 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP932)
{
  std::string data((char*)&refCP932);
  std::string expected((char*)&CP932asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-932"); //simulate CP932 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, systemToUtf8_CP932Hiranga)
{
  std::string data((char*)&refCP932Hiranga);
  std::string expected((char*)&CP932HirangaAsUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-932"); //simulate CP932 as system codepage
  g_charsetConverter.systemToUtf8(data, temp, false);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::string expected((char*)&refCP1251);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1251");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1252)
{
  std::string source((char*)&CP1252asUTF8);
  std::string expected((char*)&refCP1252);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1252");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1253)
{
  std::string source((char*)&CP1253asUTF8);
  std::string expected((char*)&refCP1253);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1253");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1255)
{
  std::string source((char*)&CP1255asUTF8);
  std::string expected((char*)&refCP1255);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1255");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1256)
{
  std::string source((char*)&CP1256asUTF8);
  std::string expected((char*)&refCP1256);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1256");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP874)
{
  std::string source((char*)&CP874asUTF8);
  std::string expected((char*)&refCP874);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-874");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP932)
{
  std::string source((char*)&CP932asUTF8);
  std::string expected((char*)&refCP932);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-932");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP932Hiranga)
{
  std::string source((char*)&CP932HirangaAsUTF8);
  std::string expected((char*)&refCP932Hiranga);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-932");
  g_charsetConverter.utf8ToSystem(source, false);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf16BEToLE)
{
  std::u16string source(bidiReorder_1_LTR_utf16BE);
  std::u16string expected(bidiReorder_1_LTR_utf16LE);
  std::string temp;
  g_charsetConverter.utf16BEtoUTF8(source, temp);
  g_charsetConverter.utf8ToUtf16(temp, source);
  EXPECT_STREQ((wchar_t*)expected.c_str(), (wchar_t*)source.c_str());
}

TEST_F(TestCharsetConverter, utf8LogicalToVisual_1)
{
  std::u16string u16Source(bidiReorder_1_LTR_utf16BE);
  std::wstring u16Expected((wchar_t*)bidiReorder_1_LTR_utf16LE_Expected);

  std::string source;
  std::string expected;
  std::string temp;

  g_charsetConverter.utf16BEtoUTF8(u16Source, source);
  g_charsetConverter.wToUTF8(u16Expected, expected, false);
  g_charsetConverter.utf8logicalToVisualBiDi(source, temp, false);
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_1)
{
  std::u16string u16Source(bidiReorder_1_LTR_utf16LE);
  std::u16string u16Expected(bidiReorder_1_LTR_utf16LE_Expected);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result, false);
  EXPECT_STREQ((wchar_t*)u16Expected.c_str(), (wchar_t*)result.c_str());
}

TEST_F(TestCharsetConverter, utf8ToW)
{
  refstra1 = "test utf8ToW";
  refstrw1 = L"test utf8ToW";
  varstrw1.clear();
  g_charsetConverter.utf8ToW(refstra1, varstrw1);
  EXPECT_STREQ(refstrw1.c_str(), varstrw1.c_str());
}

TEST_F(TestCharsetConverter, subtitleCharsetToUtf8)
{
  refstra1 = "test subtitleCharsetToW";
  varstra1.clear();
  g_charsetConverter.subtitleCharsetToUtf8(refstra1, varstra1);

  /* Assign refstra1 to refstrw1 so that we can compare */
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf8ToStringCharset_1)
{
  refstra1 = "test utf8ToStringCharset";
  varstra1.clear();
  g_charsetConverter.utf8ToStringCharset(refstra1, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf8ToStringCharset_2)
{
  refstra1 = "test utf8ToStringCharset";
  varstra1 = "test utf8ToStringCharset";
  g_charsetConverter.utf8ToStringCharset(varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem)
{
  refstra1 = "test utf8ToSystem";
  varstra1 = "test utf8ToSystem";
  g_charsetConverter.utf8ToSystem(varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf8To_ASCII)
{
  refstra1 = "test utf8To: charset ASCII, std::string";
  varstra1.clear();
  g_charsetConverter.utf8To("ASCII", refstra1, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf8ToUtf16_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::u16string expected((char16_t*)&CP1251asUTF16);
  std::u16string temp;

  g_charsetConverter.utf8ToUtf16(source, temp);
  EXPECT_EQ(expected.length(), temp.length());
  EXPECT_EQ(0, memcmp(expected.c_str(), temp.c_str(),
    expected.length() * sizeof(char16_t)));
}

TEST_F(TestCharsetConverter, utf8ToUtf32_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::u32string expected((char32_t*)&CP1251asUTF32);
  std::u32string temp;

  g_charsetConverter.utf8ToUtf32(source, temp, false);
  EXPECT_EQ(expected.length(), temp.length());
  EXPECT_EQ(0, memcmp(expected.c_str(), temp.c_str(),
    expected.length() * sizeof(char32_t)));
}

TEST_F(TestCharsetConverter, utf8To_UTF16LE)
{
  refstra1 = "ｔｅｓｔ＿ｕｔｆ８Ｔｏ：＿ｃｈａｒｓｅｔ＿ＵＴＦ－１６ＬＥ，＿"
             "ＣＳｔｄＳｔｒｉｎｇ１６";
  refstr16_1.assign(refutf16LE2);
  varstr16_1.clear();
  g_charsetConverter.utf8To("UTF-16LE", refstra1, varstr16_1);
  EXPECT_TRUE(!memcmp(refstr16_1.c_str(), varstr16_1.c_str(),
                      refstr16_1.length() * sizeof(uint16_t)));
}

TEST_F(TestCharsetConverter, utf8To_UTF32LE)
{
  refstra1 = "ｔｅｓｔ＿ｕｔｆ８Ｔｏ：＿ｃｈａｒｓｅｔ＿ＵＴＦ－３２ＬＥ，＿"
#ifdef TARGET_DARWIN
/* OSX has it's own 'special' utf-8 charset which we use (see UTF8_SOURCE in CharsetConverter.cpp)
   which is basically NFD (decomposed) utf-8.  The trouble is, it fails on the COW FACE and MOUSE FACE
   characters for some reason (possibly anything over 0x100000, or maybe there's a decomposed form of these
   that I couldn't find???)  If UTF8_SOURCE is switched to UTF-8 then this test would pass as-is, but then
   some filenames stored in utf8-mac wouldn't display correctly in the UI. */
             "ＣＳｔｄＳｔｒｉｎｇ３２＿";
#else
             "ＣＳｔｄＳｔｒｉｎｇ３２＿🐭🐮";
#endif
  refstr32_1.assign(refutf32LE1);
  varstr32_1.clear();
  g_charsetConverter.utf8To("UTF-32LE", refstra1, varstr32_1);
  EXPECT_TRUE(!memcmp(refstr32_1.c_str(), varstr32_1.c_str(),
                      sizeof(refutf32LE1)));
}

TEST_F(TestCharsetConverter, stringCharsetToUtf8)
{
  refstra1 = "ｔｅｓｔ＿ｓｔｒｉｎｇＣｈａｒｓｅｔＴｏＵｔｆ８";
  varstra1.clear();
  g_charsetConverter.ToUtf8("UTF-16LE", refutf16LE3, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, isValidUtf8_1)
{
  varstra1.clear();
  g_charsetConverter.ToUtf8("UTF-16LE", refutf16LE3, varstra1);
  EXPECT_TRUE(CUtf8Utils::isValidUtf8(varstra1.c_str()));
}

TEST_F(TestCharsetConverter, isValidUtf8_2)
{
  refstr1 = refutf16LE3;
  EXPECT_FALSE(CUtf8Utils::isValidUtf8(refstr1));
}

TEST_F(TestCharsetConverter, isValidUtf8_3)
{
  varstra1.clear();
  g_charsetConverter.ToUtf8("UTF-16LE", refutf16LE3, varstra1);
  EXPECT_TRUE(CUtf8Utils::isValidUtf8(varstra1.c_str()));
}

TEST_F(TestCharsetConverter, isValidUtf8_4)
{
  EXPECT_FALSE(CUtf8Utils::isValidUtf8(refutf16LE3));
}

/* TODO: Resolve correct input/output for this function */
// TEST_F(TestCharsetConverter, ucs2CharsetToStringCharset)
// {
//   void ucs2CharsetToStringCharset(const std::wstring& strSource,
//                                   std::string& strDest, bool swap = false);
// }

TEST_F(TestCharsetConverter, wToUTF8)
{
  refstrw1 = L"ｔｅｓｔ＿ｗＴｏＵＴＦ８";
  refstra1 = "ｔｅｓｔ＿ｗＴｏＵＴＦ８";
  varstra1.clear();
  g_charsetConverter.wToUTF8(refstrw1, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf16BEtoUTF8)
{
  refstr16_1.assign(refutf16BE);
  refstra1 = "ｔｅｓｔ＿ｕｔｆ１６ＢＥｔｏＵＴＦ８";
  varstra1.clear();
  g_charsetConverter.utf16BEtoUTF8(refstr16_1, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, ucs2ToUTF8)
{
  refstr16_1.assign(refucs2);
  refstra1 = "ｔｅｓｔ＿ｕｃｓ２ｔｏＵＴＦ８";
  varstra1.clear();
  g_charsetConverter.ucs2ToUTF8(refstr16_1, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, utf8logicalToVisualBiDi)
{
  refstra1 = "ｔｅｓｔ＿ｕｔｆ８ｌｏｇｉｃａｌＴｏＶｉｓｕａｌＢｉＤｉ";
  refstra2 = "ｔｅｓｔ＿ｕｔｆ８ｌｏｇｉｃａｌＴｏＶｉｓｕａｌＢｉＤｉ";
  varstra1.clear();
  g_charsetConverter.utf8logicalToVisualBiDi(refstra1, varstra1);
  EXPECT_STREQ(refstra2.c_str(), varstra1.c_str());
}

/* TODO: Resolve correct input/output for this function */
// TEST_F(TestCharsetConverter, utf32ToStringCharset)
// {
//   void utf32ToStringCharset(const unsigned long* strSource, std::string& strDest);
// }

TEST_F(TestCharsetConverter, getCharsetLabels)
{
  std::vector<std::string> reflabels;
  reflabels.push_back("Western Europe (ISO)");
  reflabels.push_back("Central Europe (ISO)");
  reflabels.push_back("South Europe (ISO)");
  reflabels.push_back("Baltic (ISO)");
  reflabels.push_back("Cyrillic (ISO)");
  reflabels.push_back("Arabic (ISO)");
  reflabels.push_back("Greek (ISO)");
  reflabels.push_back("Hebrew (ISO)");
  reflabels.push_back("Turkish (ISO)");
  reflabels.push_back("Central Europe (Windows)");
  reflabels.push_back("Cyrillic (Windows)");
  reflabels.push_back("Western Europe (Windows)");
  reflabels.push_back("Greek (Windows)");
  reflabels.push_back("Turkish (Windows)");
  reflabels.push_back("Hebrew (Windows)");
  reflabels.push_back("Arabic (Windows)");
  reflabels.push_back("Baltic (Windows)");
  reflabels.push_back("Vietnamesse (Windows)");
  reflabels.push_back("Thai (Windows)");
  reflabels.push_back("Chinese Traditional (Big5)");
  reflabels.push_back("Chinese Simplified (GBK)");
  reflabels.push_back("Japanese (Shift-JIS)");
  reflabels.push_back("Korean");
  reflabels.push_back("Hong Kong (Big5-HKSCS)");

  std::vector<std::string> varlabels = g_charsetConverter.getCharsetLabels();
  ASSERT_EQ(reflabels.size(), varlabels.size());

  std::vector<std::string>::iterator it;
  for (it = varlabels.begin(); it < varlabels.end(); ++it)
  {
    EXPECT_STREQ((reflabels.at(it - varlabels.begin())).c_str(), (*it).c_str());
  }
}

TEST_F(TestCharsetConverter, getCharsetLabelByName)
{
  std::string varstr =
    g_charsetConverter.getCharsetLabelByName("ISO-8859-1");
  EXPECT_STREQ("Western Europe (ISO)", varstr.c_str());
  varstr.clear();
  varstr = g_charsetConverter.getCharsetLabelByName("Bogus");
  EXPECT_STREQ("", varstr.c_str());
}

TEST_F(TestCharsetConverter, getCharsetNameByLabel)
{
  std::string varstr =
    g_charsetConverter.getCharsetNameByLabel("Western Europe (ISO)");
  EXPECT_STREQ("ISO-8859-1", varstr.c_str());
  varstr.clear();
  varstr = g_charsetConverter.getCharsetNameByLabel("Bogus");
  EXPECT_STREQ("", varstr.c_str());
}

TEST_F(TestCharsetConverter, unknownToUTF8_1)
{
  refstra1 = "ｔｅｓｔ＿ｕｎｋｎｏｗｎＴｏＵＴＦ８";
  varstra1 = "ｔｅｓｔ＿ｕｎｋｎｏｗｎＴｏＵＴＦ８";
  g_charsetConverter.unknownToUTF8(varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

TEST_F(TestCharsetConverter, unknownToUTF8_2)
{
  refstra1 = "ｔｅｓｔ＿ｕｎｋｎｏｗｎＴｏＵＴＦ８";
  varstra1.clear();
  g_charsetConverter.unknownToUTF8(refstra1, varstra1);
  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
}

//TEST_F(TestCharsetConverter, toW)
//{
//  refstra1 = "ｔｅｓｔ＿ｔｏＷ：＿ｃｈａｒｓｅｔ＿ＵＴＦ－１６ＬＥ";
//  refstrw1 = L"\xBDEF\xEF94\x85BD\xBDEF\xEF93\x94BD\xBCEF\xEFBF"
//             L"\x94BD\xBDEF\xEF8F\xB7BC\xBCEF\xEF9A\xBFBC\xBDEF"
//             L"\xEF83\x88BD\xBDEF\xEF81\x92BD\xBDEF\xEF93\x85BD"
//             L"\xBDEF\xEF94\xBFBC\xBCEF\xEFB5\xB4BC\xBCEF\xEFA6"
//             L"\x8DBC\xBCEF\xEF91\x96BC\xBCEF\xEFAC\xA5BC";
//  varstrw1.clear();
//  g_charsetConverter.toW(refstra1, varstrw1, "UTF-16LE");
//  EXPECT_STREQ(refstrw1.c_str(), varstrw1.c_str());
//}
//
//TEST_F(TestCharsetConverter, fromW)
//{
//  refstrw1 = L"\xBDEF\xEF94\x85BD\xBDEF\xEF93\x94BD\xBCEF\xEFBF"
//             L"\x86BD\xBDEF\xEF92\x8FBD\xBDEF\xEF8D\xB7BC\xBCEF"
//             L"\xEF9A\xBFBC\xBDEF\xEF83\x88BD\xBDEF\xEF81\x92BD"
//             L"\xBDEF\xEF93\x85BD\xBDEF\xEF94\xBFBC\xBCEF\xEFB5"
//             L"\xB4BC\xBCEF\xEFA6\x8DBC\xBCEF\xEF91\x96BC\xBCEF"
//             L"\xEFAC\xA5BC";
//  refstra1 = "ｔｅｓｔ＿ｆｒｏｍＷ：＿ｃｈａｒｓｅｔ＿ＵＴＦ－１６ＬＥ";
//  varstra1.clear();
//  g_charsetConverter.fromW(refstrw1, varstra1, "UTF-16LE");
//  EXPECT_STREQ(refstra1.c_str(), varstra1.c_str());
//}
