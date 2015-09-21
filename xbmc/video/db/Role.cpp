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

#include "Role.h"

#include "dbwrappers/Database.h"
#include "dbwrappers/dataset.h"
#include "video/db/Actor.h"

namespace KODI
{
namespace VIDEO
{
namespace DB
{
CRole::CRole(std::shared_ptr<dbiplus::Database> db, int actorId)
  : m_db{db}
  , m_actorId{actorId}
  , m_mediaId{0}
  , m_castOrder{0}
  , m_isLoadedFromDB{false}
{
}

CActor CRole::GetActor() const
{
  return CActor(m_db, m_actorId);
}

int CRole::GetMediaId()
{
  if (!m_isLoadedFromDB)
    Load();
  return m_mediaId;
}

int CRole::GetCastOrder()
{
  if (!m_isLoadedFromDB)
    Load();
  return m_castOrder;
}

std::string CRole::GetMediaType()
{
  if (!m_isLoadedFromDB)
    Load();
  return m_mediaType;
}

std::string CRole::GetRole()
{
  if (!m_isLoadedFromDB)
    Load();
  return m_mediaType;
}

void CRole::Load()
{
  //We can't grab a specific role with just one of these
  if (m_actorId == 0 || m_mediaId == 0)
    return;

  std::unique_ptr<dbiplus::Dataset> ds(m_db->CreateDataset());
  if (!ds->query(m_db->prepare("SELECT * FROM actor_link WHERE actor_id=%d AND media_id=%d", m_actorId, m_mediaId)))
    return;

  if (ds->num_rows() != 1)
    return;

  m_mediaType = ds->fv(2).get_asString();
  m_role = ds->fv(3).get_asString();
  m_castOrder = ds->fv(4).get_asInt();
  m_isLoadedFromDB = true;
}

std::vector<CRole> GetRolesForActor(std::shared_ptr<dbiplus::Database> db, int actorId)
{
  std::vector<CRole> roles;
  std::unique_ptr<dbiplus::Dataset> ds(db->CreateDataset());
  if (!ds->query(db->prepare("SELECT * FROM actor_link WHERE actor_id=%d", actorId)))
    return roles;

  roles.reserve(ds->num_rows());
  while (!ds->eof())
  {
    CRole role(db, ds->fv(0).get_asInt());
    role.m_mediaId = ds->fv(1).get_asInt();
    role.m_mediaType = ds->fv(2).get_asString();
    role.m_role = ds->fv(3).get_asString();
    role.m_castOrder = ds->fv(4).get_asInt();
    role.m_isLoadedFromDB = true;
    roles.push_back(role);
    ds->next();
  }
  return roles;
}

std::vector<CRole> GetRolesForActor(std::shared_ptr<dbiplus::Database> db, const CActor & actor)
{
  return GetRolesForActor(db, actor.GetId());
}

} //DB
} //VIDEO
} //KODI
