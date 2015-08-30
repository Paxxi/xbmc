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

#include "GUIPlaylistInfo.h"
#include "GUIInfoLabels.h"
#include "GUIInfoManager.h"
#include "Application.h"
#include <xbmc/guilib/LocalizeStrings.h>
#include <xbmc/PlayListPlayer.h>
#include <xbmc/utils/StringUtils.h>
#include <xbmc/playlists/PlayList.h>

using namespace PLAYLIST;

namespace GUIINFO
{
int CGUIPlaylistInfo::LabelMask()
{
  return PLAYLIST_MASK;
}

std::string CGUIPlaylistInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string* fallback)
{
  std::string strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  case PLAYLIST_LENGTH:
  case PLAYLIST_POSITION:
  case PLAYLIST_RANDOM:
  case PLAYLIST_REPEAT:
    strLabel = GetPlaylistLabel(info);
  default:
    break;
  }

  return strLabel;
}

bool CGUIPlaylistInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
{
  switch (info)
  {
  default:
    break;
  }

  return false;
}

bool CGUIPlaylistInfo::GetBool(int condition, int contextWindow, const CGUIListItem* item)
{
  switch (condition)
  {
  case PLAYLIST_ISRANDOM:
    return g_playlistPlayer.IsShuffled(g_playlistPlayer.GetCurrentPlaylist());
  case PLAYLIST_ISREPEAT:
    return g_playlistPlayer.GetRepeat(g_playlistPlayer.GetCurrentPlaylist()) ==
      PLAYLIST::REPEAT_ALL;
  case  PLAYLIST_ISREPEATONE:
    return g_playlistPlayer.GetRepeat(g_playlistPlayer.GetCurrentPlaylist()) ==
      PLAYLIST::REPEAT_ONE;
  }

  return false;
}

std::string CGUIPlaylistInfo::GetPlaylistLabel(int item, int playlistid /* = PLAYLIST_NONE */)
{
  if (playlistid <= PLAYLIST_NONE && !g_application.m_pPlayer->IsPlaying())
    return "";

  int iPlaylist = playlistid == PLAYLIST_NONE ? g_playlistPlayer.GetCurrentPlaylist() : playlistid;
  switch (item)
  {
  case PLAYLIST_LENGTH:
  {
    return StringUtils::Format("%i", g_playlistPlayer.GetPlaylist(iPlaylist).size());;
  }
  case PLAYLIST_POSITION:
  {
    return StringUtils::Format("%i", g_playlistPlayer.GetCurrentSong() + 1);
  }
  case PLAYLIST_RANDOM:
  {
    if (g_playlistPlayer.IsShuffled(iPlaylist))
      return g_localizeStrings.Get(590); // 590: Random
    else
      return g_localizeStrings.Get(591); // 591: Off
  }
  case PLAYLIST_REPEAT:
  {
    PLAYLIST::REPEAT_STATE state = g_playlistPlayer.GetRepeat(iPlaylist);
    if (state == PLAYLIST::REPEAT_ONE)
      return g_localizeStrings.Get(592); // 592: One
    else if (state == PLAYLIST::REPEAT_ALL)
      return g_localizeStrings.Get(593); // 593: All
    else
      return g_localizeStrings.Get(594); // 594: Off
  }
  }
  return "";
}
}