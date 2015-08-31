#pragma once
/*
*      Copyright (C) 2005-2015 Team XBMC
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

#include <memory>
#include <string>

class CFileItem;
typedef std::shared_ptr<CFileItem> CFileItemPtr;

namespace GUIINFO
{
class CGUIListItemInfoHelper
{
public:
  static std::string GetLabel(const CFileItemPtr& item, int info, const std::string& fallback);

private:
  static std::string GetDirector(const CFileItemPtr& item);
  static std::string GetAlbum(const CFileItemPtr& item, const std::string& fallback);
  static std::string GetYear(const CFileItemPtr& item);
  static std::string GetPremiered(const CFileItemPtr& item);
  static std::string GetGenre(const CFileItemPtr& item);
  static std::string GetDate(const CFileItemPtr& item);
  static std::string GetRating(const CFileItemPtr& item);
  static std::string GetRatingAndVotes(const CFileItemPtr& item);
  static std::string GetDuration(const CFileItemPtr& item);
  static std::string GetPlot(const CFileItemPtr& item);
  static std::string GetPlotOutline(const CFileItemPtr& item);
  static std::string GetEpisode(const CFileItemPtr& item);
  static std::string GetSeason(const CFileItemPtr& item);
  static std::string GetIcon(const CFileItemPtr& item);
  static std::string GetFoldernameOrPath(const CFileItemPtr& item, int info);
  static std::string GetFilenameAndPath(const CFileItemPtr& item);
  static std::string GetTop250(const CFileItemPtr& item);
  static std::string GetSortLetter(const CFileItemPtr& item);
  static std::string GetAudioChannels(const CFileItemPtr& item);
  static std::string GetStartTime(const CFileItemPtr& item);
  static std::string GetEndTime(const CFileItemPtr& item);
  static std::string GetStartDate(const CFileItemPtr& item);
  static std::string GetEndDate(const CFileItemPtr& item);
  static std::string GetChannelNumber(const CFileItemPtr& item);
  static std::string GetSubChannelNumber(const CFileItemPtr& item);
  static std::string GetChannelNumberLabel(const CFileItemPtr& item);
  static std::string GetChannelName(const CFileItemPtr& item);
  static std::string GetNextStartTime(const CFileItemPtr& item);
  static std::string GetNextEndTime(const CFileItemPtr& item);
  static std::string GetNextStartDate(const CFileItemPtr& item);
  static std::string GetNextEndDate(const CFileItemPtr& item);
  static std::string GetNextPlot(const CFileItemPtr& item);
  static std::string GetNextPlotOutline(const CFileItemPtr& item);
  static std::string GetEpisodeName(const CFileItemPtr& item);
  static std::string GetNextDuration(const CFileItemPtr& item);
  static std::string GetNextGenre(const CFileItemPtr& item);
  static std::string GetNextTitle(const CFileItemPtr& item);
  static std::string GetParentalRating(const CFileItemPtr& item);
  static std::string GetDBID(const CFileItemPtr& item);
  static std::string GetStereoscopicMode(const CFileItemPtr& item);
  static std::string GetImdbNumber(const CFileItemPtr& item);
  static std::string GetFilenameOrExtentsion(const CFileItemPtr& item, int info);
  static std::string GetTitle(const CFileItemPtr& item);
  static std::string GetEpgEventTitle(const CFileItemPtr& item);
  static std::string GetOriginalTitle(const CFileItemPtr& item);
  static std::string GetPlayCount(const CFileItemPtr& item);
  static std::string GetLastPlayed(const CFileItemPtr& item, const std::string& fallback);
  static std::string GetTrackNumber(const CFileItemPtr& item);
  static std::string GetDiscNumber(const CFileItemPtr& item);
  static std::string GetArtist(const CFileItemPtr& item);
};
}