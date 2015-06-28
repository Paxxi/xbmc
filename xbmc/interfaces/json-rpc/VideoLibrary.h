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

#include "dbwrappers/DatabaseUtils.h"
#include "JSONRPC.h"
#include "FileItemHandler.h"

class CVideoDatabase;

namespace JSONRPC
{
  class CVideoLibrary : public CFileItemHandler
  {
  public:
    static JSONRPC_STATUS GetMovies(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetMovieDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetMovieSets(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetMovieSetDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS GetTVShows(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetTVShowDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetSeasons(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetSeasonDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetEpisodes(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetEpisodeDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS GetMusicVideos(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetMusicVideoDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS GetRecentlyAddedMovies(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetRecentlyAddedEpisodes(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetRecentlyAddedMusicVideos(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    
    static JSONRPC_STATUS GetGenres(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS SetMovieDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetMovieSetDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetTVShowDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetSeasonDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetEpisodeDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetMusicVideoDetails(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static JSONRPC_STATUS RemoveMovie(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS RemoveTVShow(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS RemoveEpisode(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS RemoveMusicVideo(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    
    static JSONRPC_STATUS Scan(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS Export(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS Clean(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

    static bool FillFileItem(const std::string &strFilename, CFileItemPtr &item, const KODI::UTILS::CVariant &parameterObject = KODI::UTILS::CVariant(KODI::UTILS::CVariant::VariantTypeArray));
    static bool FillFileItemList(const KODI::UTILS::CVariant &parameterObject, CFileItemList &list);

  private:
    static JSONRPC_STATUS GetAdditionalMovieDetails(const KODI::UTILS::CVariant &parameterObject, CFileItemList &items, KODI::UTILS::CVariant &result, CVideoDatabase &videodatabase, bool limit = true);
    static JSONRPC_STATUS GetAdditionalEpisodeDetails(const KODI::UTILS::CVariant &parameterObject, CFileItemList &items, KODI::UTILS::CVariant &result, CVideoDatabase &videodatabase, bool limit = true);
    static JSONRPC_STATUS GetAdditionalMusicVideoDetails(const KODI::UTILS::CVariant &parameterObject, CFileItemList &items, KODI::UTILS::CVariant &result, CVideoDatabase &videodatabase, bool limit = true);
    static JSONRPC_STATUS RemoveVideo(const KODI::UTILS::CVariant &parameterObject);
    static void UpdateVideoTag(const KODI::UTILS::CVariant &parameterObject, CVideoInfoTag &details, std::map<std::string, std::string> &artwork, std::set<std::string> &removedArtwork, std::set<std::string>& updatedDetails);
    static void UpdateVideoTagField(const KODI::UTILS::CVariant& parameterObject, const std::string& fieldName, std::vector<std::string>& fieldValue, std::set<std::string>& updatedDetails);
    static void UpdateResumePoint(const KODI::UTILS::CVariant &parameterObject, CVideoInfoTag &details, CVideoDatabase &videodatabase);
  };
}
