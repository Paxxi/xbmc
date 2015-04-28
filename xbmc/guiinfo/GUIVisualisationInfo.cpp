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

#include "GUIVisualisationInfo.h"
#include "GUIInfoLabels.h"
#include "GUIInfoManager.h"

namespace GUIINFO
{
int CGUIVisualisationInfo::LabelMask()
{
  return VISUALISATION_MASK;
}

std::string CGUIVisualisationInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string* fallback)
{
  std::string strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  default:
    break;
  }

  return strLabel;
}

bool CGUIVisualisationInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
{
  switch (info)
  {
  default:
    break;
  }

  return false;
}

}