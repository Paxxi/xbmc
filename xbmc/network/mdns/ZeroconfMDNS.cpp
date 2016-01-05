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

#include "ZeroconfMDNS.h"

#include <sstream>

#include "dialogs/GUIDialogKaiToast.h"
#include "guilib/LocalizeStrings.h"
#include "threads/SingleLock.h"
#include "utils/log.h"
#if defined(TARGET_WINDOWS)
#include "win32/WIN32Util.h"
#endif //TARGET_WINDOWS

#if defined(HAS_MDNS_EMBEDDED)
#include <mDnsEmbedded.h>
#endif //HAS_MDNS_EMBEDDED

#pragma comment(lib, "dnssd.lib")

extern HWND g_hWnd;

void CZeroconfMDNS::Process()
{
#if defined(HAS_MDNS_EMBEDDED)
  CLog::Log(LOGDEBUG, "ZeroconfEmbedded - processing...");
  struct timeval timeout;
  timeout.tv_sec = 1;
  while (( !m_bStop ))
    embedded_mDNSmainLoop(timeout);
#endif //HAS_MDNS_EMBEDDED

}


CZeroconfMDNS::CZeroconfMDNS()  : CThread("ZerocconfEmbedded")
{
  m_service = nullptr;
#if defined(HAS_MDNS_EMBEDDED)
  embedded_mDNSInit();
  Create();
#endif //HAS_MDNS_EMBEDDED
}

CZeroconfMDNS::~CZeroconfMDNS()
{
  CZeroconfMDNS::doStop();
#if defined(HAS_MDNS_EMBEDDED)
  StopThread();
  embedded_mDNSExit();
#endif //HAS_MDNS_EMBEDDED
}

bool CZeroconfMDNS::IsZCdaemonRunning()
{
#if !defined(HAS_MDNS_EMBEDDED)
  uint32_t version;
  //replace with 4? not sure if there's any weird platform where it would be wrong
  uint32_t size = static_cast<uint32_t>(sizeof(version)); //sizeof returns size_t which can be 64 bit on Windows
  DNSServiceErrorType err = DNSServiceGetProperty(kDNSServiceProperty_DaemonVersion, &version, &size);
  if(err != kDNSServiceErr_NoError)
  {
    CLog::Log(LOGERROR, "ZeroconfMDNS: Zeroconf can't be started probably because Apple's Bonjour Service isn't installed. You can get it by either installing Itunes or Apple's Bonjour Print Service for Windows (http://support.apple.com/kb/DL999)");
    CGUIDialogKaiToast::QueueNotification(CGUIDialogKaiToast::Error, g_localizeStrings.Get(34300), g_localizeStrings.Get(34301), 10000, true);
    return false;
  }
  CLog::Log(LOGDEBUG, "ZeroconfMDNS:Bonjour version is %d.%d", version / 10000, version / 100 % 100);
#endif //!HAS_MDNS_EMBEDDED
  return true;
}

//methods to implement for concrete implementations
bool CZeroconfMDNS::doPublishService(const std::string& fcr_identifier,
                      const std::string& fcr_type,
                      const std::string& fcr_name,
                      unsigned int f_port,
                      const std::vector<std::pair<std::string, std::string> >& txt)
{
  DNSServiceRef netService = nullptr;
  TXTRecordRef txtRecord;
  DNSServiceErrorType err;
  TXTRecordCreate(&txtRecord, 0, nullptr);

#if !defined(HAS_MDNS_EMBEDDED)
  CSingleLock lock(m_data_guard);
  if(m_service == nullptr)
  {
    err = DNSServiceCreateConnection(&m_service);
    if (err != kDNSServiceErr_NoError)
    {
      CLog::Log(LOGERROR, "ZeroconfMDNS: DNSServiceCreateConnection failed with error = %ld", static_cast<int>(err));
      return false;
    }
    err = WSAAsyncSelect( static_cast<SOCKET>(DNSServiceRefSockFD( m_service )), g_hWnd, BONJOUR_EVENT, FD_READ | FD_CLOSE );
    if (err != kDNSServiceErr_NoError)
      CLog::Log(LOGERROR, "ZeroconfMDNS: WSAAsyncSelect failed with error = %ld", static_cast<int>(err));
  }
#endif //!HAS_MDNS_EMBEDDED

  CLog::Log(LOGDEBUG, "ZeroconfMDNS: identifier: %s type: %s name:%s port:%i", fcr_identifier.c_str(), fcr_type.c_str(), fcr_name.c_str(), f_port);

  //add txt records
  if(!txt.empty())
  {
    for (const auto& record : txt)
    {
      CLog::Log(LOGDEBUG, "ZeroconfMDNS: key:%s, value:%s",record.first.c_str(), record.second.c_str());
      uint8_t txtLen = static_cast<uint8_t>(strlen(record.second.c_str()));
      TXTRecordSetValue(&txtRecord, record.first.c_str(), txtLen, record.second.c_str());
    }
  }

  {
    //avoid warning about hiding variable
    CSingleLock lock2(m_data_guard);
    netService = m_service;
    err = DNSServiceRegister(&netService, kDNSServiceFlagsShareConnection, 0, fcr_name.c_str(), fcr_type.c_str(), nullptr, nullptr, htons(f_port), TXTRecordGetLength(&txtRecord), TXTRecordGetBytesPtr(&txtRecord), registerCallback, nullptr);
  }

  if (err != kDNSServiceErr_NoError)
  {
    // Something went wrong so lets clean up.
    if (netService)
      DNSServiceRefDeallocate(netService);

    CLog::Log(LOGERROR, "ZeroconfMDNS: DNSServiceRegister returned (error = %ld)", static_cast<int>(err));
  }
  else
  {
    //avoid warning about hiding variable
    CSingleLock lock2(m_data_guard);
    m_services.insert(std::make_pair(fcr_identifier, tServiceRef(netService, txtRecord, 0)));
  }

  return err == kDNSServiceErr_NoError;
}

bool CZeroconfMDNS::doForceReAnnounceService(const std::string& fcr_identifier)
{
  CSingleLock lock(m_data_guard);
  auto it = m_services.find(fcr_identifier);
  if(it != m_services.end())
  {
    // for force announcing a service with mdns we need
    // to change a txt record - so we diddle between
    // even and odd dummy records here
    if ( (it->second.updateNumber % 2) == 0)
      TXTRecordSetValue(&it->second.txtRecordRef, "xbmcdummy", strlen("evendummy"), "evendummy");
    else
      TXTRecordSetValue(&it->second.txtRecordRef, "xbmcdummy", strlen("odddummy"), "odddummy");

    it->second.updateNumber++;

    return DNSServiceUpdateRecord(
      it->second.serviceRef, nullptr, 0, TXTRecordGetLength(&it->second.txtRecordRef), TXTRecordGetBytesPtr(&it->second.txtRecordRef), 0)
      ==  kDNSServiceErr_NoError;
  }
  return false;
}

bool CZeroconfMDNS::doRemoveService(const std::string& fcr_ident)
{
  CSingleLock lock(m_data_guard);
  auto it = m_services.find(fcr_ident);
  if(it != m_services.end())
  {
    m_services.erase(it);
    CLog::Log(LOGDEBUG, "ZeroconfMDNS: Removed service %s", fcr_ident.c_str());
    return true;
  }

  return false;
}

void CZeroconfMDNS::doStop()
{
  {
    CSingleLock lock(m_data_guard);
    CLog::Log(LOGDEBUG, "ZeroconfMDNS: Shutdown services");
    //Don't want logging in a destructor so leave it here, not sure
    //if we need it at all?
    for (auto& service : m_services)
    {
      CLog::Log(LOGDEBUG, "ZeroconfMDNS: Removed service %s", service.first.c_str());
    }
    m_services.clear();
  }
  {
    CSingleLock lock(m_data_guard);
#if defined(TARGET_WINDOWS)
    WSAAsyncSelect( static_cast<SOCKET>(DNSServiceRefSockFD( m_service )), g_hWnd, BONJOUR_EVENT, 0 );
#endif //TARGET_WINDOWS

    if (m_service)
      DNSServiceRefDeallocate(m_service);

    m_service = nullptr;
  }
}

void DNSSD_API CZeroconfMDNS::registerCallback(DNSServiceRef sdref, const DNSServiceFlags flags, DNSServiceErrorType errorCode, const char *name, const char *regtype, const char *domain, void *context)
{
  (void)sdref;    // Unused
  (void)flags;    // Unused
  (void)context;  // Unused

  if (errorCode == kDNSServiceErr_NoError)
  {
    if (flags & kDNSServiceFlagsAdd)
      CLog::Log(LOGDEBUG, "ZeroconfMDNS: %s.%s%s now registered and active", name, regtype, domain);
    else
      CLog::Log(LOGDEBUG, "ZeroconfMDNS: %s.%s%s registration removed", name, regtype, domain);
  }
  else if (errorCode == kDNSServiceErr_NameConflict)
     CLog::Log(LOGDEBUG, "ZeroconfMDNS: %s.%s%s Name in use, please choose another", name, regtype, domain);
  else
    CLog::Log(LOGDEBUG, "ZeroconfMDNS: %s.%s%s error code %d", name, regtype, domain, errorCode);
}

CZeroconfMDNS::tServiceRef::tServiceRef(DNSServiceRef service_ref, const TXTRecordRef& txt_record_ref, int update_number)
  : serviceRef{service_ref}
  , updateNumber{update_number}
{
  txtRecordRef = txt_record_ref;
}

CZeroconfMDNS::tServiceRef::~tServiceRef()
{
  DNSServiceRefDeallocate(serviceRef);
  TXTRecordDeallocate(&txtRecordRef);
}

void CZeroconfMDNS::ProcessResults()
{
  CSingleLock lock(m_data_guard);
  DNSServiceErrorType err = DNSServiceProcessResult(m_service);
  if (err != kDNSServiceErr_NoError)
    CLog::Log(LOGERROR, "ZeroconfMDNS: DNSServiceProcessResult returned (error = %ld)", static_cast<int>(err));
}

