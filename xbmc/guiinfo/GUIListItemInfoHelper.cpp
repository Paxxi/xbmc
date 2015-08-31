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

#include "GUIListItemInfoHelper.h"
#include "GUIInfoLabels.h"
#include <xbmc/utils/URIUtils.h>
#include <xbmc/utils/StringUtils.h>
#include <xbmc/music/CueInfoLoader.h>
#include <xbmc/settings/Settings.h>
#include <xbmc/guilib/LocalizeStrings.h>
#include <xbmc/settings/AdvancedSettings.h>
#include <xbmc/video/VideoInfoTag.h>
#include <xbmc/music/tags/MusicInfoTag.h>
#include <xbmc/pvr/timers/PVRTimerInfoTag.h>
#include <xbmc/epg/EpgInfoTag.h>
#include <xbmc/pvr/recordings/PVRRecording.h>
#include <xbmc/guilib/StereoscopicsManager.h>
#include <xbmc/utils/CharsetConverter.h>
#include <xbmc/URL.h>

namespace GUIINFO
{
std::string CGUIListItemInfoHelper::GetDirector(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    auto tag = item->GetPVRChannelInfoTag()->GetEPGNow();
    if (tag)
      return tag->Director();
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->Director();
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return item->GetPVRTimerInfoTag()->GetEpgInfoTag()->Director();
  if (item->HasVideoInfoTag())
    return StringUtils::Join(item->GetVideoInfoTag()->m_director, g_advancedSettings.m_videoItemSeparator);

  return std::string();
}

std::string CGUIListItemInfoHelper::GetAlbum(const CFileItemPtr& item, const std::string& fallback)
{
  if (item->HasVideoInfoTag())
    return item->GetVideoInfoTag()->m_strAlbum;
  if (item->HasMusicInfoTag())
    return item->GetMusicInfoTag()->GetAlbum();

  return fallback;
}

std::string CGUIListItemInfoHelper::GetYear(const CFileItemPtr& item)
{
  std::string year;
  if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_iYear > 0)
    year = StringUtils::Format("%i", item->GetVideoInfoTag()->m_iYear);
  if (item->HasMusicInfoTag())
    year = item->GetMusicInfoTag()->GetYearString();
  if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->Year() > 0)
    year = StringUtils::Format("%i", item->GetEPGInfoTag()->Year());
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRTimerInfoTag()->GetEpgInfoTag());
    if (tag->Year() > 0)
      year = StringUtils::Format("%i", tag->Year());
  }
  if (item->HasPVRRecordingInfoTag() && item->GetPVRRecordingInfoTag()->m_iYear > 0)
    year = StringUtils::Format("%i", item->GetPVRRecordingInfoTag()->m_iYear);
  return year;
}

std::string CGUIListItemInfoHelper::GetPremiered(const CFileItemPtr& item)
{
  if (item->HasVideoInfoTag())
  {
    CDateTime dateTime;
    if (item->GetVideoInfoTag()->m_firstAired.IsValid())
      dateTime = item->GetVideoInfoTag()->m_firstAired;
    else if (item->GetVideoInfoTag()->m_premiered.IsValid())
      dateTime = item->GetVideoInfoTag()->m_premiered;

    if (dateTime.IsValid())
      return dateTime.GetAsLocalizedDate();
  }
  else if (item->HasEPGInfoTag())
  {
    if (item->GetEPGInfoTag()->FirstAiredAsLocalTime().IsValid())
      return item->GetEPGInfoTag()->FirstAiredAsLocalTime().GetAsLocalizedDate(true);
  }
  else if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRTimerInfoTag()->GetEpgInfoTag());
    if (tag->FirstAiredAsLocalTime().IsValid())
      return tag->FirstAiredAsLocalTime().GetAsLocalizedDate(true);
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetGenre(const CFileItemPtr& item)
{
  if (item->HasPVRRecordingInfoTag())
    return StringUtils::Join(item->GetPVRRecordingInfoTag()->m_genre, g_advancedSettings.m_videoItemSeparator);
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr epgTag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return epgTag ? StringUtils::Join(epgTag->Genre(), g_advancedSettings.m_videoItemSeparator) : "";
  }
  if (item->HasEPGInfoTag())
    return StringUtils::Join(item->GetEPGInfoTag()->Genre(), g_advancedSettings.m_videoItemSeparator);
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return StringUtils::Join(item->GetPVRTimerInfoTag()->GetEpgInfoTag()->Genre(), g_advancedSettings.m_videoItemSeparator);
  if (item->HasVideoInfoTag())
    return StringUtils::Join(item->GetVideoInfoTag()->m_genre, g_advancedSettings.m_videoItemSeparator);
  if (item->HasMusicInfoTag())
    return StringUtils::Join(item->GetMusicInfoTag()->GetGenre(), g_advancedSettings.m_musicItemSeparator);

  return std::string();
}

std::string CGUIListItemInfoHelper::GetDate(const CFileItemPtr& item)
{
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->StartAsLocalTime().GetAsLocalizedDateTime(false, false);
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr epgTag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return epgTag ? epgTag->StartAsLocalTime().GetAsLocalizedDateTime(false, false) : CDateTime::GetCurrentDateTime().GetAsLocalizedDateTime(false, false);
  }
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->RecordingTimeAsLocalTime().GetAsLocalizedDateTime(false, false);
  if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->Summary();
  if (item->m_dateTime.IsValid())
    return item->m_dateTime.GetAsLocalizedDate();

  return std::string();
}

std::string CGUIListItemInfoHelper::GetRating(const CFileItemPtr& item)
{
  std::string rating;
  if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_fRating > 0.f) // movie rating
    rating = StringUtils::Format("%.1f", item->GetVideoInfoTag()->m_fRating);
  else if (item->HasMusicInfoTag() && item->GetMusicInfoTag()->GetRating() > '0')
  { // song rating.  Images will probably be better than numbers for this in the long run
    rating.assign(1, item->GetMusicInfoTag()->GetRating());
  }
  return rating;
}

std::string CGUIListItemInfoHelper::GetRatingAndVotes(const CFileItemPtr& item)
{
  if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_fRating > 0.f) // movie rating
  {
    std::string strRatingAndVotes;
    if (item->GetVideoInfoTag()->m_strVotes.empty())
      strRatingAndVotes = StringUtils::Format("%.1f",
                                              item->GetVideoInfoTag()->m_fRating);
    else
      strRatingAndVotes = StringUtils::Format("%.1f (%s %s)",
                                              item->GetVideoInfoTag()->m_fRating,
                                              item->GetVideoInfoTag()->m_strVotes.c_str(),
                                              g_localizeStrings.Get(20350).c_str());
    return strRatingAndVotes;
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetDuration(const CFileItemPtr& item)
{
  std::string duration;
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? StringUtils::SecondsToTimeString(tag->GetDuration()) : "";
  }
  else if (item->HasPVRRecordingInfoTag())
  {
    if (item->GetPVRRecordingInfoTag()->GetDuration() > 0)
      duration = StringUtils::SecondsToTimeString(item->GetPVRRecordingInfoTag()->GetDuration());
  }
  else if (item->HasEPGInfoTag())
  {
    if (item->GetEPGInfoTag()->GetDuration() > 0)
      duration = StringUtils::SecondsToTimeString(item->GetEPGInfoTag()->GetDuration());
  }
  else if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRTimerInfoTag()->GetEpgInfoTag());
    if (tag->GetDuration() > 0)
      duration = StringUtils::SecondsToTimeString(tag->GetDuration());
  }
  else if (item->HasVideoInfoTag())
  {
    if (item->GetVideoInfoTag()->GetDuration() > 0)
      duration = StringUtils::Format("%d", item->GetVideoInfoTag()->GetDuration() / 60);
  }
  else if (item->HasMusicInfoTag())
  {
    if (item->GetMusicInfoTag()->GetDuration() > 0)
      duration = StringUtils::SecondsToTimeString(item->GetMusicInfoTag()->GetDuration());
  }
  return duration;
}

std::string CGUIListItemInfoHelper::GetPlot(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? tag->Plot() : "";
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->Plot();
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->m_strPlot;
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return item->GetPVRTimerInfoTag()->GetEpgInfoTag()->Plot();
  if (item->HasVideoInfoTag())
  {
    if (item->GetVideoInfoTag()->m_type != MediaTypeTvShow)
      if (item->GetVideoInfoTag()->m_playCount == 0 && !CSettings::GetInstance().GetBool(CSettings::SETTING_VIDEOLIBRARY_SHOWUNWATCHEDPLOTS))
        return g_localizeStrings.Get(20370);

    return item->GetVideoInfoTag()->m_strPlot;
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetPlotOutline(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? tag->PlotOutline() : "";
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->PlotOutline();
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->m_strPlotOutline;
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return item->GetPVRTimerInfoTag()->GetEpgInfoTag()->PlotOutline();
  if (item->HasVideoInfoTag())
    return item->GetVideoInfoTag()->m_strPlotOutline;

  return std::string();
}

std::string CGUIListItemInfoHelper::GetEpisode(const CFileItemPtr& item)
{
  int iSeason = -1, iEpisode = -1;
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    if (tag)
    {
      if (tag->SeriesNumber() > 0)
        iSeason = tag->SeriesNumber();
      if (tag->EpisodeNumber() > 0)
        iEpisode = tag->EpisodeNumber();
    }
  }
  else if (item->HasEPGInfoTag())
  {
    if (item->GetEPGInfoTag()->SeriesNumber() > 0)
      iSeason = item->GetEPGInfoTag()->SeriesNumber();
    if (item->GetEPGInfoTag()->EpisodeNumber() > 0)
      iEpisode = item->GetEPGInfoTag()->EpisodeNumber();
  }
  else if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRTimerInfoTag()->GetEpgInfoTag());
    if (tag->SeriesNumber() > 0)
      iSeason = tag->SeriesNumber();
    if (tag->EpisodeNumber() > 0)
      iEpisode = tag->EpisodeNumber();
  }
  else if (item->HasPVRRecordingInfoTag() && item->GetPVRRecordingInfoTag()->m_iEpisode > 0)
  {
    iSeason = item->GetPVRRecordingInfoTag()->m_iSeason;
    iEpisode = item->GetPVRRecordingInfoTag()->m_iEpisode;
  }
  else if (item->HasVideoInfoTag())
  {
    iSeason = item->GetVideoInfoTag()->m_iSeason;
    iEpisode = item->GetVideoInfoTag()->m_iEpisode;
  }

  if (iEpisode >= 0)
  {
    if (iSeason == 0) // prefix episode with 'S'
      return StringUtils::Format("S%d", iEpisode);
    else
      return StringUtils::Format("%d", iEpisode);
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetSeason(const CFileItemPtr& item)
{
  int iSeason = -1;
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    if (tag && tag->SeriesNumber() > 0)
      iSeason = tag->SeriesNumber();
  }
  else if (item->HasEPGInfoTag() &&
    item->GetEPGInfoTag()->SeriesNumber() > 0)
    iSeason = item->GetEPGInfoTag()->SeriesNumber();
  else if (item->HasPVRTimerInfoTag() &&
    item->GetPVRTimerInfoTag()->HasEpgInfoTag() &&
    item->GetPVRTimerInfoTag()->GetEpgInfoTag()->SeriesNumber() > 0)
    iSeason = item->GetPVRTimerInfoTag()->GetEpgInfoTag()->SeriesNumber();
  else if (item->HasPVRRecordingInfoTag() &&
    item->GetPVRRecordingInfoTag()->m_iSeason > 0)
    iSeason = item->GetPVRRecordingInfoTag()->m_iSeason;
  else if (item->HasVideoInfoTag())
    iSeason = item->GetVideoInfoTag()->m_iSeason;

  if (iSeason >= 0)
    return StringUtils::Format("%d", iSeason);

  return std::string();
}

std::string CGUIListItemInfoHelper::GetIcon(const CFileItemPtr& item)
{
  std::string strThumb = item->GetArt("thumb");
  if (strThumb.empty())
    strThumb = item->GetIconImage();
  return strThumb;
}

std::string CGUIListItemInfoHelper::GetFoldernameOrPath(const CFileItemPtr& item, int info)
{
  std::string path;
  if (item->IsMusicDb() && item->HasMusicInfoTag())
    path = URIUtils::GetDirectory(item->GetMusicInfoTag()->GetURL());
  else if (item->IsVideoDb() && item->HasVideoInfoTag())
  {
    if (item->m_bIsFolder)
      path = item->GetVideoInfoTag()->m_strPath;
    else
      URIUtils::GetParentPath(item->GetVideoInfoTag()->m_strFileNameAndPath, path);
  }
  else
    URIUtils::GetParentPath(item->GetPath(), path);
  path = CURL(path).GetWithoutUserDetails();
  if (info == LISTITEM_FOLDERNAME)
  {
    URIUtils::RemoveSlashAtEnd(path);
    path = URIUtils::GetFileName(path);
  }
  return path;
}

std::string CGUIListItemInfoHelper::GetFilenameAndPath(const CFileItemPtr& item)
{
  std::string path;
  if (item->IsMusicDb() && item->HasMusicInfoTag())
    path = item->GetMusicInfoTag()->GetURL();
  else if (item->IsVideoDb() && item->HasVideoInfoTag())
    path = item->GetVideoInfoTag()->m_strFileNameAndPath;
  else
    path = item->GetPath();
  path = CURL(path).GetWithoutUserDetails();
  return path;
}

std::string CGUIListItemInfoHelper::GetTop250(const CFileItemPtr& item)
{
  if (item->HasVideoInfoTag())
  {
    std::string strResult;
    if (item->GetVideoInfoTag()->m_iTop250 > 0)
      strResult = StringUtils::Format("%i", item->GetVideoInfoTag()->m_iTop250);
    return strResult;
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetSortLetter(const CFileItemPtr& item)
{
  std::string letter;
  std::wstring character(1, item->GetSortLabel()[0]);
  StringUtils::ToUpper(character);
  g_charsetConverter.wToUTF8(character, letter);
  return letter;
}

std::string CGUIListItemInfoHelper::GetAudioChannels(const CFileItemPtr& item)
{
  if (item->HasVideoInfoTag())
  {
    std::string strResult;
    int iChannels = item->GetVideoInfoTag()->m_streamDetails.GetAudioChannels();
    if (iChannels > -1)
      strResult = StringUtils::Format("%i", iChannels);
    return strResult;
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetStartTime(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? tag->StartAsLocalTime().GetAsLocalizedTime("", false) : CDateTime::GetCurrentDateTime().GetAsLocalizedTime("", false);
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->StartAsLocalTime().GetAsLocalizedTime("", false);
  if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->StartAsLocalTime().GetAsLocalizedTime("", false);
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->RecordingTimeAsLocalTime().GetAsLocalizedTime("", false);
  if (item->m_dateTime.IsValid())
    return item->m_dateTime.GetAsLocalizedTime("", false);

  return std::string();
}

std::string CGUIListItemInfoHelper::GetEndTime(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    auto tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? tag->EndAsLocalTime().GetAsLocalizedTime("", false) : CDateTime::GetCurrentDateTime().GetAsLocalizedTime("", false);
  }
  else if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->EndAsLocalTime().GetAsLocalizedTime("", false);
  else if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->EndAsLocalTime().GetAsLocalizedTime("", false);
  else if (item->HasVideoInfoTag())
  {
    CDateTimeSpan duration(0, 0, 0, item->GetVideoInfoTag()->GetDuration());
    return (CDateTime::GetCurrentDateTime() + duration).GetAsLocalizedTime("", false);
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetStartDate(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? tag->StartAsLocalTime().GetAsLocalizedDate(true) : CDateTime::GetCurrentDateTime().GetAsLocalizedDate(true);
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->StartAsLocalTime().GetAsLocalizedDate(true);
  if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->StartAsLocalTime().GetAsLocalizedDate(true);
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->RecordingTimeAsLocalTime().GetAsLocalizedDate(true);
  if (item->m_dateTime.IsValid())
    return item->m_dateTime.GetAsLocalizedDate(true);

  return std::string();
}

std::string CGUIListItemInfoHelper::GetEndDate(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return tag ? tag->EndAsLocalTime().GetAsLocalizedDate(true) : CDateTime::GetCurrentDateTime().GetAsLocalizedDate(true);
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->EndAsLocalTime().GetAsLocalizedDate(true);
  if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->EndAsLocalTime().GetAsLocalizedDate(true);

  return std::string();
}

std::string CGUIListItemInfoHelper::GetChannelNumber(const CFileItemPtr& item)
{
  std::string number;
  if (item->HasPVRChannelInfoTag())
    number = StringUtils::Format("%i", item->GetPVRChannelInfoTag()->ChannelNumber());
  if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->HasPVRChannel())
    number = StringUtils::Format("%i", item->GetEPGInfoTag()->PVRChannelNumber());
  if (item->HasPVRTimerInfoTag())
    number = StringUtils::Format("%i", item->GetPVRTimerInfoTag()->ChannelNumber());

  return number;
}

std::string CGUIListItemInfoHelper::GetSubChannelNumber(const CFileItemPtr& item)
{
  std::string number;
  if (item->HasPVRChannelInfoTag())
    number = StringUtils::Format("%i", item->GetPVRChannelInfoTag()->SubChannelNumber());
  if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->HasPVRChannel())
    number = StringUtils::Format("%i", item->GetEPGInfoTag()->ChannelTag()->SubChannelNumber());
  if (item->HasPVRTimerInfoTag())
    number = StringUtils::Format("%i", item->GetPVRTimerInfoTag()->ChannelTag()->SubChannelNumber());

  return number;
}

std::string CGUIListItemInfoHelper::GetChannelNumberLabel(const CFileItemPtr& item)
{
  PVR::CPVRChannelPtr channel;
  if (item->HasPVRChannelInfoTag())
    channel = item->GetPVRChannelInfoTag();
  else if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->HasPVRChannel())
    channel = item->GetEPGInfoTag()->ChannelTag();
  else if (item->HasPVRTimerInfoTag())
    channel = item->GetPVRTimerInfoTag()->ChannelTag();

  return channel ?
           channel->FormattedChannelNumber() :
           "";
}

std::string CGUIListItemInfoHelper::GetChannelName(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
    return item->GetPVRChannelInfoTag()->ChannelName();
  if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->HasPVRChannel())
    return item->GetEPGInfoTag()->PVRChannelName();
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->m_strChannelName;
  if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->ChannelName();

  return std::string();
}

std::string CGUIListItemInfoHelper::GetNextStartTime(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->StartAsLocalTime().GetAsLocalizedTime("", false);
  }
  return CDateTime::GetCurrentDateTime().GetAsLocalizedTime("", false);
}

std::string CGUIListItemInfoHelper::GetNextEndTime(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->EndAsLocalTime().GetAsLocalizedTime("", false);
  }
  return CDateTime::GetCurrentDateTime().GetAsLocalizedTime("", false);
}

std::string CGUIListItemInfoHelper::GetNextStartDate(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->StartAsLocalTime().GetAsLocalizedDate(true);
  }
  return CDateTime::GetCurrentDateTime().GetAsLocalizedDate(true);
}

std::string CGUIListItemInfoHelper::GetNextEndDate(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->EndAsLocalTime().GetAsLocalizedDate(true);
  }
  return CDateTime::GetCurrentDateTime().GetAsLocalizedDate(true);
}

std::string CGUIListItemInfoHelper::GetNextPlot(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->Plot();
  }
  return "";
}

std::string CGUIListItemInfoHelper::GetNextPlotOutline(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->PlotOutline();
  }
  return "";
}

std::string CGUIListItemInfoHelper::GetEpisodeName(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    if (tag)
      return tag->EpisodeName();
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->EpisodeName();
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return item->GetPVRTimerInfoTag()->GetEpgInfoTag()->EpisodeName();
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->EpisodeName();

  return std::string();
}

std::string CGUIListItemInfoHelper::GetNextDuration(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return StringUtils::SecondsToTimeString(tag->GetDuration());
  }
  return "";
}

std::string CGUIListItemInfoHelper::GetNextGenre(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return StringUtils::Join(tag->Genre(), g_advancedSettings.m_videoItemSeparator);
  }
  return "";
}

std::string CGUIListItemInfoHelper::GetNextTitle(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNext());
    if (tag)
      return tag->Title();
  }
  return "";
}

std::string CGUIListItemInfoHelper::GetParentalRating(const CFileItemPtr& item)
{
  std::string rating;
  if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->ParentalRating() > 0)
    rating = StringUtils::Format("%i", item->GetEPGInfoTag()->ParentalRating());
  return rating;
}

std::string CGUIListItemInfoHelper::GetDBID(const CFileItemPtr& item)
{
  if (item->HasVideoInfoTag())
  {
    return StringUtils::Format("%i", item->GetVideoInfoTag()->m_iDbId);
  }
  if (item->HasMusicInfoTag())
  {
    return StringUtils::Format("%i", item->GetMusicInfoTag()->GetDatabaseId());
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetStereoscopicMode(const CFileItemPtr& item)
{
  std::string stereoMode = item->GetProperty("stereomode").asString();
  if (stereoMode.empty() && item->HasVideoInfoTag())
    stereoMode = CStereoscopicsManager::GetInstance().NormalizeStereoMode(item->GetVideoInfoTag()->m_streamDetails.GetStereoMode());
  return stereoMode;
}

std::string CGUIListItemInfoHelper::GetImdbNumber(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    if (tag)
      return tag->IMDBNumber();
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->IMDBNumber();
  if (item->HasVideoInfoTag())
    return item->GetVideoInfoTag()->m_strIMDBNumber;
  return;
}

std::string CGUIListItemInfoHelper::GetLabel(const CFileItemPtr& item, int info, const std::string& fallback)
{
  switch (info)
  {
  case LISTITEM_LABEL:
    return item->GetLabel();

  case LISTITEM_LABEL2:
    return item->GetLabel2();

  case LISTITEM_TITLE:
    return GetTitle(item);

  case LISTITEM_EPG_EVENT_TITLE:
    return GetEpgEventTitle(item);

  case LISTITEM_ORIGINALTITLE:
    return GetOriginalTitle(item);

  case LISTITEM_PLAYCOUNT:
    return GetPlayCount(item);

  case LISTITEM_LASTPLAYED:
    return GetLastPlayed(item, fallback);

  case LISTITEM_TRACKNUMBER:
    return GetTrackNumber(item);

  case LISTITEM_DISC_NUMBER:
    return GetDiscNumber(item);

  case LISTITEM_ARTIST:
    return GetArtist(item);

  case LISTITEM_ALBUM_ARTIST:
    if (item->HasMusicInfoTag())
      return StringUtils::Join(item->GetMusicInfoTag()->GetAlbumArtist(), g_advancedSettings.m_musicItemSeparator);
    return fallback;

  case LISTITEM_DIRECTOR:
    return GetDirector(item);

  case LISTITEM_ALBUM:
    return GetAlbum(item, fallback);

  case LISTITEM_YEAR:
    return GetYear(item);

  case LISTITEM_PREMIERED:
    return GetPremiered(item);

  case LISTITEM_GENRE:
    return GetGenre(item);

  case LISTITEM_FILENAME:
  case LISTITEM_FILE_EXTENSION:
    return GetFilenameOrExtentsion(item, info);

  case LISTITEM_DATE:
    return GetDate(item);

  case LISTITEM_SIZE:
    if (!item->m_bIsFolder || item->m_dwSize)
      return StringUtils::SizeToString(item->m_dwSize);
    return fallback;

  case LISTITEM_RATING:
    return GetRating(item);

  case LISTITEM_RATING_AND_VOTES:
    return GetRatingAndVotes(item);

  case LISTITEM_VOTES:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_strVotes;
    return fallback;

  case LISTITEM_PROGRAM_COUNT:
    return StringUtils::Format("%i", item->m_iprogramCount);

  case LISTITEM_DURATION:
    return GetDuration(item);

  case LISTITEM_PLOT:
    return GetPlot(item);

  case LISTITEM_PLOT_OUTLINE:
    return GetPlotOutline(item);

  case LISTITEM_EPISODE:
    return GetEpisode(item);

  case LISTITEM_SEASON:
    return GetSeason(item);

  case LISTITEM_TVSHOW:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_strShowTitle;
    return fallback;

  case LISTITEM_COMMENT:
    if (item->HasPVRTimerInfoTag())
      return item->GetPVRTimerInfoTag()->GetStatus();
    if (item->HasMusicInfoTag())
      return item->GetMusicInfoTag()->GetComment();
    return fallback;

  case LISTITEM_ACTUAL_ICON:
    return item->GetIconImage();

  case LISTITEM_ICON:
    return GetIcon(item);

  case LISTITEM_OVERLAY:
    return item->GetOverlayImage();

  case LISTITEM_THUMB:
    return item->GetArt("thumb");

  case LISTITEM_FOLDERPATH:
    return CURL(item->GetPath()).GetWithoutUserDetails();

  case LISTITEM_FOLDERNAME:
  case LISTITEM_PATH:
    return GetFoldernameOrPath(item, info);

  case LISTITEM_FILENAME_AND_PATH:
    return GetFilenameAndPath(item);

  case LISTITEM_PICTURE_PATH:
    if (item->IsPicture() && (!item->IsZIP() || item->IsRAR() || item->IsCBZ() || item->IsCBR()))
      return item->GetPath();
    return fallback;

  case LISTITEM_STUDIO:
    if (item->HasVideoInfoTag())
      return StringUtils::Join(item->GetVideoInfoTag()->m_studio, g_advancedSettings.m_videoItemSeparator);
    return fallback;

  case LISTITEM_COUNTRY:
    if (item->HasVideoInfoTag())
      return StringUtils::Join(item->GetVideoInfoTag()->m_country, g_advancedSettings.m_videoItemSeparator);
    return fallback;

  case LISTITEM_MPAA:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_strMPAARating;
    return fallback;

  case LISTITEM_CAST:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->GetCast();
    if (item->HasEPGInfoTag())
      return item->GetEPGInfoTag()->Cast();
    return fallback;

  case LISTITEM_CAST_AND_ROLE:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->GetCast(true);
    return fallback;

  case LISTITEM_WRITER:
    if (item->HasVideoInfoTag())
      return StringUtils::Join(item->GetVideoInfoTag()->m_writingCredits, g_advancedSettings.m_videoItemSeparator);
    if (item->HasEPGInfoTag())
      return item->GetEPGInfoTag()->Writer();
    return fallback;

  case LISTITEM_TAGLINE:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_strTagLine;
    return fallback;

  case LISTITEM_TRAILER:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_strTrailer;
    return fallback;

  case LISTITEM_TOP250:
    return GetTop250(item);

  case LISTITEM_SORT_LETTER:
    return GetSortLetter(item);

  case LISTITEM_VIDEO_CODEC:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_streamDetails.GetVideoCodec();
    return fallback;

  case LISTITEM_VIDEO_RESOLUTION:
    if (item->HasVideoInfoTag())
      return CStreamDetails::VideoDimsToResolutionDescription(item->GetVideoInfoTag()->m_streamDetails.GetVideoWidth(), item->GetVideoInfoTag()->m_streamDetails.GetVideoHeight());
    return fallback;

  case LISTITEM_VIDEO_ASPECT:
    if (item->HasVideoInfoTag())
      return CStreamDetails::VideoAspectToAspectDescription(item->GetVideoInfoTag()->m_streamDetails.GetVideoAspect());
    return fallback;

  case LISTITEM_AUDIO_CODEC:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_streamDetails.GetAudioCodec();
    return fallback;

  case LISTITEM_AUDIO_CHANNELS:
    return GetAudioChannels(item);

  case LISTITEM_AUDIO_LANGUAGE:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_streamDetails.GetAudioLanguage();
    return fallback;

  case LISTITEM_SUBTITLE_LANGUAGE:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_streamDetails.GetSubtitleLanguage();
    return fallback;

  case LISTITEM_STARTTIME:
    return GetStartTime(item);

  case LISTITEM_ENDTIME:
    return GetEndTime(item);

  case LISTITEM_STARTDATE:
    return GetStartDate(item);

  case LISTITEM_ENDDATE:
    return GetEndDate(item);

  case LISTITEM_CHANNEL_NUMBER:
    return GetChannelNumber(item);

  case LISTITEM_SUB_CHANNEL_NUMBER:
    return GetSubChannelNumber(item);

  case LISTITEM_CHANNEL_NUMBER_LBL:
    return GetChannelNumberLabel(item);

  case LISTITEM_CHANNEL_NAME:
    return GetChannelName(item);

  case LISTITEM_NEXT_STARTTIME:
    return GetNextStartTime(item);

  case LISTITEM_NEXT_ENDTIME:
    return GetNextEndTime(item);

  case LISTITEM_NEXT_STARTDATE:
    return GetNextStartDate(item);

  case LISTITEM_NEXT_ENDDATE:
    return GetNextEndDate(item);

  case LISTITEM_NEXT_PLOT:
    return GetNextPlot(item);

  case LISTITEM_NEXT_PLOT_OUTLINE:
    return GetNextPlotOutline(item);

  case LISTITEM_NEXT_DURATION:
    return GetNextDuration(item);

  case LISTITEM_NEXT_GENRE:
    return GetNextGenre(item);

  case LISTITEM_NEXT_TITLE:
    return GetNextTitle(item);

  case LISTITEM_PARENTALRATING:
    return GetParentalRating(item);

  /*case LISTITEM_PERCENT_PLAYED:
  {
    int val;
    if (m_manager->GetItemInt(val, item, info))
    {
      return StringUtils::Format("%d", val);
    }
    break;
  }*/
  case LISTITEM_DATE_ADDED:
    if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_dateAdded.IsValid())
      return item->GetVideoInfoTag()->m_dateAdded.GetAsLocalizedDate();
    break;

  case LISTITEM_DBTYPE:
    if (item->HasVideoInfoTag())
      return item->GetVideoInfoTag()->m_type;
    break;

  case LISTITEM_DBID:
    return GetDBID(item);

  case LISTITEM_STEREOSCOPIC_MODE:
    return GetStereoscopicMode(item);

  case LISTITEM_IMDBNUMBER:
    return GetImdbNumber(item);

  case LISTITEM_EPISODENAME:
    return GetEpisodeName(item);

  case LISTITEM_TIMERTYPE:
    if (item->HasPVRTimerInfoTag())
      return item->GetPVRTimerInfoTag()->GetTypeAsString();
    break;
  }

  return fallback;
}

std::string CGUIListItemInfoHelper::GetFilenameOrExtentsion(const CFileItemPtr& item, int info)
{
  std::string strFile;
  if (item->IsMusicDb() && item->HasMusicInfoTag())
    strFile = URIUtils::GetFileName(item->GetMusicInfoTag()->GetURL());
  else if (item->IsVideoDb() && item->HasVideoInfoTag())
    strFile = URIUtils::GetFileName(item->GetVideoInfoTag()->m_strFileNameAndPath);
  else
    strFile = URIUtils::GetFileName(item->GetPath());

  if (info == LISTITEM_FILE_EXTENSION)
  {
    std::string strExtension = URIUtils::GetExtension(strFile);
    return StringUtils::TrimLeft(strExtension, ".");
  }
  return strFile;
}

std::string CGUIListItemInfoHelper::GetTitle(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr epgTag(item->GetPVRChannelInfoTag()->GetEPGNow());
    return epgTag ?
      epgTag->Title() :
      CSettings::GetInstance().GetBool(CSettings::SETTING_EPG_HIDENOINFOAVAILABLE) ?
      "" : g_localizeStrings.Get(19055);
    // no information available
  }
  if (item->HasPVRRecordingInfoTag())
    return item->GetPVRRecordingInfoTag()->m_strTitle;
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->Title();
  if (item->HasPVRTimerInfoTag())
    return item->GetPVRTimerInfoTag()->Title();
  if (item->HasVideoInfoTag())
    return item->GetVideoInfoTag()->m_strTitle;
  if (item->HasMusicInfoTag())
    return item->GetMusicInfoTag()->GetTitle();

  return std::string();
}

std::string CGUIListItemInfoHelper::GetEpgEventTitle(const CFileItemPtr& item)
{
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->Title();
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return item->GetPVRTimerInfoTag()->GetEpgInfoTag()->Title();
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr epgTag(item->GetPVRChannelInfoTag()->GetEPGNow());
    if (epgTag)
      return epgTag->Title();
  }

  return std::string();
}

std::string CGUIListItemInfoHelper::GetOriginalTitle(const CFileItemPtr& item)
{
  if (item->HasPVRChannelInfoTag())
  {
    EPG::CEpgInfoTagPtr tag(item->GetPVRChannelInfoTag()->GetEPGNow());
    if (tag)
      return tag->OriginalTitle();
  }
  if (item->HasEPGInfoTag())
    return item->GetEPGInfoTag()->OriginalTitle();
  if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    return item->GetPVRTimerInfoTag()->GetEpgInfoTag()->OriginalTitle();
  if (item->HasVideoInfoTag())
    return item->GetVideoInfoTag()->m_strOriginalTitle;

  return std::string();
}

std::string CGUIListItemInfoHelper::GetPlayCount(const CFileItemPtr& item)
{
  std::string strPlayCount;
  if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_playCount > 0)
    strPlayCount = StringUtils::Format("%i", item->GetVideoInfoTag()->m_playCount);
  if (item->HasMusicInfoTag() && item->GetMusicInfoTag()->GetPlayCount() > 0)
    strPlayCount = StringUtils::Format("%i", item->GetMusicInfoTag()->GetPlayCount());
  return strPlayCount;
}

std::string CGUIListItemInfoHelper::GetLastPlayed(const CFileItemPtr& item, const std::string& fallback)
{
  CDateTime dateTime;
  if (item->HasVideoInfoTag())
    dateTime = item->GetVideoInfoTag()->m_lastPlayed;
  else if (item->HasMusicInfoTag())
    dateTime = item->GetMusicInfoTag()->GetLastPlayed();

  if (dateTime.IsValid())
    return dateTime.GetAsLocalizedDate();

  return fallback;
}

std::string CGUIListItemInfoHelper::GetTrackNumber(const CFileItemPtr& item)
{
  std::string track;
  if (item->HasMusicInfoTag())
    track = StringUtils::Format("%i", item->GetMusicInfoTag()->GetTrackNumber());
  if (item->HasVideoInfoTag() && item->GetVideoInfoTag()->m_iTrack > -1)
    track = StringUtils::Format("%i", item->GetVideoInfoTag()->m_iTrack);
  return track;
}

std::string CGUIListItemInfoHelper::GetDiscNumber(const CFileItemPtr& item)
{
  std::string disc;
  if (item->HasMusicInfoTag() && item->GetMusicInfoTag()->GetDiscNumber() > 0)
    disc = StringUtils::Format("%i", item->GetMusicInfoTag()->GetDiscNumber());
  return disc;
}

std::string CGUIListItemInfoHelper::GetArtist(const CFileItemPtr& item)
{
  if (item->HasVideoInfoTag())
    return StringUtils::Join(item->GetVideoInfoTag()->m_artist, g_advancedSettings.m_videoItemSeparator);
  if (item->HasMusicInfoTag())
    return StringUtils::Join(item->GetMusicInfoTag()->GetArtist(), g_advancedSettings.m_musicItemSeparator);

  return std::string();
}
}