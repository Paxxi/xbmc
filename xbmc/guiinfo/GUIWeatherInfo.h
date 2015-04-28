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

#include <string>

#include "GUIInfo.h"

namespace GUIINFO
{
class CGUIWeatherInfo : public IGUIInfo
{
public:
  CGUIWeatherInfo(CGUIInfoManager* manager) : IGUIInfo(manager) { }
  virtual ~CGUIWeatherInfo() { }

  virtual std::string GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback) override;
  virtual bool GetInt(int &value, int info, int contextWindow, const CGUIListItem *item = nullptr) override;

  static int LabelMask();
};
}