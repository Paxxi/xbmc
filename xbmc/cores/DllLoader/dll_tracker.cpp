/*
 *      Copyright (C) 2005-2013 Team XBMC
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

#include "dll_tracker.h"
#include "dll_tracker_library.h"
#include "dll_tracker_file.h"
#include "DllLoader.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#include <stdlib.h>

#ifdef _cplusplus
extern "C"
{
#endif

CCriticalSection g_trackerLock;
TrackedDllList g_trackedDlls;

void tracker_dll_add(DllLoader* pDll)
{
  DllTrackInfo* trackInfo = new DllTrackInfo;
  trackInfo->pDll = pDll;
  trackInfo->lMinAddr = 0;
  trackInfo->lMaxAddr = 0;
  CSingleLock locktd(g_trackerLock);
  g_trackedDlls.push_back(trackInfo);
}

void tracker_dll_free(DllLoader* pDll)
{
  CSingleLock locktd(g_trackerLock);
  for (TrackedDllsIter it = g_trackedDlls.begin(); it != g_trackedDlls.end();)
  {
    // NOTE: This code assumes that the same dll pointer can be in more than one
    //       slot of the vector g_trackedDlls.  If it's not, then it can be
    //       simplified by returning after we've found the one we want, saving
    //       the iterator shuffling, and reducing potential bugs.
    if ((*it)->pDll == pDll)
    {
      try
      {
        tracker_library_free_all(*it);
        tracker_file_free_all(*it);
      }
      catch (...)
      {
        CLog::Log(LOGFATAL, "Error freeing tracked dll resources");
      }
      // free all functions which where created at the time we loaded the dll
	    DummyListIter dit = (*it)->dummyList.begin();
	    while (dit != (*it)->dummyList.end()) { free((void*)*dit); ++dit;	}
	    (*it)->dummyList.clear();
	
      delete (*it);
      it = g_trackedDlls.erase(it);
    }
    else
      ++it;
  }
}

void tracker_dll_set_addr(DllLoader* pDll, uintptr_t min, uintptr_t max)
{
  CSingleLock locktd(g_trackerLock);
  for (auto & g_trackedDll : g_trackedDlls)
  {
    if (g_trackedDll->pDll == pDll)
    {
      g_trackedDll->lMinAddr = min;
      g_trackedDll->lMaxAddr = max;
      break;
    }
  }
}

const char* tracker_getdllname(uintptr_t caller)
{
  DllTrackInfo *track = tracker_get_dlltrackinfo(caller);
  if(track)
    return track->pDll->GetFileName();
  return "";
}

DllTrackInfo* tracker_get_dlltrackinfo(uintptr_t caller)
{
  CSingleLock locktd(g_trackerLock);
  for (auto & g_trackedDll : g_trackedDlls)
  {
    if (caller >= g_trackedDll->lMinAddr && caller <= g_trackedDll->lMaxAddr)
    {
      return g_trackedDll;
    }
  }

  // crap not in any base address, check if it may be in virtual spaces
  for (auto & g_trackedDll : g_trackedDlls)
  {
    for(VAllocListIter it2 = g_trackedDll->virtualList.begin(); it2 != g_trackedDll->virtualList.end(); ++it2)
    {
      if( it2->first <= caller && caller < it2->first + it2->second.size )
        return g_trackedDll;

    }
  }

  return nullptr;
}

DllTrackInfo* tracker_get_dlltrackinfo_byobject(DllLoader* pDll)
{
  CSingleLock locktd(g_trackerLock);
  for (auto & g_trackedDll : g_trackedDlls)
  {
    if (g_trackedDll->pDll == pDll)
    {
      return g_trackedDll;
    }
  }
  return nullptr;
}

void tracker_dll_data_track(DllLoader* pDll, uintptr_t addr)
{
  CSingleLock locktd(g_trackerLock);
  for (auto & g_trackedDll : g_trackedDlls)
  {
    if (pDll == g_trackedDll->pDll)
    {
      g_trackedDll->dummyList.push_back((uintptr_t)addr);
      break;
    }
  }
}

#ifdef _cplusplus
}
#endif
