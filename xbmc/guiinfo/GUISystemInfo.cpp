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

#include "GUISystemInfo.h"
#include "GUIInfoLabels.h"
#include "GUIInfoManager.h"
#include "LangInfo.h"
#include "guilib/GUIWindowManager.h"
#include "windowing/WindowingFactory.h"
#include <xbmc/utils/SystemInfo.h>
#include <xbmc/guilib/LocalizeStrings.h>
#include <xbmc/settings/DisplaySettings.h>
#include <xbmc/utils/StringUtils.h>
#include <xbmc/settings/Settings.h>
#include <xbmc/profiles/ProfilesManager.h>
#include <xbmc/utils/AlarmClock.h>
#include <xbmc/guilib/GraphicContext.h>
#include <xbmc/guilib/GUIControl.h>
#include <xbmc/storage/MediaManager.h>
#include <xbmc/powermanagement/PowerManager.h>
#include <xbmc/utils/CPUInfo.h>
#include <xbmc/dialogs/GUIDialogProgress.h>

namespace GUIINFO
{
int CGUISystemInfo::LabelMask()
{
  return SYSTEM_MASK;
}

std::string CGUISystemInfo::GetLabel(CFileItem* /*currentFile*/, int info, int /*contextWindow*/, std::string *fallback)
{
  std::string strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  case SYSTEM_FREE_SPACE:
  case SYSTEM_USED_SPACE:
  case SYSTEM_TOTAL_SPACE:
  case SYSTEM_FREE_SPACE_PERCENT:
  case SYSTEM_USED_SPACE_PERCENT:
    return g_sysinfo.GetHddSpaceInfo(info);
    break;

  case SYSTEM_CPU_TEMPERATURE:
  case SYSTEM_GPU_TEMPERATURE:
  case SYSTEM_FAN_SPEED:
  case SYSTEM_CPU_USAGE:
    return m_manager->GetSystemHeatInfo(info);
    break;

  case SYSTEM_VIDEO_ENCODER_INFO:
  case SYSTEM_OS_VERSION_INFO:
  case SYSTEM_CPUFREQUENCY:
  case SYSTEM_INTERNET_STATE:
  case SYSTEM_UPTIME:
  case SYSTEM_TOTALUPTIME:
  case SYSTEM_BATTERY_LEVEL:
    return g_sysinfo.GetInfo(info);
    break;

  case SYSTEM_SCREEN_RESOLUTION:
    if (g_Windowing.IsFullScreen())
      strLabel = StringUtils::Format("%ix%i@%.2fHz - %s (%02.2f fps)",
      CDisplaySettings::Get().GetCurrentResolutionInfo().iScreenWidth,
      CDisplaySettings::Get().GetCurrentResolutionInfo().iScreenHeight,
      CDisplaySettings::Get().GetCurrentResolutionInfo().fRefreshRate,
      g_localizeStrings.Get(244).c_str(),
      m_manager->GetFPS());
    else
      strLabel = StringUtils::Format("%ix%i - %s (%02.2f fps)",
      CDisplaySettings::Get().GetCurrentResolutionInfo().iScreenWidth,
      CDisplaySettings::Get().GetCurrentResolutionInfo().iScreenHeight,
      g_localizeStrings.Get(242).c_str(),
      m_manager->GetFPS());
    return strLabel;
    break;

  case SYSTEM_BUILD_VERSION_SHORT:
    strLabel = CSysInfo::GetVersionShort();
    break;
  case SYSTEM_BUILD_VERSION:
    strLabel = CSysInfo::GetVersion();
    break;
  case SYSTEM_BUILD_DATE:
    strLabel = CSysInfo::GetBuildDate();
    break;
  case SYSTEM_FREE_MEMORY:
  case SYSTEM_FREE_MEMORY_PERCENT:
  case SYSTEM_USED_MEMORY:
  case SYSTEM_USED_MEMORY_PERCENT:
  case SYSTEM_TOTAL_MEMORY:
  {
    MEMORYSTATUSEX stat;
    stat.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&stat);
    int iMemPercentFree = 100 - static_cast<int>(100.0f* (stat.ullTotalPhys - stat.ullAvailPhys) / stat.ullTotalPhys + 0.5f);
    int iMemPercentUsed = 100 - iMemPercentFree;

    if (info == SYSTEM_FREE_MEMORY)
      strLabel = StringUtils::Format("%luMB", static_cast<ULONG>((stat.ullAvailPhys / MB)));
    else if (info == SYSTEM_FREE_MEMORY_PERCENT)
      strLabel = StringUtils::Format("%i%%", iMemPercentFree);
    else if (info == SYSTEM_USED_MEMORY)
      strLabel = StringUtils::Format("%luMB", static_cast<ULONG>(((stat.ullTotalPhys - stat.ullAvailPhys) / MB)));
    else if (info == SYSTEM_USED_MEMORY_PERCENT)
      strLabel = StringUtils::Format("%i%%", iMemPercentUsed);
    else if (info == SYSTEM_TOTAL_MEMORY)
      strLabel = StringUtils::Format("%luMB", static_cast<ULONG>((stat.ullTotalPhys / MB)));
  }
  break;
  case SYSTEM_SCREEN_MODE:
    strLabel = g_graphicsContext.GetResInfo().strMode;
    break;
  case SYSTEM_SCREEN_WIDTH:
    strLabel = StringUtils::Format("%i", g_graphicsContext.GetResInfo().iScreenWidth);
    break;
  case SYSTEM_SCREEN_HEIGHT:
    strLabel = StringUtils::Format("%i", g_graphicsContext.GetResInfo().iScreenHeight);
    break;
  case SYSTEM_CURRENT_WINDOW:
    return g_localizeStrings.Get(g_windowManager.GetFocusedWindow());
    break;
  case SYSTEM_STARTUP_WINDOW:
    strLabel = StringUtils::Format("%i", CSettings::Get().GetInt("lookandfeel.startupwindow"));
    break;
  case SYSTEM_CURRENT_CONTROL:
  {
    CGUIWindow *window = g_windowManager.GetWindow(g_windowManager.GetFocusedWindow());
    if (window)
    {
      CGUIControl *control = window->GetFocusedControl();
      if (control)
        strLabel = control->GetDescription();
    }
  }
  break;
#ifdef HAS_DVD_DRIVE
  case SYSTEM_DVD_LABEL:
    strLabel = g_mediaManager.GetDiskLabel();
    break;
#endif
  case SYSTEM_ALARM_POS:
    if (g_alarmClock.GetRemaining("shutdowntimer") == 0.f)
      strLabel = "";
    else
    {
      double fTime = g_alarmClock.GetRemaining("shutdowntimer");
      if (fTime > 60.f)
        strLabel = StringUtils::Format(g_localizeStrings.Get(13213).c_str(), g_alarmClock.GetRemaining("shutdowntimer") / 60.f);
      else
        strLabel = StringUtils::Format(g_localizeStrings.Get(13214).c_str(), g_alarmClock.GetRemaining("shutdowntimer"));
    }
    break;
  case SYSTEM_PROFILENAME:
    strLabel = CProfilesManager::Get().GetCurrentProfile().getName();
    break;
  case SYSTEM_PROFILECOUNT:
    strLabel = StringUtils::Format("%" PRIuS, CProfilesManager::Get().GetNumberOfProfiles());
    break;
  case SYSTEM_PROFILEAUTOLOGIN:
  {
    int profileId = CProfilesManager::Get().GetAutoLoginProfileId();
    if ((profileId < 0) || (!CProfilesManager::Get().GetProfileName(profileId, strLabel)))
      strLabel = g_localizeStrings.Get(37014); // Last used profile
  }
  break;
  case SYSTEM_LANGUAGE:
    strLabel = g_langInfo.GetEnglishLanguageName();
    break;
  case SYSTEM_TEMPERATURE_UNITS:
    strLabel = g_langInfo.GetTemperatureUnitString();
    break;
  case SYSTEM_PROGRESS_BAR:
  {
    int percent;
    if (m_manager->GetInt(percent, SYSTEM_PROGRESS_BAR) && percent > 0)
      strLabel = StringUtils::Format("%i", percent);
  }
  break;
  case SYSTEM_FRIENDLY_NAME:
    strLabel = CSysInfo::GetDeviceName();
    break;
  case SYSTEM_STEREOSCOPIC_MODE:
  {
    int stereoMode = CSettings::Get().GetInt("videoscreen.stereoscopicmode");
    strLabel = StringUtils::Format("%i", stereoMode);
  }
  break;

  case SYSTEM_DATE:
    strLabel = m_manager->GetDate();
    break;
  case SYSTEM_FPS:
    strLabel = StringUtils::Format("%02.2f", m_manager->GetFPS());
    break;
  case SYSTEM_RENDER_VENDOR:
    strLabel = g_Windowing.GetRenderVendor();
    break;
  case SYSTEM_RENDER_RENDERER:
    strLabel = g_Windowing.GetRenderRenderer();
    break;
  case SYSTEM_RENDER_VERSION:
    strLabel = g_Windowing.GetRenderVersionString();
    break;
  default:
    break;
  }

  return strLabel;
}

bool CGUISystemInfo::GetInt(int &value, int info, int /*contextWindow*/, const CGUIListItem */*item*/ /* = nullptr */)
{
  switch (info)
  {

  case SYSTEM_FREE_MEMORY:
  case SYSTEM_USED_MEMORY:
  {
    MEMORYSTATUSEX stat;
    stat.dwLength = sizeof(MEMORYSTATUSEX);
    GlobalMemoryStatusEx(&stat);
    int memPercentUsed = static_cast<int>(100.0f* (stat.ullTotalPhys - stat.ullAvailPhys) / stat.ullTotalPhys + 0.5f);
    if (info == SYSTEM_FREE_MEMORY)
      value = 100 - memPercentUsed;
    else
      value = memPercentUsed;
    return true;
  }
  case SYSTEM_PROGRESS_BAR:
  {
    CGUIDialogProgress *bar = static_cast<CGUIDialogProgress *>(g_windowManager.GetWindow(WINDOW_DIALOG_PROGRESS));
    if (bar && bar->IsDialogRunning())
      value = bar->GetPercentage();
    return true;
  }
  case SYSTEM_FREE_SPACE:
  case SYSTEM_USED_SPACE:
  {
    g_sysinfo.GetHddSpaceInfo(value, info, true);
    return true;
  }
  case SYSTEM_CPU_USAGE:
    value = g_cpuInfo.getUsedPercentage();
    return true;
  case SYSTEM_BATTERY_LEVEL:
    value = g_powerManager.BatteryLevel();
    return true;
  }
  return false;
}
}