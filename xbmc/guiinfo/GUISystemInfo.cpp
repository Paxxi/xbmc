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
#include "utils/SystemInfo.h"
#include "guilib/LocalizeStrings.h"
#include "settings/DisplaySettings.h"
#include "utils/StringUtils.h"
#include "settings/Settings.h"
#include "profiles/ProfilesManager.h"
#include "utils/AlarmClock.h"
#include "guilib/GraphicContext.h"
#include "guilib/GUIControl.h"
#include "storage/MediaManager.h"
#include "powermanagement/PowerManager.h"
#include "utils/CPUInfo.h"
#include "dialogs/GUIDialogProgress.h"
#include "Application.h"
#include "settings/AdvancedSettings.h"
#include "utils/log.h"
#include "utils/TimeUtils.h"

namespace GUIINFO
{
#if defined(TARGET_LINUX) || defined(TARGET_FREEBSD)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_LINUX;
#elif defined(TARGET_WINDOWS)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_WINDOWS;
#elif defined(TARGET_DARWIN_OSX)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_DARWIN_OSX;
#define PLATFORM_DARWIN true
#elif defined(TARGET_DARWIN_IOS)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_DARWIN_IOS
#define PLATFORM_DARWIN true
#elif defined(TARGET_DARWIN_IOS_ATV2)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_DARWIN_ATV2
#define PLATFORM_DARWIN true
#elif defined(TARGET_ANDROID)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_ANDROID
#elif defined(TARGET_RASPBERRY_PI)
#define CURRENT_PLATFORM SYSTEM_PLATFORM_LINUX_RASPBERRY_PI
#endif

#if !defined(PLATFORM_DARWIN)
#define PLATFORM_DARWIN false
#endif

int CGUISystemInfo::LabelMask()
{
  return SYSTEM_MASK;
}

std::string CGUISystemInfo::GetScreenResolution() const
{
  if (g_Windowing.IsFullScreen())
  {
    return StringUtils::Format("%ix%i@%.2fHz - %s (%02.2f fps)",
                               CDisplaySettings::GetInstance().GetCurrentResolutionInfo().iScreenWidth,
                               CDisplaySettings::GetInstance().GetCurrentResolutionInfo().iScreenHeight,
                               CDisplaySettings::GetInstance().GetCurrentResolutionInfo().fRefreshRate,
                               g_localizeStrings.Get(244).c_str(),
                               m_manager->GetFPS());
  }

  return StringUtils::Format("%ix%i - %s (%02.2f fps)",
                             CDisplaySettings::GetInstance().GetCurrentResolutionInfo().iScreenWidth,
                             CDisplaySettings::GetInstance().GetCurrentResolutionInfo().iScreenHeight,
                             g_localizeStrings.Get(242).c_str(),
                             m_manager->GetFPS());
}

std::string CGUISystemInfo::GetMemory(int info) const
{
  MEMORYSTATUSEX stat;
  stat.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&stat);
  auto iMemPercentFree = 100 - static_cast<int>(100.0f* (stat.ullTotalPhys - stat.ullAvailPhys) / stat.ullTotalPhys + 0.5f);
  auto iMemPercentUsed = 100 - iMemPercentFree;

  if (info == SYSTEM_FREE_MEMORY)
    return StringUtils::Format("%luMB", static_cast<ULONG>((stat.ullAvailPhys / MB)));
  if (info == SYSTEM_FREE_MEMORY_PERCENT)
    return StringUtils::Format("%i%%", iMemPercentFree);
  if (info == SYSTEM_USED_MEMORY)
    return StringUtils::Format("%luMB", static_cast<ULONG>(((stat.ullTotalPhys - stat.ullAvailPhys) / MB)));
  if (info == SYSTEM_USED_MEMORY_PERCENT)
    return StringUtils::Format("%i%%", iMemPercentUsed);
  if (info == SYSTEM_TOTAL_MEMORY)
    return StringUtils::Format("%luMB", static_cast<ULONG>((stat.ullTotalPhys / MB)));

  return std::string();
}

std::string CGUISystemInfo::GetLabel(CFileItem* /*currentFile*/, int info, int /*contextWindow*/, std::string *fallback)
{
  auto strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  case SYSTEM_FREE_SPACE:
  case SYSTEM_USED_SPACE:
  case SYSTEM_TOTAL_SPACE:
  case SYSTEM_FREE_SPACE_PERCENT:
  case SYSTEM_USED_SPACE_PERCENT:
    strLabel = g_sysinfo.GetHddSpaceInfo(info);
    break;

  case SYSTEM_CPU_TEMPERATURE:
  case SYSTEM_GPU_TEMPERATURE:
  case SYSTEM_FAN_SPEED:
  case SYSTEM_CPU_USAGE:
    strLabel = GetSystemHeatInfo(info);
    break;

  case SYSTEM_VIDEO_ENCODER_INFO:
  case SYSTEM_OS_VERSION_INFO:
  case SYSTEM_CPUFREQUENCY:
  case SYSTEM_INTERNET_STATE:
  case SYSTEM_UPTIME:
  case SYSTEM_TOTALUPTIME:
  case SYSTEM_BATTERY_LEVEL:
    strLabel = g_sysinfo.GetInfo(info);
    break;

  case SYSTEM_SCREEN_RESOLUTION:
    strLabel = GetScreenResolution();
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
    strLabel = GetMemory(info);
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
    strLabel = g_localizeStrings.Get(g_windowManager.GetFocusedWindow());
    break;
  case SYSTEM_STARTUP_WINDOW:
    strLabel = StringUtils::Format("%i", CSettings::GetInstance().GetInt("lookandfeel.startupwindow"));
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
    strLabel = CProfilesManager::GetInstance().GetCurrentProfile().getName();
    break;
  case SYSTEM_PROFILECOUNT:
    strLabel = StringUtils::Format("%" PRIuS, CProfilesManager::GetInstance().GetNumberOfProfiles());
    break;
  case SYSTEM_PROFILEAUTOLOGIN:
  {
    auto profileId = CProfilesManager::GetInstance().GetAutoLoginProfileId();
    if ((profileId < 0) || (!CProfilesManager::GetInstance().GetProfileName(profileId, strLabel)))
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
    auto stereoMode = CSettings::GetInstance().GetInt("videoscreen.stereoscopicmode");
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

bool CGUISystemInfo::GetInt(int &value, int info, int /*contextWindow*/, const CGUIListItem *item /*= nullptr */)
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

bool CGUISystemInfo::GetBool(int condition, int contextWindow, const CGUIListItem* item)
{
  switch (condition)
  {
  case SYSTEM_ALWAYS_TRUE:
    return true;
  case SYSTEM_ALWAYS_FALSE:
    return false;
  case SYSTEM_ETHERNET_LINK_ACTIVE:
    return true;
  case SYSTEM_PLATFORM_LINUX:
  case SYSTEM_PLATFORM_WINDOWS:
  case SYSTEM_PLATFORM_ANDROID:
  case SYSTEM_PLATFORM_LINUX_RASPBERRY_PI:
  case SYSTEM_PLATFORM_DARWIN_OSX:
  case SYSTEM_PLATFORM_DARWIN_IOS:
  case SYSTEM_PLATFORM_DARWIN_ATV2:
    return condition == CURRENT_PLATFORM;
  case SYSTEM_PLATFORM_DARWIN:
    return PLATFORM_DARWIN;
  case SYSTEM_CAN_POWERDOWN:
    return g_powerManager.CanPowerdown();
  case SYSTEM_CAN_SUSPEND:
    return g_powerManager.CanSuspend();
  case SYSTEM_CAN_HIBERNATE:
    return g_powerManager.CanHibernate();
  case SYSTEM_CAN_REBOOT:
    return g_powerManager.CanReboot();
  case SYSTEM_SCREENSAVER_ACTIVE:
    return g_application.IsInScreenSaver();
  case SYSTEM_DPMS_ACTIVE:
    return g_application.IsDPMSActive();
  case SYSTEM_HASLOCKS:
    return CProfilesManager::GetInstance().GetMasterProfile()
      .getLockMode() != LOCK_MODE_EVERYONE;
  case SYSTEM_ISMASTER:
    return CProfilesManager::GetInstance().GetMasterProfile()
      .getLockMode() != LOCK_MODE_EVERYONE && 
      g_passwordManager.bMasterUser;
  case SYSTEM_HAS_PVR:
  case SYSTEM_HAS_ADSP:
    return true;
  case SYSTEM_ISFULLSCREEN:
    return g_Windowing.IsFullScreen();
  case SYSTEM_ISSTANDALONE:
    return g_application.IsStandAlone();
  case SYSTEM_ISINHIBIT:
    return g_application.IsIdleShutdownInhibited();
  case SYSTEM_HAS_SHUTDOWN:
    return (CSettings::GetInstance().GetInt(
      CSettings::SETTING_POWERMANAGEMENT_SHUTDOWNTIME) > 0);
  case SYSTEM_LOGGEDON:
    return !(g_windowManager.GetActiveWindow() == WINDOW_LOGIN_SCREEN);
  case SYSTEM_SHOW_EXIT_BUTTON:
    return g_advancedSettings.m_showExitButton;
  case SYSTEM_HAS_LOGINSCREEN:
    return CProfilesManager::GetInstance().UsingLoginScreen();
  case SYSTEM_INTERNET_STATE:
    g_sysinfo.GetInfo(condition);
    return g_sysinfo.HasInternet();
  case SYSTEM_MEDIA_DVD:
    return g_mediaManager.IsDiscInDrive();

#if defined(HAS_DVD_DRIVE)
  case SYSTEM_DVDREADY:
    return g_mediaManager.GetDriveStatus() != DRIVE_NOT_READY;
  case SYSTEM_TRAYOPEN:
    return g_mediaManager.GetDriveStatus() == DRIVE_OPEN;
#endif
  }

  CLog::Log(LOGERROR, "%s: Called with unkown infolabel %d", __FUNCTION__, condition);
  return false;
}

std::string CGUISystemInfo::GetSystemHeatInfo(int info)
{
#define SYSHEATUPDATEINTERVAL 60000
  if (m_lastSysHeatInfoTime == 0 ||
    CTimeUtils::GetFrameTime() - m_lastSysHeatInfoTime >= SYSHEATUPDATEINTERVAL)
  { // update our variables
    m_lastSysHeatInfoTime = CTimeUtils::GetFrameTime();
#if defined(TARGET_POSIX)
    g_cpuInfo.getTemperature(m_cpuTemp);
    m_gpuTemp = GetGPUTemperature();
#endif
  }

  std::string text;
  switch (info)
  {
  case SYSTEM_CPU_TEMPERATURE:
    return m_cpuTemp.IsValid() ? g_langInfo.GetTemperatureAsString(m_cpuTemp) : "?";

  case SYSTEM_GPU_TEMPERATURE:
    return m_gpuTemp.IsValid() ? g_langInfo.GetTemperatureAsString(m_gpuTemp) : "?";

  case SYSTEM_FAN_SPEED:
    text = StringUtils::Format("%i%%", m_fanSpeed * 2);
    break;

  case SYSTEM_CPU_USAGE:
#if defined(TARGET_DARWIN_OSX)
    text = StringUtils::Format("%4.2f%%", m_resourceCounter.GetCPUUsage());
#elif defined(TARGET_DARWIN) || defined(TARGET_WINDOWS)
    text = StringUtils::Format("%d%%", g_cpuInfo.getUsedPercentage());
#else
    text = StringUtils::Format("%s", g_cpuInfo.GetCoresUsageString().c_str());
#endif
    break;
  }
  return text;
}

CTemperature CGUISystemInfo::GetGPUTemperature() const
{
  int  value = 0;
  char scale = 0;

#if defined(TARGET_DARWIN_OSX)
  value = SMCGetTemperature(SMC_KEY_GPU_TEMP);
  return CTemperature::CreateFromCelsius(value);
#else
  auto cmd = g_advancedSettings.m_gpuTempCmd;
  int ret = 0;
  FILE *p = nullptr;

  if (cmd.empty() || !(p = popen(cmd.c_str(), "r")))
    return CTemperature();

  ret = fscanf(p, "%d %c", &value, &scale);
  pclose(p);

  if (ret != 2)
    return CTemperature();
#endif

  if (scale == 'C' || scale == 'c')
    return CTemperature::CreateFromCelsius(value);
  if (scale == 'F' || scale == 'f')
    return CTemperature::CreateFromFahrenheit(value);
  return CTemperature();
}
}