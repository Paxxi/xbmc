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

class CGUIInfoManager;
class CFileItem;
class CGUIListItem;

namespace GUIINFO
{
class IGUIInfo
{
protected:
  CGUIInfoManager* m_manager;

public:
  IGUIInfo() = delete;
  IGUIInfo(CGUIInfoManager * manager) : m_manager{manager} { }
  virtual ~IGUIInfo() { }

  virtual std::string GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback) = 0;
  virtual bool GetInt(int &value, int info, int contextWindow, const CGUIListItem *item = nullptr) = 0;
  virtual bool GetBool(int condition, int contextWindow = 0, const CGUIListItem *item = nullptr) = 0;

  static int LabelMask() { return 0; };
};
}