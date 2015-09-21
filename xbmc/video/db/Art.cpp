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

#include "Art.h"

#include "dbwrappers/Database.h"
#include "dbwrappers/dataset.h"

namespace KODI
{
namespace VIDEO
{
namespace DB
{
CArt::CArt(std::shared_ptr<dbiplus::Database> db, int artId)
  : m_db{db}
  , m_artId{artId}
  , m_mediaId{0}
  , m_isLoadedFromDB{false}
{
}

int CArt::GetMediaId()
{
  if (!m_isLoadedFromDB)
    Load();

  return m_mediaId;
}

std::string CArt::GetMediaType()
{
  if (!m_isLoadedFromDB)
    Load();

  return m_mediaType;
}

std::string CArt::GetType()
{
  if (!m_isLoadedFromDB)
    Load();

  return m_type;
}

std::string CArt::GetUrl()
{
  if (!m_isLoadedFromDB)
    Load();

  return m_url;
}

void CArt::Load()
{
  std::unique_ptr<dbiplus::Dataset> ds(m_db->CreateDataset());
  if (!ds->query(m_db->prepare("SELECT * FROM art WHERE art_id=%d", m_artId)))
    return;

  if (ds->num_rows() != 1)
    return;

  m_mediaId = ds->fv(1).get_asInt();
  m_mediaType = ds->fv(2).get_asString();
  m_type = ds->fv(3).get_asString();
  m_url = ds->fv(4).get_asString();
  m_isLoadedFromDB = true;
}

} //DB
} //VIDEO
} //KODI
