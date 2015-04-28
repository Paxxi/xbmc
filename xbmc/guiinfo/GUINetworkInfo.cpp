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

#include "GUINetworkInfo.h"
#include "GUIInfoManager.h"
#include "GUIInfoLabels.h"
#include "Application.h"
#include <xbmc/utils/SystemInfo.h>
#include <xbmc/guilib/LocalizeStrings.h>
#include <xbmc/network/Network.h>

namespace GUIINFO
{
int CGUINetworkInfo::LabelMask()
{
  return NETWORK_MASK;
}

std::string CGUINetworkInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string* fallback)
{
  std::string strLabel = fallback == nullptr ? "" : *fallback;

  switch (info)
  {
  case NETWORK_MAC_ADDRESS:
    return g_sysinfo.GetInfo(info);
    break;
  case NETWORK_IP_ADDRESS:
  {
    CNetworkInterface* iface = g_application.getNetwork().GetFirstConnectedInterface();
    if (iface)
      return iface->GetCurrentIPAddress();
  }
  break;
  case NETWORK_SUBNET_MASK:
  {
    CNetworkInterface* iface = g_application.getNetwork().GetFirstConnectedInterface();
    if (iface)
      return iface->GetCurrentNetmask();
  }
  break;
  case NETWORK_GATEWAY_ADDRESS:
  {
    CNetworkInterface* iface = g_application.getNetwork().GetFirstConnectedInterface();
    if (iface)
      return iface->GetCurrentDefaultGateway();
  }
  break;
  case NETWORK_DNS1_ADDRESS:
  {
    std::vector<std::string> nss = g_application.getNetwork().GetNameServers();
    if (nss.size() >= 1)
      return nss[0];
  }
  break;
  case NETWORK_DNS2_ADDRESS:
  {
    std::vector<std::string> nss = g_application.getNetwork().GetNameServers();
    if (nss.size() >= 2)
      return nss[1];
  }
  break;
  case NETWORK_DHCP_ADDRESS:
  {
    std::string dhcpserver;
    return dhcpserver;
  }
  break;
  case NETWORK_LINK_STATE:
  {
    std::string linkStatus = g_localizeStrings.Get(151);
    linkStatus += " ";
    CNetworkInterface* iface = g_application.getNetwork().GetFirstConnectedInterface();
    if (iface && iface->IsConnected())
      linkStatus += g_localizeStrings.Get(15207);
    else
      linkStatus += g_localizeStrings.Get(15208);
    return linkStatus;
  }
  break;
  default:
    break;
  }
  return strLabel;
}

bool CGUINetworkInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item) 
{

  return false;
}

}