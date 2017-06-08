#ifndef __IPTC_PARSE_H
#define __IPTC_PARSE_H

#include "libexif.h"

class CIptcParse
{
  public:
    static bool Process(const unsigned char* const Data, const unsigned short itemlen, IPTCInfo_t *info);
};

#endif      // __IPTC_H
