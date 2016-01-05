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

#include "ZipDirectory.h"

#include <memory>
#include <vector>

#include "FileItem.h"
#include "filesystem/Directorization.h"
#include "URL.h"
#include "utils/URIUtils.h"
#include "ZipManager.h"


namespace XFILE
{

  static CFileItemPtr ZipEntryToFileItem(const SZipEntry& entry, const std::string& label, const std::string& path, bool isFolder)
  {
    auto item = std::make_shared<CFileItem>(label);
    if (!isFolder)
    {
      item->m_dwSize = entry.usize;
      item->m_idepth = entry.method;
    }

    return item;
  }

  bool CZipDirectory::GetDirectory(const CURL& urlOrig, CFileItemList& items)
  {
    CURL urlZip(urlOrig);

    /* if this isn't a proper archive path, assume it's the path to a archive file */
    if (!urlOrig.IsProtocol("zip"))
      urlZip = URIUtils::CreateArchivePath("zip", urlOrig);

    std::vector<SZipEntry> zipEntries;
    if (!g_ZipManager.GetZipList(urlZip, zipEntries))
      return false;

    // prepare the ZIP entries for directorization
    DirectorizeEntries<SZipEntry> entries;
    entries.reserve(zipEntries.size());
    for (const auto& zipEntry : zipEntries)
      entries.push_back(DirectorizeEntry<SZipEntry>(zipEntry.name, zipEntry));

    // directorize the ZIP entries into files and directories
    Directorize(urlZip, entries, ZipEntryToFileItem, items);

    return true;
  }

  bool CZipDirectory::ContainsFiles(const CURL& url)
  {
    std::vector<SZipEntry> items;
    g_ZipManager.GetZipList(url, items);

    return items.size() > 1;
  }
}

