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
#include <stdio.h>
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

static const uint16_t CP1251asUTF16LE[] = {
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434,
  0x0435, 0x0436, 0x0437, 0x0438, 0x0439,
  0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
  0x043F, 0x0440, 0x0441, 0x0442, 0x0443,
  0x0444, 0x0445, 0x0446, 0x0447, 0x0448,
  0x0449, 0x044A, 0x044B, 0x044C, 0x044D,
  0x044E, 0x044F,
  0x0000
};

static const uint16_t CP1251asUTF16BE[] = {
  0x3004, 0x3104, 0x3204, 0x3304, 0x3404,
  0x3504, 0x3604, 0x3704, 0x3804, 0x3904,
  0x3A04, 0x3B04, 0x3C04, 0x3D04, 0x3E04,
  0x3F04, 0x4004, 0x4104, 0x4204, 0x4304,
  0x4404, 0x4504, 0x4604, 0x4704, 0x4804,
  0x4904, 0x4A04, 0x4B04, 0x4C04, 0x4D04,
  0x4E04, 0x4F04,
  0x0000
};

static const uint32_t CP1251asUTF32LE[] = {
  0x0430, 0x0431, 0x0432, 0x0433, 0x0434,
  0x0435, 0x0436, 0x0437, 0x0438, 0x0439,
  0x043A, 0x043B, 0x043C, 0x043D, 0x043E,
  0x043F, 0x0440, 0x0441, 0x0442, 0x0443,
  0x0444, 0x0445, 0x0446, 0x0447, 0x0448,
  0x0449, 0x044A, 0x044B, 0x044C, 0x044D,
  0x044E, 0x044F,
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

static const uint16_t bidiLogicalOrder_1_UTF16LE[] = {
  /* Arabic mathematical Symbols 0x1EE00 - 0x1EE1B */
  0xD83B, 0xDE00, 0xD83B, 0xDE01, 0xD83B, 0xDE02, 0xD83B, 0xDE03, 0x20,
    0xD83B, 0xDE24, 0xD83B, 0xDE05, 0xD83B, 0xDE06, 0x20,
    0xD83B, 0xDE07, 0xD83B, 0xDE08, 0xD83B, 0xDE09, 0x20,
    0xD83B, 0xDE0A, 0xD83B, 0xDE0B, 0xD83B, 0xDE0C, 0xD83B, 0xDE0D, 0x20,
    0xD83B, 0xDE0E, 0xD83B, 0xDE0F, 0xD83B, 0xDE10, 0xD83B, 0xDE11, 0x20,
    0xD83B, 0xDE12, 0xD83B, 0xDE13, 0xD83B, 0xDE14, 0xD83B, 0xDE15, 0x20,
    0xD83B, 0xDE16, 0xD83B, 0xDE17, 0xD83B, 0xDE18, 0x20,
  0xD83B, 0xDE19, 0xD83B, 0xDE1A, 0xD83B, 0xDE1B,
  0X0
};

static const uint16_t bidiLogicalOrder_2_UTF16LE[] = {
    /* Arabic mathematical Symbols - Looped Symbols, 0x1EE80 - 0x1EE9B */
  0xD83B, 0xDE80, 0xD83B, 0xDE81, 0xD83B, 0xDE82, 0xD83B, 0xDE83, 0x20,
    0xD83B, 0xDE84, 0xD83B, 0xDE85, 0xD83B, 0xDE86, 0x20,
    0xD83B, 0xDE87, 0xD83B, 0xDE88, 0xD83B, 0xDE89, 0x20,
    0xD83B, 0xDE8B, 0xD83B, 0xDE8C, 0xD83B, 0xDE8D, 0x20,
    0xD83B, 0xDE8E, 0xD83B, 0xDE8F, 0xD83B, 0xDE90, 0xD83B, 0xDE91, 0x20,
    0xD83B, 0xDE92, 0xD83B, 0xDE93, 0xD83B, 0xDE94, 0xD83B, 0xDE95, 0x20,
    0xD83B, 0xDE96, 0xD83B, 0xDE97, 0xD83B, 0xDE98, 0x20,
  0xD83B, 0xDE99, 0xD83B, 0xDE9A, 0xD83B, 0xDE9B,
  0x00
};

static const uint16_t bidiLogicalOrder_3_UTF16LE[] = {
    /* Arabic mathematical Symbols - Double-struck Symbols, 0x1EEA1 - 0x1EEBB */
  0xD83B, 0xDEA1, 0xD83B, 0xDEA2, 0xD83B, 0xDEA3, 0x20,
    0xD83B, 0xDEA5, 0xD83B, 0xDEA6, 0x20,
    0xD83B, 0xDEA7, 0xD83B, 0xDEA8, 0xD83B, 0xDEA9, 0x20,
    0xD83B, 0xDEAB, 0xD83B, 0xDEAC, 0xD83B, 0xDEAD, 0x20,
    0xD83B, 0xDEAE, 0xD83B, 0xDEAF, 0xD83B, 0xDEB0, 0xD83B, 0xDEB1, 0x20,
    0xD83B, 0xDEB2, 0xD83B, 0xDEB3, 0xD83B, 0xDEB4, 0xD83B, 0xDEB5, 0x20,
    0xD83B, 0xDEB6, 0xD83B, 0xDEB7, 0xD83B, 0xDEB8, 0x20,
  0xD83B, 0xDEB9, 0xD83B, 0xDEBA, 0xD83B, 0xDEBB,
  0x00
};

static const uint16_t bidiLogicalOrder_4_UTF16LE[] = {
    /* Arabic mathematical Symbols - Initial Symbols, 0x1EE21 - 0x1EE3B */
  0xD83B, 0xDE21, 0xD83B, 0xDE22, 0x20,
    0xD83B, 0xDE27, 0xD83B, 0xDE29, 0x20,
    0xD83B, 0xDE2A, 0xD83B, 0xDE2B, 0xD83B, 0xDE2C, 0xD83B, 0xDE2D, 0x20,
    0xD83B, 0xDE2E, 0xD83B, 0xDE2F, 0xD83B, 0xDE30, 0xD83B, 0xDE31, 0x20,
    0xD83B, 0xDE32, 0xD83B, 0xDE34, 0xD83B, 0xDE35, 0x20,
    0xD83B, 0xDE36, 0xD83B, 0xDE37, 0x20,
  0xD83B, 0xDE39, 0xD83B, 0xDE3B,
  0x00
};

static const uint16_t bidiLogicalOrder_5_UTF16LE[] = {
    /* Arabic mathematical Symbols - Tailed Symbols */
    0xD83B, 0xDE42, 0xD83B, 0xDE47, 0xD83B, 0xDE49, 0xD83B, 0xDE4B, 0x20,
    0xD83B, 0xDE4D, 0xD83B, 0xDE4E, 0xD83B, 0xDE4F, 0x20,
    0xD83B, 0xDE51, 0xD83B, 0xDE52, 0xD83B, 0xDE54, 0xD83B, 0xDE57, 0x20,
    0xD83B, 0xDE59, 0xD83B, 0xDE5B, 0xD83B, 0xDE5D, 0xD83B, 0xDE5F,
    0x00
};

//using control characters RLE and PDF to reverse text
static const uint16_t bidiLogicalOrder_6_UTF16LE[] = {
  //AB \RLEשָׁלוֹם\PDF EF
  //hebrew word shalom
  0x41, 0x42, 0x20, 0x202B, 0x05E9, 0x05DC, 0x05D5, 0x05DD, 0x202C,
  0x20, 0x45, 0x46,
  0x00
};

static const uint16_t bidiVisualOrder_1_UTF16LE[] = {
  /* Arabic mathematical Symbols 0x1EE00 - 0x1EE1B */
  0xD83B, 0xDE1B, 0xD83B, 0xDE1A, 0xD83B, 0xDE19, 0x20,
    0xD83B, 0xDE18, 0xD83B, 0xDE17, 0xD83B, 0xDE16, 0x20,
    0xD83B, 0xDE15, 0xD83B, 0xDE14, 0xD83B, 0xDE13, 0xD83B, 0xDE12, 0x20,
    0xD83B, 0xDE11, 0xD83B, 0xDE10, 0xD83B, 0xDE0F, 0xD83B, 0xDE0E, 0x20,
    0xD83B, 0xDE0D, 0xD83B, 0xDE0C, 0xD83B, 0xDE0B, 0xD83B, 0xDE0A, 0x20,
    0xD83B, 0xDE09, 0xD83B, 0xDE08, 0xD83B, 0xDE07, 0x20,
    0xD83B, 0xDE06, 0xD83B, 0xDE05, 0xD83B, 0xDE24, 0x20,
  0xD83B, 0xDE03, 0xD83B, 0xDE02, 0xD83B, 0xDE01, 0xD83B, 0xDE00,
  0x00
};

static const uint16_t bidiVisualOrder_2_UTF16LE[] = {
    /* Arabic mathematical Symbols - Looped Symbols, 0x1EE80 - 0x1EE9B */
  0xD83B, 0xDE9B, 0xD83B, 0xDE9A, 0xD83B, 0xDE99, 0x20,
    0xD83B, 0xDE98, 0xD83B, 0xDE97, 0xD83B, 0xDE96, 0x20,
    0xD83B, 0xDE95, 0xD83B, 0xDE94, 0xD83B, 0xDE93, 0xD83B, 0xDE92, 0x20,
    0xD83B, 0xDE91, 0xD83B, 0xDE90, 0xD83B, 0xDE8F, 0xD83B, 0xDE8E, 0x20,
    0xD83B, 0xDE8D, 0xD83B, 0xDE8C, 0xD83B, 0xDE8B, 0x20,
    0xD83B, 0xDE89, 0xD83B, 0xDE88, 0xD83B, 0xDE87, 0x20,
    0xD83B, 0xDE86, 0xD83B, 0xDE85, 0xD83B, 0xDE84, 0x20,
  0xD83B, 0xDE83, 0xD83B, 0xDE82, 0xD83B, 0xDE81, 0xD83B, 0xDE80,
  0x00
};

static const uint16_t bidiVisualOrder_3_UTF16LE[] = {
    /* Arabic mathematical Symbols - Double-struck Symbols, 0x1EEA1 - 0x1EEBB */
  0xD83B, 0xDEBB, 0xD83B, 0xDEBA, 0xD83B, 0xDEB9, 0x20,
    0xD83B, 0xDEB8, 0xD83B, 0xDEB7, 0xD83B, 0xDEB6, 0x20,
    0xD83B, 0xDEB5, 0xD83B, 0xDEB4, 0xD83B, 0xDEB3, 0xD83B, 0xDEB2, 0x20,
    0xD83B, 0xDEB1, 0xD83B, 0xDEB0, 0xD83B, 0xDEAF, 0xD83B, 0xDEAE, 0x20,
    0xD83B, 0xDEAD, 0xD83B, 0xDEAC, 0xD83B, 0xDEAB, 0x20,
    0xD83B, 0xDEA9, 0xD83B, 0xDEA8, 0xD83B, 0xDEA7, 0x20,
    0xD83B, 0xDEA6, 0xD83B, 0xDEA5, 0x20,
  0xD83B, 0xDEA3, 0xD83B, 0xDEA2, 0xD83B, 0xDEA1,
  0x00
};

static const uint16_t bidiVisualOrder_4_UTF16LE[] = {
    /* Arabic mathematical Symbols - Initial Symbols, 0x1EE21 - 0x1EE3B */
  0xD83B, 0xDE3B, 0xD83B, 0xDE39, 0x20,
    0xD83B, 0xDE37, 0xD83B, 0xDE36, 0x20,
    0xD83B, 0xDE35, 0xD83B, 0xDE34, 0xD83B, 0xDE32, 0x20,
    0xD83B, 0xDE31, 0xD83B, 0xDE30, 0xD83B, 0xDE2F, 0xD83B, 0xDE2E, 0x20,
    0xD83B, 0xDE2D, 0xD83B, 0xDE2C, 0xD83B, 0xDE2B, 0xD83B, 0xDE2A, 0x20,
    0xD83B, 0xDE29, 0xD83B, 0xDE27, 0x20,
  0xD83B, 0xDE22, 0xD83B, 0xDE21,
  0x00
};

static const uint16_t bidiVisualOrder_5_UTF16LE[] = {
    /* Arabic mathematical Symbols - Tailed Symbols */
    0xD83B, 0xDE5F, 0xD83B, 0xDE5D, 0xD83B, 0xDE5B, 0xD83B, 0xDE59, 0x20,
    0xD83B, 0xDE57, 0xD83B, 0xDE54, 0xD83B, 0xDE52, 0xD83B, 0xDE51, 0x20,
    0xD83B, 0xDE4F, 0xD83B, 0xDE4E, 0xD83B, 0xDE4D, 0x20,
    0xD83B, 0xDE4B, 0xD83B, 0xDE49, 0xD83B, 0xDE47, 0xD83B, 0xDE42,
    0x00
};

//using control characters RLE and PDF to reverse text
static const uint16_t bidiVisualOrder_6_UTF16LE[] = {
  //AB (hebrew word shalom) EF
  0x41, 0x42, 0x20, 0x05DD, 0x05D5, 0x05DC, 0x05E9,
  0x20, 0x45, 0x46,
  0x00
};

template<typename T>
std::string toHex(const T& str)
{
  std::string result = "\"";
  char buf[16];

  for (size_t i = 0; i < str.length(); ++i)
  {
    snprintf(buf, 16, "\\%#X", str[i]);
    buf[15] = 0;
    result.append(buf);
  }
  result.append("\"");

  return result;
}

template<typename STR>
::testing::AssertionResult AssertStringEquals(const char* exp_expr,
                                              const char* act_expr,
                                              STR& exp,
                                              STR& act) 
{
  if (exp == act)
    return ::testing::AssertionSuccess();

  return ::testing::AssertionFailure()
    << "Value of: " << act_expr << std::endl
    << "Actual: " << toHex(act) << std::endl
    << "Expected: " << exp_expr << std::endl
    << "Which is: " << toHex(exp);
}


class TestCharsetConverter : public testing::Test
{
protected:

  std::string refstra1, refstra2, varstra1;
  std::wstring refstrw1, varstrw1;
  std::u16string refstr16_1, varstr16_1;
  std::u32string refstr32_1, varstr32_1;
  std::string refstr1;
};

TEST_F(TestCharsetConverter, utf8ToSystemSafe_1)
{
  uint16_t c[] = { 0xFA1B, 0x0000 };
  std::u16string cs(c);

  std::string u8str;
  std::string u8str2;

  g_charsetConverter.utf16LEtoUTF8(cs, u8str);
  ASSERT_TRUE(g_charsetConverter.utf8ToSystemSafe(u8str, u8str2));
}

TEST_F(TestCharsetConverter, systemToUtf8_CP1251)
{
  std::string data((char*)&refCP1251);
  std::string expected((char*)&CP1251asUTF8);
  std::string temp;
  const char* defCodePage = ucnv_getDefaultName();
  ucnv_setDefaultName("CP-1251"); //simulate CP1251 as system codepage
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
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
  g_charsetConverter.systemToUtf8(data, temp);
  ucnv_setDefaultName(defCodePage); //reset codepage to avoid tainting other tests
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::string expected((char*)&refCP1251);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1251");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1252)
{
  std::string source((char*)&CP1252asUTF8);
  std::string expected((char*)&refCP1252);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1252");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1253)
{
  std::string source((char*)&CP1253asUTF8);
  std::string expected((char*)&refCP1253);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1253");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1255)
{
  std::string source((char*)&CP1255asUTF8);
  std::string expected((char*)&refCP1255);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1255");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP1256)
{
  std::string source((char*)&CP1256asUTF8);
  std::string expected((char*)&refCP1256);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-1256");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP874)
{
  std::string source((char*)&CP874asUTF8);
  std::string expected((char*)&refCP874);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-874");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP932)
{
  std::string source((char*)&CP932asUTF8);
  std::string expected((char*)&refCP932);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-932");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8ToSystem_CP932Hiranga)
{
  std::string source((char*)&CP932HirangaAsUTF8);
  std::string expected((char*)&refCP932Hiranga);
  const char* defCodePage = ucnv_getDefaultName();

  ucnv_setDefaultName("CP-932");
  g_charsetConverter.utf8ToSystem(source);
  ucnv_setDefaultName(defCodePage);
  EXPECT_STREQ(expected.c_str(), source.c_str());
}

TEST_F(TestCharsetConverter, utf8LogicalToVisual_1)
{
  std::u16string u16Source(bidiLogicalOrder_1_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_1_UTF16LE);

  std::string source;
  std::string expected;
  std::string temp;

  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Source, source));
  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Expected, expected));
  EXPECT_TRUE(g_charsetConverter.logicalToVisualBiDi(source, temp));
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf8LogicalToVisual_2)
{
  std::u16string u16Source(bidiLogicalOrder_2_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_2_UTF16LE);

  std::string source;
  std::string expected;
  std::string temp;

  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Source, source));
  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Expected, expected));
  EXPECT_TRUE(g_charsetConverter.logicalToVisualBiDi(source, temp));
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf8LogicalToVisual_3)
{
  std::u16string u16Source(bidiLogicalOrder_3_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_3_UTF16LE);

  std::string source;
  std::string expected;
  std::string temp;

  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Source, source));
  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Expected, expected));
  EXPECT_TRUE(g_charsetConverter.logicalToVisualBiDi(source, temp));
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf8LogicalToVisual_4)
{
  std::u16string u16Source(bidiLogicalOrder_4_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_4_UTF16LE);

  std::string source;
  std::string expected;
  std::string temp;

  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Source, source));
  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Expected, expected));
  EXPECT_TRUE(g_charsetConverter.logicalToVisualBiDi(source, temp));
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf8LogicalToVisual_5)
{
  std::u16string u16Source(bidiLogicalOrder_5_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_5_UTF16LE);

  std::string source;
  std::string expected;
  std::string temp;

  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Source, source));
  EXPECT_TRUE(g_charsetConverter.utf16LEtoUTF8(u16Expected, expected));
  EXPECT_TRUE(g_charsetConverter.logicalToVisualBiDi(source, temp));
  EXPECT_STREQ(expected.c_str(), temp.c_str());
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_1)
{
  std::u16string u16Source(bidiLogicalOrder_1_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_1_UTF16LE);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result);
  EXPECT_PRED_FORMAT2(AssertStringEquals, u16Expected, result);
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_2)
{
  std::u16string u16Source(bidiLogicalOrder_2_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_2_UTF16LE);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result);
  EXPECT_PRED_FORMAT2(AssertStringEquals, u16Expected, result);
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_3)
{
  std::u16string u16Source(bidiLogicalOrder_3_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_3_UTF16LE);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result);
  EXPECT_PRED_FORMAT2(AssertStringEquals, u16Expected, result);
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_4)
{
  std::u16string u16Source(bidiLogicalOrder_4_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_4_UTF16LE);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result);
  EXPECT_PRED_FORMAT2(AssertStringEquals, u16Expected, result);
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_5)
{
  std::u16string u16Source(bidiLogicalOrder_5_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_5_UTF16LE);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result);
  EXPECT_PRED_FORMAT2(AssertStringEquals, u16Expected, result);
}

TEST_F(TestCharsetConverter, utf16LogicalToVisual_6)
{
  std::u16string u16Source(bidiLogicalOrder_6_UTF16LE);
  std::u16string u16Expected(bidiVisualOrder_6_UTF16LE);
  std::u16string result;

  g_charsetConverter.logicalToVisualBiDi(u16Source, result,
                                         CCharsetConverter::LTR |
                                         CCharsetConverter::REMOVE_CONTROLS);
  EXPECT_PRED_FORMAT2(AssertStringEquals, u16Expected, result);
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
  std::u16string expected(CP1251asUTF16LE);
  std::u16string temp;

  g_charsetConverter.utf8ToUtf16(source, temp);
  EXPECT_EQ(expected.length(), temp.length());
  EXPECT_PRED_FORMAT2(AssertStringEquals, expected, temp);
}

TEST_F(TestCharsetConverter, utf8ToUtf16LE_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::u16string expected(CP1251asUTF16LE);
  std::u16string temp;

  g_charsetConverter.utf8ToUtf16LE(source, temp);
  EXPECT_EQ(expected.length(), temp.length());
  EXPECT_PRED_FORMAT2(AssertStringEquals, expected, temp);
}

TEST_F(TestCharsetConverter, utf8ToUtf16BE_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::u16string expected(CP1251asUTF16BE);
  std::u16string temp;

  g_charsetConverter.utf8ToUtf16BE(source, temp);
  EXPECT_EQ(expected.length(), temp.length());
  EXPECT_PRED_FORMAT2(AssertStringEquals, expected, temp);
}

TEST_F(TestCharsetConverter, utf8ToUtf32_CP1251)
{
  std::string source((char*)&CP1251asUTF8);
  std::u32string expected(CP1251asUTF32LE);
  std::u32string temp;

  g_charsetConverter.utf8ToUtf32(source, temp);
  EXPECT_PRED_FORMAT2(AssertStringEquals, expected, temp);
}

TEST_F(TestCharsetConverter, utf8To_UTF16LE)
{
  refstra1 = "ｔｅｓｔ＿ｕｔｆ８Ｔｏ：＿ｃｈａｒｓｅｔ＿ＵＴＦ－１６ＬＥ，＿"
             "ＣＳｔｄＳｔｒｉｎｇ１６";
  refstr16_1.assign(refutf16LE2);
  varstr16_1.clear();
  g_charsetConverter.utf8To("UTF-16LE", refstra1, varstr16_1);
  EXPECT_PRED_FORMAT2(AssertStringEquals, refstr16_1, varstr16_1);
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
  EXPECT_PRED_FORMAT2(AssertStringEquals, refstr32_1, varstr32_1);
}

TEST_F(TestCharsetConverter, stringCharsetToUtf8)
{
  refstra1 = "ｔｅｓｔ＿ｓｔｒｉｎｇＣｈａｒｓｅｔＴｏＵｔｆ８";
  varstra1.clear();
  g_charsetConverter.ToUtf8("UTF-8", refstra1, varstra1);
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

TEST_F(TestCharsetConverter, wToUTF8)
{
  refstrw1 = L"test utf8ToW";
  refstra1 = "test utf8ToW";
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
  g_charsetConverter.logicalToVisualBiDi(refstra1, varstra1);
  EXPECT_STREQ(refstra2.c_str(), varstra1.c_str());
}

//keep it here for now, no CLangInfo test cases?

//TEST_F(TestCharsetConverter, getCharsetLabels)
//{
//  std::vector<std::string> reflabels;
//  reflabels.push_back("Western Europe (ISO)");
//  reflabels.push_back("Central Europe (ISO)");
//  reflabels.push_back("South Europe (ISO)");
//  reflabels.push_back("Baltic (ISO)");
//  reflabels.push_back("Cyrillic (ISO)");
//  reflabels.push_back("Arabic (ISO)");
//  reflabels.push_back("Greek (ISO)");
//  reflabels.push_back("Hebrew (ISO)");
//  reflabels.push_back("Turkish (ISO)");
//  reflabels.push_back("Central Europe (Windows)");
//  reflabels.push_back("Cyrillic (Windows)");
//  reflabels.push_back("Western Europe (Windows)");
//  reflabels.push_back("Greek (Windows)");
//  reflabels.push_back("Turkish (Windows)");
//  reflabels.push_back("Hebrew (Windows)");
//  reflabels.push_back("Arabic (Windows)");
//  reflabels.push_back("Baltic (Windows)");
//  reflabels.push_back("Vietnamesse (Windows)");
//  reflabels.push_back("Thai (Windows)");
//  reflabels.push_back("Chinese Traditional (Big5)");
//  reflabels.push_back("Chinese Simplified (GBK)");
//  reflabels.push_back("Japanese (Shift-JIS)");
//  reflabels.push_back("Korean");
//  reflabels.push_back("Hong Kong (Big5-HKSCS)");
//
//  std::vector<std::string> varlabels = g_charsetConverter.getCharsetLabels();
//  ASSERT_EQ(reflabels.size(), varlabels.size());
//
//  std::vector<std::string>::iterator it;
//  for (it = varlabels.begin(); it < varlabels.end(); ++it)
//  {
//    EXPECT_STREQ((reflabels.at(it - varlabels.begin())).c_str(), (*it).c_str());
//  }
//}
//
//TEST_F(TestCharsetConverter, getCharsetLabelByName)
//{
//  CStdString varstr =
//    g_charsetConverter.getCharsetLabelByName("ISO-8859-1");
//  EXPECT_STREQ("Western Europe (ISO)", varstr.c_str());
//  varstr.clear();
//  varstr = g_charsetConverter.getCharsetLabelByName("Bogus");
//  EXPECT_STREQ("", varstr.c_str());
//}
//
//TEST_F(TestCharsetConverter, getCharsetNameByLabel)
//{
//  CStdString varstr =
//    g_charsetConverter.getCharsetNameByLabel("Western Europe (ISO)");
//  EXPECT_STREQ("ISO-8859-1", varstr.c_str());
//  varstr.clear();
//  varstr = g_charsetConverter.getCharsetNameByLabel("Bogus");
//  EXPECT_STREQ("", varstr.c_str());
//}

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
