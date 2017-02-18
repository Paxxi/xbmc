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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include <string>

#include "LibraryLoader.h"

class Win32DllLoader : public LibraryLoader
{
public:

  Win32DllLoader(const std::string& dll, bool isSystemDll);
  ~Win32DllLoader();

  bool Load() override;
  void Unload() override;

  int ResolveExport(const char* symbol, void** ptr, bool logging = true) override;
  bool IsSystemDll() override;
  HMODULE GetHModule() override;
  bool HasSymbols() override;

private:
  void OverrideImports(const std::string &dll);
  void RestoreImports();
  static bool ResolveImport(const char *dllName, const char *functionName, void **fixup);
  static bool ResolveOrdinal(const char *dllName, unsigned long ordinal, void **fixup);
  bool NeedsHooking(const char *dllName);

  HMODULE m_dllHandle;
  bool bIsSystemDll;
};

