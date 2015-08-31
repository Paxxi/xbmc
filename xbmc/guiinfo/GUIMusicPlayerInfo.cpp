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

#include "GUIMusicPlayerInfo.h"
#include "GUIInfoManager.h"
#include "GUIInfoLabels.h"
#include <xbmc/utils/StringUtils.h>
#include <xbmc/settings/AdvancedSettings.h>
#include <xbmc/FileItem.h>
#include <xbmc/music/tags/MusicInfoTag.h>
#include <xbmc/Application.h>
#include <xbmc/utils/MathUtils.h>
#include <xbmc/playlists/PlayList.h>

namespace GUIINFO
{
int CGUIMusicPlayerInfo::LabelMask()
{
  return MUSICPLAYER_MASK;
}

void CGUIMusicPlayerInfo::UpdateAVInfo()
{
  SPlayerAudioStreamInfo audio;
  g_application.m_pPlayer->GetAudioStreamInfo(CURRENT_STREAM, audio);
  m_audioInfo = audio;
}

std::string CGUIMusicPlayerInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback)
{
  auto strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  case MUSICPLAYER_TITLE:
  case MUSICPLAYER_ALBUM:
  case MUSICPLAYER_ARTIST:
  case MUSICPLAYER_ALBUM_ARTIST:
  case MUSICPLAYER_GENRE:
  case MUSICPLAYER_YEAR:
  case MUSICPLAYER_TRACK_NUMBER:
  case MUSICPLAYER_BITRATE:
  case MUSICPLAYER_PLAYLISTLEN:
  case MUSICPLAYER_PLAYLISTPOS:
  case MUSICPLAYER_CHANNELS:
  case MUSICPLAYER_BITSPERSAMPLE:
  case MUSICPLAYER_SAMPLERATE:
  case MUSICPLAYER_CODEC:
  case MUSICPLAYER_DISC_NUMBER:
  case MUSICPLAYER_RATING:
  case MUSICPLAYER_COMMENT:
  case MUSICPLAYER_LYRICS:
  case MUSICPLAYER_CHANNEL_NAME:
  case MUSICPLAYER_CHANNEL_NUMBER:
  case MUSICPLAYER_SUB_CHANNEL_NUMBER:
  case MUSICPLAYER_CHANNEL_NUMBER_LBL:
  case MUSICPLAYER_CHANNEL_GROUP:
  case MUSICPLAYER_PLAYCOUNT:
  case MUSICPLAYER_LASTPLAYED:
    strLabel = m_manager->GetMusicLabel(info);
    break;
  default:
    break;
  }

  return strLabel;
}

bool CGUIMusicPlayerInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
{
  switch (info)
  {
  default:
    break;
  }

  return false;
}

bool CGUIMusicPlayerInfo::GetBool(int condition, int contextWindow, const CGUIListItem* item)
{
  switch (condition)
  {
  case MUSICPLAYER_HASPREVIOUS:
    // requires current playlist be PLAYLIST_MUSIC
    if (g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC)
      return (g_playlistPlayer.GetCurrentSong() > 0); // not first song
    break;

  case MUSICPLAYER_HASNEXT:
    // requires current playlist be PLAYLIST_MUSIC
    if (g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC)
      return (g_playlistPlayer.GetCurrentSong() < (g_playlistPlayer.GetPlaylist(PLAYLIST_MUSIC).size() - 1)); // not last song
    break;

  case MUSICPLAYER_PLAYLISTPLAYING:
    if (g_application.m_pPlayer->IsPlayingAudio() && g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC)
      return true;
    break;
  }

  return false;
}

std::string CGUIMusicPlayerInfo::GetMusicLabel(int item)
{
  if (!g_application.m_pPlayer->IsPlaying() || !m_currentFile->HasMusicInfoTag()) return "";

  switch (item)
  {
  case MUSICPLAYER_PLAYLISTLEN:
  {
    if (g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC)
      return GetPlaylistLabel(PLAYLIST_LENGTH);
  }
  break;
  case MUSICPLAYER_PLAYLISTPOS:
  {
    if (g_playlistPlayer.GetCurrentPlaylist() == PLAYLIST_MUSIC)
      return GetPlaylistLabel(PLAYLIST_POSITION);
  }
  break;
  case MUSICPLAYER_BITRATE:
  {
    std::string strBitrate = "";
    if (m_audioInfo.bitrate > 0)
      strBitrate = StringUtils::Format("%i", MathUtils::round_int((double)m_audioInfo.bitrate / 1000.0));
    return strBitrate;
  }
  break;
  case MUSICPLAYER_CHANNELS:
  {
    std::string strChannels = "";
    if (m_audioInfo.channels > 0)
    {
      strChannels = StringUtils::Format("%i", m_audioInfo.channels);
    }
    return strChannels;
  }
  break;
  case MUSICPLAYER_BITSPERSAMPLE:
  {
    std::string strBitsPerSample = "";
    if (m_audioInfo.bitspersample > 0)
      strBitsPerSample = StringUtils::Format("%i", m_audioInfo.bitspersample);
    return strBitsPerSample;
  }
  break;
  case MUSICPLAYER_SAMPLERATE:
  {
    std::string strSampleRate = "";
    if (m_audioInfo.samplerate > 0)
      strSampleRate = StringUtils::Format("%.5g", ((double)m_audioInfo.samplerate / 1000.0));
    return strSampleRate;
  }
  break;
  case MUSICPLAYER_CODEC:
  {
    return StringUtils::Format("%s", m_audioInfo.audioCodecName.c_str());
  }
  break;
  }
  return GetMusicTagLabel(item, m_currentFile);
}

std::string CGUIMusicPlayerInfo::GetMusicTagLabel(int info, const CFileItem *item)
{
  if (!item->HasMusicInfoTag()) return "";
  const MUSIC_INFO::CMusicInfoTag &tag = *item->GetMusicInfoTag();
  switch (info)
  {
  case MUSICPLAYER_TITLE:
    if (tag.GetTitle().size()) { return tag.GetTitle(); }
    break;
  case MUSICPLAYER_ALBUM:
    if (tag.GetAlbum().size()) { return tag.GetAlbum(); }
    break;
  case MUSICPLAYER_ARTIST:
    if (tag.GetArtist().size()) { return StringUtils::Join(tag.GetArtist(), g_advancedSettings.m_musicItemSeparator); }
    break;
  case MUSICPLAYER_ALBUM_ARTIST:
    if (tag.GetAlbumArtist().size()) { return StringUtils::Join(tag.GetAlbumArtist(), g_advancedSettings.m_musicItemSeparator); }
    break;
  case MUSICPLAYER_YEAR:
    if (tag.GetYear()) { return tag.GetYearString(); }
    break;
  case MUSICPLAYER_GENRE:
    if (tag.GetGenre().size()) { return StringUtils::Join(tag.GetGenre(), g_advancedSettings.m_musicItemSeparator); }
    break;
  case MUSICPLAYER_LYRICS:
    if (tag.GetLyrics().size()) { return tag.GetLyrics(); }
    break;
  case MUSICPLAYER_TRACK_NUMBER:
  {
    std::string strTrack;
    if (tag.Loaded() && tag.GetTrackNumber() > 0)
    {
      return StringUtils::Format("%02i", tag.GetTrackNumber());
    }
  }
  break;
  case MUSICPLAYER_DISC_NUMBER:
    return GetItemLabel(item, LISTITEM_DISC_NUMBER);
  case MUSICPLAYER_RATING:
    return GetItemLabel(item, LISTITEM_RATING);
  case MUSICPLAYER_COMMENT:
    return GetItemLabel(item, LISTITEM_COMMENT);
  case MUSICPLAYER_DURATION:
    return GetItemLabel(item, LISTITEM_DURATION);
  case MUSICPLAYER_CHANNEL_NAME:
  {
    if (m_currentFile->HasPVRChannelInfoTag())
      return m_currentFile->GetPVRChannelInfoTag()->ChannelName();
  }
  break;
  case MUSICPLAYER_CHANNEL_NUMBER:
  {
    if (m_currentFile->HasPVRChannelInfoTag())
      return StringUtils::Format("%i", m_currentFile->GetPVRChannelInfoTag()->ChannelNumber());
  }
  break;
  case MUSICPLAYER_SUB_CHANNEL_NUMBER:
  {
    if (m_currentFile->HasPVRChannelInfoTag())
      return StringUtils::Format("%i", m_currentFile->GetPVRChannelInfoTag()->SubChannelNumber());
  }
  break;
  case MUSICPLAYER_CHANNEL_NUMBER_LBL:
  {
    if (m_currentFile->HasPVRChannelInfoTag())
      return m_currentFile->GetPVRChannelInfoTag()->FormattedChannelNumber();
  }
  break;
  case MUSICPLAYER_CHANNEL_GROUP:
  {
    if (m_currentFile->HasPVRChannelInfoTag() && m_currentFile->GetPVRChannelInfoTag()->IsRadio())
      return g_PVRManager.GetPlayingGroup(true)->GroupName();
  }
  break;
  case MUSICPLAYER_PLAYCOUNT:
    return GetItemLabel(item, LISTITEM_PLAYCOUNT);
  case MUSICPLAYER_LASTPLAYED:
    return GetItemLabel(item, LISTITEM_LASTPLAYED);
  }
  return "";
}
}