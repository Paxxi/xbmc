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

#include "GUIPlayerInfo.h"
#include "GUIInfoLabels.h"
#include "Application.h"
#include "GUIInfoManager.h"
#include "Util.h"

#include "cores/AudioEngine/Utils/AEUtil.h"
#include "settings/Settings.h"
#include "settings/MediaSettings.h"
#include "utils/URIUtils.h"
#include "guilib/LocalizeStrings.h"
#include "epg/EpgInfoTag.h"
#include "music/tags/MusicInfoTag.h"
#include "utils/SeekHandler.h"
#include "guilib/GUIListItem.h"

using namespace MUSIC_INFO;


namespace GUIINFO
{

int CGUIPlayerInfo::LabelMask()
{
  return PLAYER_MASK;
}

std::string CGUIPlayerInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback)
{
  std::string strLabel = *fallback;

  switch (info)
  {
  case PLAYER_VOLUME:
    strLabel = StringUtils::Format("%2.1f dB", CAEUtil::PercentToGain(g_application.GetVolume(false)));
    break;
  case PLAYER_SUBTITLE_DELAY:
    strLabel = StringUtils::Format("%2.3f s", CMediaSettings::GetInstance().GetCurrentVideoSettings().m_SubtitleDelay);
    break;
  case PLAYER_AUDIO_DELAY:
    strLabel = StringUtils::Format("%2.3f s", CMediaSettings::GetInstance().GetCurrentVideoSettings().m_AudioDelay);
    break;
  case PLAYER_CHAPTER:
    if (g_application.m_pPlayer->IsPlaying())
      strLabel = StringUtils::Format("%02d", g_application.m_pPlayer->GetChapter());
    break;
  case PLAYER_CHAPTERCOUNT:
    if (g_application.m_pPlayer->IsPlaying())
      strLabel = StringUtils::Format("%02d", g_application.m_pPlayer->GetChapterCount());
    break;
  case PLAYER_CHAPTERNAME:
    if (g_application.m_pPlayer->IsPlaying())
      g_application.m_pPlayer->GetChapterName(strLabel);
    break;
  case PLAYER_CACHELEVEL:
  {
    int iLevel = 0;
    if (g_application.m_pPlayer->IsPlaying() && m_manager->GetInt(iLevel, PLAYER_CACHELEVEL) && iLevel >= 0)
      strLabel = StringUtils::Format("%i", iLevel);
  }
  break;
  case PLAYER_TIME:
    if (g_application.m_pPlayer->IsPlaying())
      strLabel = m_manager->GetCurrentPlayTime(TIME_FORMAT_HH_MM);
    break;
  case PLAYER_DURATION:
    if (g_application.m_pPlayer->IsPlaying())
      strLabel = m_manager->GetDuration(TIME_FORMAT_HH_MM);
    break;
  case PLAYER_PATH:
  case PLAYER_FILENAME:
  case PLAYER_FILEPATH:
    if (currentFile)
    {
      if (currentFile->HasMusicInfoTag())
        strLabel = currentFile->GetMusicInfoTag()->GetURL();
      else if (currentFile->HasVideoInfoTag())
        strLabel = currentFile->GetVideoInfoTag()->m_strFileNameAndPath;
      if (strLabel.empty())
        strLabel = currentFile->GetPath();
    }
    if (info == PLAYER_PATH)
    {
      // do this twice since we want the path outside the archive if this
      // is to be of use.
      if (URIUtils::IsInArchive(strLabel))
        strLabel = URIUtils::GetParentPath(strLabel);
      strLabel = URIUtils::GetParentPath(strLabel);
    }
    else if (info == PLAYER_FILENAME)
      strLabel = URIUtils::GetFileName(strLabel);
    break;
  case PLAYER_TITLE:
  {
    if (currentFile)
    {
      if (currentFile->HasPVRChannelInfoTag())
      {
        EPG::CEpgInfoTagPtr tag(currentFile->GetPVRChannelInfoTag()->GetEPGNow());
        return tag ?
          tag->Title() :
          CSettings::GetInstance().GetBool("epg.hidenoinfoavailable") ?
          "" : g_localizeStrings.Get(19055); // no information available
      }
      if (currentFile->HasPVRRecordingInfoTag() && !currentFile->GetPVRRecordingInfoTag()->m_strTitle.empty())
        return currentFile->GetPVRRecordingInfoTag()->m_strTitle;
      if (currentFile->HasVideoInfoTag() && !currentFile->GetVideoInfoTag()->m_strTitle.empty())
        return currentFile->GetVideoInfoTag()->m_strTitle;
      if (currentFile->HasMusicInfoTag() && !currentFile->GetMusicInfoTag()->GetTitle().empty())
        return currentFile->GetMusicInfoTag()->GetTitle();
      // don't have the title, so use dvdplayer, label, or drop down to title from path
      if (!g_application.m_pPlayer->GetPlayingTitle().empty())
        return g_application.m_pPlayer->GetPlayingTitle();
      if (!currentFile->GetLabel().empty())
        return currentFile->GetLabel();
      return CUtil::GetTitleFromPath(currentFile->GetPath());
    }
    else
    {
      if (!g_application.m_pPlayer->GetPlayingTitle().empty())
        return g_application.m_pPlayer->GetPlayingTitle();
    }
  }
  break;
  default:
    break;
  }

  return strLabel;
}

bool CGUIPlayerInfo::HandleIsPlayingLabels(int& value, int info)
{
  if (g_application.m_pPlayer->IsPlaying())
  {
    switch (info)
    {
    case PLAYER_PROGRESS:
      value = static_cast<int>(g_application.GetPercentage());
      return true;
    case PLAYER_PROGRESS_CACHE:
      value = static_cast<int>(g_application.GetCachePercentage());
      return true;
    case PLAYER_SEEKBAR:
      value = static_cast<int>(CSeekHandler::GetInstance().GetPercent());
      return true;
    case PLAYER_CACHELEVEL:
      value = static_cast<int>(g_application.m_pPlayer->GetCacheLevel());
      return true;
    case PLAYER_CHAPTER:
      value = g_application.m_pPlayer->GetChapter();
      return true;
    case PLAYER_CHAPTERCOUNT:
      value = g_application.m_pPlayer->GetChapterCount();
      return true;
    }
  }

  return false;
}

bool CGUIPlayerInfo::GetInt(int &value, int info, int contextWindow, const CGUIListItem *item /* = nullptr */)
{
  switch (info)
  {
  case PLAYER_VOLUME:
    value = static_cast<int>(g_application.GetVolume());
    return true;
  case PLAYER_SUBTITLE_DELAY:
    value = g_application.GetSubtitleDelay();
    return true;
  case PLAYER_AUDIO_DELAY:
    value = g_application.GetAudioDelay();
    return true;
  case PLAYER_PROGRESS:
  case PLAYER_PROGRESS_CACHE:
  case PLAYER_SEEKBAR:
  case PLAYER_CACHELEVEL:
  case PLAYER_CHAPTER:
  case PLAYER_CHAPTERCOUNT:
    return HandleIsPlayingLabels(value, info);
  default:
    break;
  }

  return false;
}

bool CGUIPlayerInfo::GetBool(int condition, int contextWindow, const CGUIListItem* item)
{
  switch (condition)
  {
    
  }

  return false;
}
} //end namespace GUIINFO