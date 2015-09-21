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

#include <memory>
#include <string>
#include <vector>

namespace dbiplus
{
class Database;
}

namespace KODI
{
namespace VIDEO
{
namespace DB
{
class CArt
{
public:
  CArt(std::shared_ptr<dbiplus::Database> db, int artId);

  int GetArtId() const
  {
    return m_artId;
  }

  int GetMediaId();
  std::string GetMediaType();
  std::string GetType();
  std::string GetUrl();
private:
  void Load();
  std::shared_ptr<dbiplus::Database> m_db;

  int m_artId;
  int m_mediaId;
  std::string m_mediaType;
  std::string m_type;
  std::string m_url;
  bool m_isLoadedFromDB;
};
} //DB
} //VIDEO
} //KODI