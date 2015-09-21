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

#include "Actor.h"

#include "dbwrappers/Database.h"
#include "dbwrappers/dataset.h"

namespace KODI
{
namespace VIDEO
{
namespace DB
{
CActor::CActor(std::shared_ptr<dbiplus::Database> db, int id)
  : m_db{db}
  , m_id{id}
  , m_isLoadedFromDB{false}
{

}


std::string CActor::GetName()
{
  if (!m_isLoadedFromDB)
    Load();

  return m_name;
}

std::string CActor::GetArt()
{
  if (!m_isLoadedFromDB)
    Load();

  return m_art;
}

void CActor::Load()
{
  std::unique_ptr<dbiplus::Dataset> ds(m_db->CreateDataset());
  if (!ds->query(m_db->prepare("SELECT * FROM actor WHERE actor_id=%d", m_id)))
    return;

  if (ds->num_rows() != 1)
    return;

  m_name = ds->fv(1).get_asString();
  m_art = ds->fv(2).get_asString();
  m_isLoadedFromDB = true;
}

std::vector<CActor> GetActors(std::shared_ptr<dbiplus::Database> db)
{
  std::vector<CActor> actors;
  std::unique_ptr<dbiplus::Dataset> ds(db->CreateDataset());
  if (!ds->query("SELECT * FROM actor"))
    return actors;

  actors.reserve(ds->num_rows());

  while (!ds->eof())
  {
    CActor actor(db, ds->fv(0).get_asInt());
    actor.m_name = ds->fv(1).get_asString();
    actor.m_art = ds->fv(2).get_asString();
    actor.m_isLoadedFromDB = true;
    actors.push_back(actor);
    ds->next();
  }
  return actors;
}

} //DB
} //VIDEO
} //KODI
