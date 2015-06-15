#pragma once
/*
 *      Copyright (C) 2013 Team XBMC
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

#include "JSONRPC.h"

class ISetting;
class CSettingSection;
class CSettingCategory;
class CSettingGroup;
class CSetting;
class CSettingBool;
class CSettingInt;
class CSettingNumber;
class CSettingString;
class CSettingAction;
class CSettingList;
class CSettingPath;
class CSettingAddon;
class ISettingControl;

namespace JSONRPC
{
  class CSettingsOperations
  {
  public:
    static JSONRPC_STATUS GetSections(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetCategories(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS GetSettings(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    
    static JSONRPC_STATUS GetSettingValue(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS SetSettingValue(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);
    static JSONRPC_STATUS ResetSettingValue(const std::string &method, ITransportLayer *transport, IClient *client, const KODI::UTILS::CVariant &parameterObject, KODI::UTILS::CVariant &result);

  private:
    static int ParseSettingLevel(const std::string &strLevel);

    static bool SerializeISetting(const ISetting* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingSection(const CSettingSection* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingCategory(const CSettingCategory* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingGroup(const CSettingGroup* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSetting(const CSetting* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingBool(const CSettingBool* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingInt(const CSettingInt* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingNumber(const CSettingNumber* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingString(const CSettingString* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingAction(const CSettingAction* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingList(const CSettingList* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingPath(const CSettingPath* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingAddon(const CSettingAddon* setting, KODI::UTILS::CVariant &obj);
    static bool SerializeSettingControl(const ISettingControl* control, KODI::UTILS::CVariant &obj);

    static void SerializeSettingListValues(const std::vector<KODI::UTILS::CVariant> &values, KODI::UTILS::CVariant &obj);
  };
}
