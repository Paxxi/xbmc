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

// See http://www.pkware.com/documents/casestudies/APPNOTE.TXT
#define ZIP_LOCAL_HEADER 0x04034b50
#define ZIP_DATA_RECORD_HEADER 0x08074b50
#define ZIP_CENTRAL_HEADER 0x02014b50
#define ZIP_END_CENTRAL_HEADER 0x06054b50
#define ZIP_SPLIT_ARCHIVE_HEADER 0x30304b50
#define LHDR_SIZE 30
#define CHDR_SIZE 46
#define ECDREC_SIZE 22

#include <cinttypes>
#include <map>
#include <string>
#include <vector>

class CURL;

struct SZipEntry {
  uint32_t header{0};
  uint16_t version{0};
  uint16_t flags{0};
  uint16_t method{0};
  uint16_t mod_time{0};
  uint16_t mod_date{0};
  uint32_t crc32{0};
  uint32_t csize{0}; // compressed size
  uint32_t usize{0}; // uncompressed size
  uint16_t flength{0}; // filename length
  uint16_t elength{0}; // extra field length (local file header)
  uint16_t eclength{0}; // extra field length (central file header)
  uint16_t clength{0}; // file comment length (central file header)
  uint32_t lhdrOffset{0}; // Relative offset of local header
  int64_t offset{0};         // offset in file to compressed data
  std::string name;
};

class CZipManager
{
public:
  bool GetZipList(const CURL& url, std::vector<SZipEntry>& items);
  bool GetZipEntry(const CURL& url, SZipEntry& item);
  bool ExtractArchive(const std::string& strArchive, const std::string& strPath);
  bool ExtractArchive(const CURL& archive, const std::string& strPath);
  void release(const std::string& strPath); // release resources used by list zip
  static void readHeader(const char* buffer, SZipEntry& info);
  static void readCHeader(const char* buffer, SZipEntry& info);
private:
  std::map<std::string,std::vector<SZipEntry> > mZipMap;
  std::map<std::string,int64_t> mZipDate;
};

extern CZipManager g_ZipManager;
