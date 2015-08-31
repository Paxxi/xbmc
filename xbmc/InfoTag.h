#pragma once
/*
*      Copyright (C) 2005-2015 Team Kodi
*      http://kodi.tv
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

#include <map>
#include <memory>
#include <string>

class CDateTime;
class CVariant;

namespace KODI
{
enum class InfoTagType
{
  MUSIC,
  VIDEO,
  EPG,
  PVRCHANNEL
};

class IInfoTag
{
protected:
  IInfoTag(){};

public:
  virtual ~IInfoTag() {};


  virtual std::string GetLabel() const = 0;
  virtual std::string GetLabel2() const = 0;
  virtual std::string GetPath() const = 0;
  virtual std::string GetIcon() const = 0;
  
  virtual bool IsFolder() const = 0;

  virtual CDateTime GetDateTime() const = 0;

  virtual InfoTagType GetTagType() const = 0;
  virtual std::map<std::string, CVariant> GetProperties() const = 0;

  virtual std::shared_ptr<IInfoTag> GetSubTag(InfoTagType type) const
  {
    return nullptr;
  }
  virtual bool AddSubTag(std::shared_ptr<IInfoTag> tag)
  {
    return false;
  }

};
}