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

#include "GUIListItemInfo.h"
#include "guiinfo/GUIInfoLabels.h"
#include <xbmc/music/CueInfoLoader.h>
#include <xbmc/settings/Settings.h>
#include <xbmc/guilib/LocalizeStrings.h>
#include <xbmc/pvr/recordings/PVRRecording.h>
#include <xbmc/pvr/channels/PVRChannel.h>
#include <xbmc/epg/EpgInfoTag.h>
#include <xbmc/music/tags/MusicInfoTag.h>
#include <xbmc/utils/StringUtils.h>
#include <xbmc/settings/AdvancedSettings.h>
#include <xbmc/utils/URIUtils.h>
#include <xbmc/URL.h>
#include <xbmc/utils/CharsetConverter.h>
#include <xbmc/guilib/StereoscopicsManager.h>
#include <xbmc/GUIInfoManager.h>
#include <xbmc/PlayListPlayer.h>

namespace GUIINFO
{
std::string CGUIListItemInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string* fallback)
{

}

bool CGUIListItemInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
{
}

bool CGUIListItemInfo::GetBool(int condition, int contextWindow, const CGUIListItem* item)
{
}

bool CGUIListItemInfo::GetItemBool(const CGUIListItemPtr& item, int condition) const
{
  if (!item) 
    return false;

  switch (condition)
  {
  case LISTITEM_ISPLAYING:
    {
      if (item->HasProperty("playlistposition"))
        return (int)item->GetProperty("playlisttype").asInteger() == g_playlistPlayer.GetCurrentPlaylist() && (int)item->GetProperty("playlistposition").asInteger() == g_playlistPlayer.GetCurrentSong();
      else if (item->IsFileItem() && !m_currentFile->GetPath().empty())
      {
        if (!g_application.m_strPlayListFile.empty())
        {
          //playlist file that is currently playing or the playlistitem that is currently playing.
          return ((const CFileItem *)item)->IsPath(g_application.m_strPlayListFile) || m_currentFile->IsSamePath((const CFileItem *)item);
        }
        return m_currentFile->IsSamePath((const CFileItem *)item);
      }
    }
  case LISTITEM_ISSELECTED:
    return item->IsSelected();
  case LISTITEM_IS_FOLDER:
    return item->m_bIsFolder;
  case LISTITEM_IS_RESUMABLE:
    {
      if (item->IsFileItem())
      {
        if (((const CFileItem *)item)->HasVideoInfoTag())
          return ((const CFileItem *)item)->GetVideoInfoTag()->m_resumePoint.timeInSeconds > 0;
        else if (((const CFileItem *)item)->HasPVRRecordingInfoTag())
          return ((const CFileItem *)item)->GetPVRRecordingInfoTag()->m_resumePoint.timeInSeconds > 0;
      }
    }
  }

  return false;
}



bool CGUIListItemInfo::GetFileItemBool(const CFileItemPtr& item, int condition) const
{
  switch (condition)
  {
  case LISTITEM_ISRECORDING:
    {
      if (!g_PVRManager.IsStarted())
        return false;

      if (item->HasPVRChannelInfoTag())
      {
        return item->GetPVRChannelInfoTag()->IsRecording();
      }
      else if (item->HasPVRTimerInfoTag())
      {
        const CPVRTimerInfoTagPtr timer = item->GetPVRTimerInfoTag();
        if (timer)
          return timer->IsRecording();
      }
      else if (item->HasEPGInfoTag())
      {
        CEpgInfoTagPtr epgTag = item->GetEPGInfoTag();

        // Check if the tag has a currently active recording associated
        if (epgTag->HasRecording() && epgTag->IsActive())
          return true;

        // Search all timers for something that matches the tag
        CFileItemPtr timer = g_PVRTimers->GetTimerForEpgTag(pItem);
        if (timer && timer->HasPVRTimerInfoTag())
          return timer->GetPVRTimerInfoTag()->IsRecording();
      }
    }
  case LISTITEM_INPROGRESS:
  {
    if (!g_PVRManager.IsStarted())
      return false;

    if (item->HasEPGInfoTag())
      return item->GetEPGInfoTag()->IsActive();
  }
  case LISTITEM_HASTIMER:
  {
    if (item->HasEPGInfoTag())
    {
      CFileItemPtr timer = g_PVRTimers->GetTimerForEpgTag(pItem);
      if (timer && timer->HasPVRTimerInfoTag())
        return timer->GetPVRTimerInfoTag()->IsActive();
    }
  }
  case LISTITEM_HASTIMERSCHEDULE:
  {
    if (item->HasEPGInfoTag())
    {
      CFileItemPtr timer = g_PVRTimers->GetTimerForEpgTag(item);
      if (timer && timer->HasPVRTimerInfoTag())
        return timer->GetPVRTimerInfoTag()->GetTimerScheduleId() > 0;
    }
  }
  case LISTITEM_HASRECORDING:
  {
    return item->HasEPGInfoTag() && item->GetEPGInfoTag()->HasRecording();
  }
  case LISTITEM_HAS_EPG:
  {
    if (item->HasPVRChannelInfoTag())
    {
      return (item->GetPVRChannelInfoTag()->GetEPGNow().get() != NULL);
    }
    if (item->HasPVRTimerInfoTag() && item->GetPVRTimerInfoTag()->HasEpgInfoTag())
    {
      return true;
    }
    else
    {
      return item->HasEPGInfoTag();
    }
  }
  case LISTITEM_ISENCRYPTED:
  {
    if (item->HasPVRChannelInfoTag())
    {
      return item->GetPVRChannelInfoTag()->IsEncrypted();
    }
    else if (item->HasEPGInfoTag() && item->GetEPGInfoTag()->HasPVRChannel())
    {
      return item->GetEPGInfoTag()->ChannelTag()->IsEncrypted();
    }
  }
  case LISTITEM_IS_STEREOSCOPIC:
  {
    std::string stereoMode = item->GetProperty("stereomode").asString();
    if (stereoMode.empty() && item->HasVideoInfoTag())
      stereoMode = CStereoscopicsManager::GetInstance().NormalizeStereoMode(item->GetVideoInfoTag()->m_streamDetails.GetStereoMode());
    if (!stereoMode.empty() && stereoMode != "mono")
      return true;
  }
  case LISTITEM_IS_COLLECTION:
  {
    if (item->HasVideoInfoTag())
      return (item->GetVideoInfoTag()->m_type == MediaTypeVideoCollection);
  }
  }

  return false;
}

std::string CGUIListItemInfo::GetItemLabel(const CFileItemPtr& item, int info, std::string* fallback) const
{
  if (!item) return "";

  switch (info)
  {
  
  return "";
}

int CGUIListItemInfo::LabelMask()
{
  return LISTITEM_MASK;
}
}