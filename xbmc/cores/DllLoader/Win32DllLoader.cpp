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

#include "Win32DllLoader.h"
#include "DllLoaderContainer.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "filesystem/SpecialProtocol.h"
#include "platform/win32/CharsetConverter.h"


Win32DllLoader::Win32DllLoader(const std::string& dll, bool isSystemDll)
  : LibraryLoader(dll)
  , bIsSystemDll(isSystemDll)
{
  m_dllHandle = NULL;
  DllLoaderContainer::RegisterDll(this);
}

Win32DllLoader::~Win32DllLoader()
{
  if (m_dllHandle)
    Unload();
  DllLoaderContainer::UnRegisterDll(this);
}

bool Win32DllLoader::Load()
{
  using namespace KODI::PLATFORM::WINDOWS;
  if (m_dllHandle != NULL)
    return true;

  std::string strFileName = GetFileName();

  std::wstring strDllW = ToW(CSpecialProtocol::TranslatePath(strFileName));
  m_dllHandle = LoadLibraryExW(strDllW.c_str(), NULL, LOAD_WITH_ALTERED_SEARCH_PATH);
  if (!m_dllHandle)
  {
    DWORD dw = GetLastError();
    wchar_t* lpMsgBuf = NULL;
    DWORD strLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), (LPWSTR)&lpMsgBuf, 0, NULL);
    if (strLen == 0)
      strLen = FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), (LPWSTR)&lpMsgBuf, 0, NULL);

    if (strLen != 0)
    {
      std::string strMessage = FromW(lpMsgBuf, strLen);
      CLog::Log(LOGERROR, "%s: Failed to load \"%s\" with error %lu: \"%s\"", __FUNCTION__, CSpecialProtocol::TranslatePath(strFileName).c_str(), dw, strMessage.c_str());
    }
    else
      CLog::Log(LOGERROR, "%s: Failed to load \"%s\" with error %lu", __FUNCTION__, CSpecialProtocol::TranslatePath(strFileName).c_str(), dw);

    LocalFree(lpMsgBuf);
    return false;
  }

  return true;
}

void Win32DllLoader::Unload()
{
  if (m_dllHandle)
  {
    if (!FreeLibrary(m_dllHandle))
       CLog::Log(LOGERROR, "%s Unable to unload %s", __FUNCTION__, GetName());
  }

  m_dllHandle = nullptr;
}

int Win32DllLoader::ResolveExport(const char* symbol, void** f, bool logging)
{
  if (!m_dllHandle && !Load())
  {
    if (logging)
      CLog::Log(LOGWARNING, "%s - Unable to resolve: %s %s, reason: DLL not loaded", __FUNCTION__, GetName(), symbol);
    return 0;
  }

  void *s = GetProcAddress(m_dllHandle, symbol);

  if (!s)
  {
    if (logging)
      CLog::Log(LOGWARNING, "%s - Unable to resolve: %s %s", __FUNCTION__, GetName(), symbol);
    return 0;
  }

  *f = s;
  return 1;
}

bool Win32DllLoader::IsSystemDll()
{
  return bIsSystemDll;
}

HMODULE Win32DllLoader::GetHModule()
{
  return m_dllHandle;
}

bool Win32DllLoader::HasSymbols()
{
  return false;
}

void Win32DllLoader::OverrideImports(const std::string &dll)
{
}

bool Win32DllLoader::NeedsHooking(const char *dllName)
{
  return false;
}

void Win32DllLoader::RestoreImports()
{
}

bool Win32DllLoader::ResolveImport(const char *dllName, const char *functionName, void **fixup)
{
  return true;
}

bool Win32DllLoader::ResolveOrdinal(const char *dllName, unsigned long ordinal, void **fixup)
{
  return true;
}