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

#include <set>

#include "JSONRPC.h"
#include "JSONUtils.h"
#include "FileItem.h"

class CThumbLoader;

namespace JSONRPC
{
  class CFileItemHandler : public CJSONUtils
  {
  protected:
    static void FillDetails(const ISerializable *info, const CFileItemPtr &item, std::set<std::string> &fields, KODI::UTILS::CVariant &result, CThumbLoader *thumbLoader = NULL);
    static void HandleFileItemList(const char *ID, bool allowFile, const char *resultname, CFileItemList &items, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result, bool sortLimit = true);
    static void HandleFileItemList(const char *ID, bool allowFile, const char *resultname, CFileItemList &items, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result, int size, bool sortLimit = true);
    static void HandleFileItem(const char *ID, bool allowFile, const char *resultname, CFileItemPtr item, const KODI::UTILS::CVariant &parameterObject, const KODI::UTILS::CVariant &validFields, KODI::UTILS::CVariant &result, bool append = true, CThumbLoader *thumbLoader = NULL);
    static void HandleFileItem(const char *ID, bool allowFile, const char *resultname, CFileItemPtr item, const KODI::UTILS::CVariant &parameterObject, const std::set<std::string> &validFields, KODI::UTILS::CVariant &result, bool append = true, CThumbLoader *thumbLoader = NULL);

    static bool FillFileItemList(const KODI::UTILS::CVariant &parameterObject, CFileItemList &list);
  private:
    static void Sort(CFileItemList &items, const KODI::UTILS::CVariant& parameterObject);
    static bool GetField(const std::string &field, const KODI::UTILS::CVariant &info, const CFileItemPtr &item, KODI::UTILS::CVariant &result, bool &fetchedArt, CThumbLoader *thumbLoader = NULL);
  };
}
