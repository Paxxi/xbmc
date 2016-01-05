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
#pragma once

#include <dns_sd.h>

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "network/Zeroconf.h"
#include "threads/CriticalSection.h"
#include "threads/Thread.h"


class CZeroconfMDNS : public CZeroconf, public CThread
{
public:
  CZeroconfMDNS();
  ~CZeroconfMDNS();

protected:

  //CThread interface
  void Process() override;

  //implement base CZeroConf interface
  bool doPublishService(const std::string& fcr_identifier,
                        const std::string& fcr_type,
                        const std::string& fcr_name,
                        unsigned int f_port,
                        const std::vector<std::pair<std::string, std::string> >& txt) override;

  bool doForceReAnnounceService(const std::string& fcr_identifier) override;
  bool doRemoveService(const std::string& fcr_ident) override;

  void doStop() override;

  bool IsZCdaemonRunning() override;

  void ProcessResults() override;

private:

  static void DNSSD_API registerCallback(DNSServiceRef sdref,
                                         const DNSServiceFlags flags,
                                         DNSServiceErrorType errorCode,
                                         const char *name,
                                         const char *regtype,
                                         const char *domain,
                                         void *context);


  //lock + data (accessed from runloop(main thread) + the rest)
  CCriticalSection m_data_guard;
  struct tServiceRef
  {
    tServiceRef(DNSServiceRef service_ref, const TXTRecordRef& txt_record_ref, int update_number);
    ~tServiceRef();

    tServiceRef(const tServiceRef& other) = default;

    tServiceRef(tServiceRef&& other)
      : serviceRef(other.serviceRef)
      , txtRecordRef(std::move(other.txtRecordRef))
      , updateNumber(other.updateNumber)
    {
    }

    tServiceRef& operator=(const tServiceRef& other) = default;

    //There's a VS bug with the default move operator in 2013, can replace this once
    //we upgrade
    tServiceRef& operator=(tServiceRef&& other)
    {
      if (this == &other)
        return *this;

      serviceRef = other.serviceRef;
      txtRecordRef = std::move(other.txtRecordRef);
      updateNumber = other.updateNumber;
      return *this;
    }

    DNSServiceRef serviceRef;
    TXTRecordRef txtRecordRef;
    int updateNumber;
  };
  std::map<std::string, struct tServiceRef> m_services;
  DNSServiceRef m_service;
};
