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
#include "utils/Temperature.h"

class CGUIInfoManager;

namespace GUIINFO
{
class CGUISystemInfo : public IGUIInfo
{
public:
  CGUISystemInfo(CGUIInfoManager* manager) : IGUIInfo(manager) { }
  virtual ~CGUISystemInfo() { }

  virtual std::string GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback) override;
  virtual bool GetInt(int &value, int info, int contextWindow, const CGUIListItem *item = nullptr) override;
  virtual bool GetBool(int condition, int contextWindow = 0, const CGUIListItem *item = nullptr) override;
  static int LabelMask();

private:
  std::string GetScreenResolution() const;
  std::string GetMemory(int info) const;
  std::string GetSystemHeatInfo(int info);
  CTemperature GetGPUTemperature() const;

  // fan stuff
  unsigned int m_lastSysHeatInfoTime{0};
  int m_fanSpeed{0};
  CTemperature m_gpuTemp;
  CTemperature m_cpuTemp;
  int m_system{0};
};
}