/*
 *  Copyright (C) 2005-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "network/Network.h"
#include "threads/CriticalSection.h"
#include "utils/stopwatch.h"

#include <string>
#include <vector>

class CNetworkWin32 : public CNetworkBase
{
public:
  CNetworkWin32();
  ~CNetworkWin32(void) override;

  // Return the list of interfaces
  virtual std::vector<CNetworkInterface*>& GetInterfaceList(void) override;

  // Ping remote host
  using CNetworkBase::PingHost;
  bool PingHost(unsigned long host, unsigned int timeout_ms = 2000) override;
  bool PingHost(const struct sockaddr& host, unsigned int timeout_ms = 2000);

  // Get/set the nameserver(s)
  std::vector<std::string> GetNameServers(void) override;

  friend class CNetworkInterfaceWin32;

private:
  int GetSocket() { return m_sock; }
  void queryInterfaceList();
  void CleanInterfaceList();
  std::vector<CNetworkInterface*> m_interfaces;
  int m_sock;
  CStopWatch m_netrefreshTimer;
  CCriticalSection m_critSection;
};

using CNetwork = CNetworkWin32;
