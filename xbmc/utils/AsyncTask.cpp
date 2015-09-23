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

#include "AsyncTask.h"

#include "utils/Job.h"
#include "utils/JobManager.h"

namespace KODI
{
namespace UTILS
{
class AsyncTask : public CJob
{
public:
  AsyncTask() = delete;
  AsyncTask(std::function<void()> task)
    : m_task{task}
  { }
  virtual ~AsyncTask()
  { }
  virtual bool DoWork() override
  {
    m_task();
    return true;
  }
private:
  std::function<void()> m_task;
};

bool RunAsync(std::function<void()> task)
{
  return CJobManager::GetInstance().AddJob(new AsyncTask(task), nullptr, CJob::PRIORITY_NORMAL);
}
}
}
