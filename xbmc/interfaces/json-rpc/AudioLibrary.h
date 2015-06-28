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
#include "FileItemHandler.h"

class CMusicDatabase;

namespace JSONRPC
{
  class CAudioLibrary : public CFileItemHandler
  {
  public:
    static JSONRPC_STATUS GetArtists(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetArtistDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetAlbums(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetAlbumDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetSongs(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetSongDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetGenres(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS GetRecentlyAddedAlbums(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetRecentlyAddedSongs(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetRecentlyPlayedAlbums(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetRecentlyPlayedSongs(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS SetArtistDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetAlbumDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetSongDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS Scan(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS Export(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS Clean(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static bool FillFileItem(const std::string &strFilename, CFileItemPtr &item, const KODI::UTILS::CVariant &parameterObject = KODI::UTILS::CVariant(KODI::UTILS::CVariant::VariantTypeArray));
    static bool FillFileItemList(const KODI::UTILS::CVariant &parameterObject, CFileItemList &list);

    static JSONRPC_STATUS GetAdditionalDetails(const KODI::UTILS::CVariant &parameterObject, CFileItemList &items);
    static JSONRPC_STATUS GetAdditionalAlbumDetails(const KODI::UTILS::CVariant &parameterObject, CFileItemList &items, CMusicDatabase &musicdatabase);
    static JSONRPC_STATUS GetAdditionalSongDetails(const KODI::UTILS::CVariant &parameterObject, CFileItemList &items, CMusicDatabase &musicdatabase);

  private:
    static void FillAlbumItem(const CAlbum &album, const std::string &path, CFileItemPtr &item);
    
    static bool CheckForAdditionalProperties(const KODI::UTILS::CVariant &properties, const std::set<std::string> &checkProperties, std::set<std::string> &foundProperties);
  };
}
