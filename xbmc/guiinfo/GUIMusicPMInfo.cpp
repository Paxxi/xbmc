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

#include "GUIMusicPMInfo.h"
#include "GUIInfoLabels.h"
#include "GUIInfoManager.h"
#include <xbmc/utils/StringUtils.h>
#include <xbmc/PartyModeManager.h>

namespace GUIINFO
{
int CGUIMusicPMInfo::LabelMask()
{
  return MUSICPM_MASK;
}

std::string CGUIMusicPMInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string* fallback)
{
  std::string strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  case MUSICPM_SONGSPLAYED:
  case MUSICPM_MATCHINGSONGS:
  case MUSICPM_MATCHINGSONGSPICKED:
  case MUSICPM_MATCHINGSONGSLEFT:
  case MUSICPM_RELAXEDSONGSPICKED:
  case MUSICPM_RANDOMSONGSPICKED:
    strLabel = GetMusicPartyModeLabel(info);
    break;
  default:
    break;
  }

  return strLabel;
}

bool CGUIMusicPMInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
{
  switch (info)
  {
  default:
    break;
  }

  return false;
}

std::string CGUIMusicPMInfo::GetMusicPartyModeLabel(int item)
{
  // get song counts
  if (item >= MUSICPM_SONGSPLAYED && item <= MUSICPM_RANDOMSONGSPICKED)
  {
    int iSongs = -1;
    switch (item)
    {
    case MUSICPM_SONGSPLAYED:
    {
      iSongs = g_partyModeManager.GetSongsPlayed();
      break;
    }
    case MUSICPM_MATCHINGSONGS:
    {
      iSongs = g_partyModeManager.GetMatchingSongs();
      break;
    }
    case MUSICPM_MATCHINGSONGSPICKED:
    {
      iSongs = g_partyModeManager.GetMatchingSongsPicked();
      break;
    }
    case MUSICPM_MATCHINGSONGSLEFT:
    {
      iSongs = g_partyModeManager.GetMatchingSongsLeft();
      break;
    }
    case MUSICPM_RELAXEDSONGSPICKED:
    {
      iSongs = g_partyModeManager.GetRelaxedSongs();
      break;
    }
    case MUSICPM_RANDOMSONGSPICKED:
    {
      iSongs = g_partyModeManager.GetRandomSongs();
      break;
    }
    }
    if (iSongs < 0)
      return "";
    std::string strLabel = StringUtils::Format("%i", iSongs);
    return strLabel;
  }
  return "";
}
}