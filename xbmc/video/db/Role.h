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
class CActor;
class CRole;

//only int version needs to be a friend, it's the one doing the work
std::vector<CRole> GetRolesForActor(std::shared_ptr<dbiplus::Database> db, int actorId);
std::vector<CRole> GetRolesForActor(std::shared_ptr<dbiplus::Database> db, const CActor& actor);

class CRole
{
public:
  CRole(std::shared_ptr<dbiplus::Database> db, int actorId);

  int GetActorId() const
  {
    return m_actorId;
  }

  CActor GetActor() const;

  int GetMediaId();
  int GetCastOrder();
  std::string GetMediaType();
  std::string GetRole();

private:
  friend std::vector<CRole> GetRolesForActor(std::shared_ptr<dbiplus::Database> db, int actorId);
  void Load();
  std::shared_ptr<dbiplus::Database> m_db;

  int m_actorId;
  int m_mediaId;
  int m_castOrder;
  bool m_isLoadedFromDB;

  std::string m_mediaType;
  std::string m_role;
};
} //DB
} //VIDEO
} //KODI