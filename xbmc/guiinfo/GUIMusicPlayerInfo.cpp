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

namespace GUIINFO
{
int CGUIMusicPlayerInfo::LabelMask()
{
  return MUSICPLAYER_MASK;
} 

std::string CGUIMusicPlayerInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback)
{
  std::string strLabel = fallback == nullptr ? "" : *fallback;

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

}