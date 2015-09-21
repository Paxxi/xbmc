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

namespace dbiplus
{
class Database;
class Dataset;
}

#include <memory>
#include <string>
#include <vector>

namespace KODI
{
namespace VIDEO
{
namespace DB
{
class CRole;
class CActor;

std::vector<CActor> GetActors(std::shared_ptr<dbiplus::Database> db);

class CActor
{
public:
  CActor(std::shared_ptr<dbiplus::Database> db, int id);

  int GetId() const
  {
    return m_id;
  }

  std::string GetName();
  std::string GetArt();

private:
  friend std::vector<CActor> GetActors(std::shared_ptr<dbiplus::Database> db);
  void Load();
  std::shared_ptr<dbiplus::Database> m_db;

  int m_id;
  bool m_isLoadedFromDB;
  std::string m_name;
  std::string m_art;
  //std::vector<CRole> m_roles;
};

}
}
}