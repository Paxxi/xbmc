#pragma once
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

#include <string>
#include <vector>
#include "network/Network.h"
#include "Iphlpapi.h"
#include "utils/stopwatch.h"
#include "threads/CriticalSection.h"

class CNetworkWin32;

class CNetworkInterfaceWin32 : public CNetworkInterface
{
public:
  CNetworkInterfaceWin32(CNetworkWin32* network, const IP_ADAPTER_INFO& adapter);
  ~CNetworkInterfaceWin32();

  std::string& GetName() override;

  bool IsEnabled() override;
  bool IsConnected() override;
  bool IsWireless() override;

  std::string GetMacAddress() override;
  void GetMacAddressRaw(char rawMac[6]) override;

  bool GetHostMacAddress(unsigned long host, std::string& mac) override;

  std::string GetCurrentIPAddress() override;
  std::string GetCurrentNetmask() override;
  std::string GetCurrentDefaultGateway() override;
  std::string GetCurrentWirelessEssId() override;

  void GetSettings(NetworkAssignment& assignment, std::string& ipAddress, std::string& networkMask, std::string& defaultGateway, std::string& essId, std::string& key, EncMode& encryptionMode) override;
  void SetSettings(NetworkAssignment& assignment, std::string& ipAddress, std::string& networkMask, std::string& defaultGateway, std::string& essId, std::string& key, EncMode& encryptionMode) override;

  // Returns the list of access points in the area
  std::vector<NetworkAccessPoint> GetAccessPoints() override;

private:
  void WriteSettings(FILE* fw, NetworkAssignment assignment, std::string& ipAddress, std::string& networkMask, std::string& defaultGateway, std::string& essId, std::string& key, EncMode& encryptionMode);
  IP_ADAPTER_INFO m_adapter;
  CNetworkWin32* m_network;
  std::string m_adaptername;
};

class CNetworkWin32 : public CNetwork
{
public:
  CNetworkWin32();
  virtual ~CNetworkWin32();

  // Return the list of interfaces
  std::vector<CNetworkInterface*>& GetInterfaceList() override;

  // Ping remote host
  bool PingHost(unsigned long host, unsigned int timeout_ms = 2000) override;

  // Get/set the nameserver(s)
  std::vector<std::string> GetNameServers() override;
  void SetNameServers(const std::vector<std::string>& nameServers) override;

  friend class CNetworkInterfaceWin32;

private:
  int GetSocket() const { return m_sock; }
  void queryInterfaceList();
  void CleanInterfaceList();
  std::vector<CNetworkInterface*> m_interfaces;
  int m_sock;
  CStopWatch m_netrefreshTimer;
  CCriticalSection m_critSection;
};
