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

#include "GUIInfo.h"

class CFileItem;
typedef std::shared_ptr<CFileItem> CFileItemPtr;

class CGUIListItem;
typedef std::shared_ptr<CGUIListItem> CGUIListItemPtr;

namespace GUIINFO
{
class CGUIListItemInfo : public IGUIInfo
{
public:
  CGUIListItemInfo(CGUIInfoManager* manager) : IGUIInfo(manager) { }
  virtual ~CGUIListItemInfo() { }

  virtual std::string GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback) override;
  virtual bool GetInt(int &value, int info, int contextWindow, const CGUIListItem *item = nullptr) override;
  virtual bool GetBool(int condition, int contextWindow = 0, const CGUIListItem *item = nullptr) override;
  static int LabelMask();

  bool GetItemBool(const CGUIListItemPtr& item, int condition) const;
  std::string GetItemLabel(const CFileItemPtr& item, int info, std::string *fallback) const;
private:
  
  bool GetFileItemBool(const CFileItemPtr& item, int condition) const;
};
}