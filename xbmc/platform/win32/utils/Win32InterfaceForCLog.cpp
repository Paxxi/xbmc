/*
 *  Copyright (C) 2014-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#ifndef TARGET_WINDOWS
#error This file is for win32 platforms only
#endif //!TARGET_WINDOWS

#include "Win32InterfaceForCLog.h"

#include "utils/StringUtils.h"
#include "utils/auto_buffer.h"

#include "platform/win32/CharsetConverter.h"
#include "platform/win32/WIN32Util.h"

#include <Windows.h>

CWin32InterfaceForCLog::CWin32InterfaceForCLog()
  : m_hFile(INVALID_HANDLE_VALUE)
{
}

CWin32InterfaceForCLog::~CWin32InterfaceForCLog()
{
  if (m_hFile != INVALID_HANDLE_VALUE)
    CloseHandle(m_hFile);
}

bool CWin32InterfaceForCLog::OpenLogFile(const std::string& logFilename,
                                         const std::string& backupOldLogToFilename)
{
  if (m_hFile != INVALID_HANDLE_VALUE)
    return false; // file was already opened

  std::wstring strLogFileW(CWIN32Util::ConvertPathToWin32Form(CWIN32Util::SmbToUnc(logFilename)));
  std::wstring strLogFileOldW(
      CWIN32Util::ConvertPathToWin32Form(CWIN32Util::SmbToUnc(backupOldLogToFilename)));

  if (strLogFileW.empty())
    return false;

  if (!strLogFileOldW.empty())
  {
    (void)DeleteFileW(strLogFileOldW.c_str()); // if it's failed, try to continue
#ifdef TARGET_WINDOWS_STORE
    (void)MoveFileEx(strLogFileW.c_str(), strLogFileOldW.c_str(),
                     MOVEFILE_REPLACE_EXISTING); // if it's failed, try to continue
#else
    (void)MoveFileW(strLogFileW.c_str(), strLogFileOldW.c_str()); // if it's failed, try to continue
#endif
  }

#ifdef TARGET_WINDOWS_STORE
  m_hFile = CreateFile2(strLogFileW.c_str(), GENERIC_WRITE, FILE_SHARE_READ, CREATE_ALWAYS, NULL);
#else
  m_hFile = CreateFileW(strLogFileW.c_str(), GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
#endif

  if (m_hFile == INVALID_HANDLE_VALUE)
    return false;

  static const unsigned char BOM[3] = {0xEF, 0xBB, 0xBF};
  DWORD written;
  (void)WriteFile(m_hFile, BOM, sizeof(BOM), &written, NULL); // write BOM, ignore possible errors
  (void)FlushFileBuffers(m_hFile);

  return true;
}

void CWin32InterfaceForCLog::CloseLogFile(void)
{
  if (m_hFile != INVALID_HANDLE_VALUE)
  {
    CloseHandle(m_hFile);
    m_hFile = INVALID_HANDLE_VALUE;
  }
}

bool CWin32InterfaceForCLog::WriteStringToLog(const std::string& logString)
{
  if (m_hFile == INVALID_HANDLE_VALUE)
    return false;

  std::string strData(logString);
  StringUtils::Replace(strData, "\n", "\r\n");
  strData += "\r\n";

  DWORD written;
  const bool ret = (WriteFile(m_hFile, strData.c_str(), strData.length(), &written, NULL) != 0) &&
                   written == strData.length();

  return ret;
}

void CWin32InterfaceForCLog::PrintDebugString(const std::string& debugString)
{
#ifdef _DEBUG
  ::OutputDebugStringW(L"Debug Print: ");
  std::wstring debugMessage = KODI::PLATFORM::WINDOWS::ToW(debugString);
  ::OutputDebugStringW(debugMessage.c_str());
  ::OutputDebugStringW(L"\n");
#endif // _DEBUG
}

void CWin32InterfaceForCLog::GetCurrentLocalTime(
    int& year, int& month, int& day, int& hour, int& minute, int& second, double& millisecond)
{
  SYSTEMTIME time;
  GetLocalTime(&time);
  year = time.wYear;
  month = time.wMonth;
  day = time.wDay;
  hour = time.wHour;
  minute = time.wMinute;
  second = time.wSecond;
  millisecond = static_cast<double>(time.wMilliseconds);
}
