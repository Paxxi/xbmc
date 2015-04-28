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

#include "GUIVideoPlayerInfo.h"
#include "GUIInfoManager.h"
#include "GUIInfoLabels.h"
#include "Application.h"
#include <xbmc/utils/StringUtils.h>

namespace GUIINFO
{
  int CGUIVideoPlayerInfo::LabelMask()
  {
    return VIDEOPLAYER_MASK;
  }

  std::string CGUIVideoPlayerInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback)
  {
    std::string strLabel = fallback == nullptr ? "" : *fallback;

    switch (info)
    {
    case VIDEOPLAYER_TITLE:
    case VIDEOPLAYER_ORIGINALTITLE:
    case VIDEOPLAYER_GENRE:
    case VIDEOPLAYER_DIRECTOR:
    case VIDEOPLAYER_YEAR:
    case VIDEOPLAYER_PLAYLISTLEN:
    case VIDEOPLAYER_PLAYLISTPOS:
    case VIDEOPLAYER_PLOT:
    case VIDEOPLAYER_PLOT_OUTLINE:
    case VIDEOPLAYER_EPISODE:
    case VIDEOPLAYER_SEASON:
    case VIDEOPLAYER_RATING:
    case VIDEOPLAYER_RATING_AND_VOTES:
    case VIDEOPLAYER_TVSHOW:
    case VIDEOPLAYER_PREMIERED:
    case VIDEOPLAYER_STUDIO:
    case VIDEOPLAYER_COUNTRY:
    case VIDEOPLAYER_MPAA:
    case VIDEOPLAYER_TOP250:
    case VIDEOPLAYER_CAST:
    case VIDEOPLAYER_CAST_AND_ROLE:
    case VIDEOPLAYER_ARTIST:
    case VIDEOPLAYER_ALBUM:
    case VIDEOPLAYER_WRITER:
    case VIDEOPLAYER_TAGLINE:
    case VIDEOPLAYER_TRAILER:
    case VIDEOPLAYER_STARTTIME:
    case VIDEOPLAYER_ENDTIME:
    case VIDEOPLAYER_NEXT_TITLE:
    case VIDEOPLAYER_NEXT_GENRE:
    case VIDEOPLAYER_NEXT_PLOT:
    case VIDEOPLAYER_NEXT_PLOT_OUTLINE:
    case VIDEOPLAYER_NEXT_STARTTIME:
    case VIDEOPLAYER_NEXT_ENDTIME:
    case VIDEOPLAYER_NEXT_DURATION:
    case VIDEOPLAYER_CHANNEL_NAME:
    case VIDEOPLAYER_CHANNEL_NUMBER:
    case VIDEOPLAYER_SUB_CHANNEL_NUMBER:
    case VIDEOPLAYER_CHANNEL_NUMBER_LBL:
    case VIDEOPLAYER_CHANNEL_GROUP:
    case VIDEOPLAYER_PARENTAL_RATING:
    case VIDEOPLAYER_PLAYCOUNT:
    case VIDEOPLAYER_LASTPLAYED:
    case VIDEOPLAYER_IMDBNUMBER:
    case VIDEOPLAYER_EPISODENAME:
      strLabel = m_manager->GetVideoLabel(info);
      break;
    case VIDEOPLAYER_VIDEO_CODEC:
      if (g_application.m_pPlayer->IsPlaying())
      {
        strLabel = m_videoInfo.videoCodecName;
      }
      break;
    case VIDEOPLAYER_VIDEO_RESOLUTION:
      if (g_application.m_pPlayer->IsPlaying())
      {
        return CStreamDetails::VideoDimsToResolutionDescription(m_videoInfo.width, m_videoInfo.height);
      }
      break;
    case VIDEOPLAYER_AUDIO_CODEC:
      if (g_application.m_pPlayer->IsPlaying())
      {
        strLabel = m_audioInfo.audioCodecName;
      }
      break;
    case VIDEOPLAYER_VIDEO_ASPECT:
      if (g_application.m_pPlayer->IsPlaying())
      {
        strLabel = CStreamDetails::VideoAspectToAspectDescription(m_videoInfo.videoAspectRatio);
      }
      break;
    case VIDEOPLAYER_AUDIO_CHANNELS:
      if (g_application.m_pPlayer->IsPlaying())
      {
        strLabel = StringUtils::Format("%i", m_audioInfo.channels);
      }
      break;
    case VIDEOPLAYER_AUDIO_LANG:
      if (g_application.m_pPlayer->IsPlaying())
      {
        SPlayerAudioStreamInfo info;
        g_application.m_pPlayer->GetAudioStreamInfo(CURRENT_STREAM, info);
        strLabel = info.language;
      }
      break;
    case VIDEOPLAYER_STEREOSCOPIC_MODE:
      if (g_application.m_pPlayer->IsPlaying())
      {
        strLabel = m_videoInfo.stereoMode;
      }
      break;
    case VIDEOPLAYER_SUBTITLES_LANG:
      if (g_application.m_pPlayer && g_application.m_pPlayer->IsPlaying() && g_application.m_pPlayer->GetSubtitleVisible())
      {
        SPlayerSubtitleStreamInfo ssi;
        g_application.m_pPlayer->GetSubtitleStreamInfo(g_application.m_pPlayer->GetSubtitle(), ssi);
        strLabel = ssi.language;
      }
      break;
    default:
      break;
    }

    return strLabel;
  }

  bool CGUIVideoPlayerInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
  {
    switch (info)
    {
    default:
      break;
    }

    return false;
  }

}