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

#include "Peripheral.h"

#include <utility>

#include "guilib/LocalizeStrings.h"
#include "peripherals/Peripherals.h"
#include "settings/lib/Setting.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXML.h"
#include "utils/XMLUtils.h"

using namespace PERIPHERALS;

struct SortBySettingsOrder
{
  bool operator()(const PeripheralDeviceSetting &left, const PeripheralDeviceSetting& right)
  {
    return left.m_order < right.m_order;
  }
};

CPeripheral::CPeripheral(const PeripheralScanResult& scanResult) :
  m_type(scanResult.m_mappedType),
  m_busType(scanResult.m_busType),
  m_mappedBusType(scanResult.m_mappedBusType),
  m_strLocation(scanResult.m_strLocation),
  m_strDeviceName(scanResult.m_strDeviceName),
  m_iVendorId(scanResult.m_iVendorId),
  m_iProductId(scanResult.m_iProductId),
  m_strVersionInfo(g_localizeStrings.Get(13205)), // "unknown"
  m_bInitialised(false),
  m_bHidden(false),
  m_bError(false)
{
  PeripheralTypeTranslator::FormatHexString(scanResult.m_iVendorId, m_strVendorId);
  PeripheralTypeTranslator::FormatHexString(scanResult.m_iProductId, m_strProductId);
  if (scanResult.m_iSequence > 0)
  {
    m_strFileLocation = StringUtils::Format("peripherals://%s/%s_%d.dev",
                                            PeripheralTypeTranslator::BusTypeToString(scanResult.m_busType),
                                            scanResult.m_strLocation.c_str(),
                                            scanResult.m_iSequence);
  }
  else
  {
    m_strFileLocation = StringUtils::Format("peripherals://%s/%s.dev",
                                            PeripheralTypeTranslator::BusTypeToString(scanResult.m_busType),
                                            scanResult.m_strLocation.c_str());
  }
}

CPeripheral::~CPeripheral()
{
  PersistSettings(true);

  for (auto & m_subDevice : m_subDevices)
    delete m_subDevice;
  m_subDevices.clear();

  ClearSettings();
}

bool CPeripheral::operator ==(const CPeripheral &right) const
{
  return m_type == right.m_type &&
      m_strLocation == right.m_strLocation &&
      m_iVendorId == right.m_iVendorId &&
      m_iProductId == right.m_iProductId;
}

bool CPeripheral::operator !=(const CPeripheral &right) const
{
  return !(*this == right);
}

bool CPeripheral::HasFeature(const PeripheralFeature feature) const
{
  bool bReturn(false);

  for (auto m_feature : m_features)
  {
    if (m_feature == feature)
    {
      bReturn = true;
      break;
    }
  }

  if (!bReturn)
  {
    for (auto m_subDevice : m_subDevices)
    {
      if (m_subDevice->HasFeature(feature))
      {
        bReturn = true;
        break;
      }
    }
  }

  return bReturn;
}

void CPeripheral::GetFeatures(std::vector<PeripheralFeature> &features) const
{
  for (auto m_feature : m_features)
    features.push_back(m_feature);

  for (auto m_subDevice : m_subDevices)
    m_subDevice->GetFeatures(features);
}

bool CPeripheral::Initialise()
{
  bool bReturn(false);

  if (m_bError)
    return bReturn;

  bReturn = true;
  if (m_bInitialised)
    return bReturn;

  g_peripherals.GetSettingsFromMapping(*this);
  m_strSettingsFile = StringUtils::Format("special://profile/peripheral_data/%s_%s_%s.xml",
                                          PeripheralTypeTranslator::BusTypeToString(m_mappedBusType),
                                          m_strVendorId.c_str(),
                                          m_strProductId.c_str());
  LoadPersistedSettings();

  for (auto feature : m_features)
  {
    bReturn &= InitialiseFeature(feature);
  }

  for (auto & m_subDevice : m_subDevices)
    bReturn &= m_subDevice->Initialise();

  if (bReturn)
  {
    CLog::Log(LOGDEBUG, "%s - initialised peripheral on '%s' with %d features and %d sub devices",
      __FUNCTION__, m_strLocation.c_str(), (int)m_features.size(), (int)m_subDevices.size());
    m_bInitialised = true;
  }

  return bReturn;
}

void CPeripheral::GetSubdevices(std::vector<CPeripheral *> &subDevices) const
{
  for (auto m_subDevice : m_subDevices)
    subDevices.push_back(m_subDevice);
}

bool CPeripheral::IsMultiFunctional() const
{
  return m_subDevices.size() > 0;
}

std::vector<CSetting *> CPeripheral::GetSettings() const
{
  std::vector<PeripheralDeviceSetting> tmpSettings;
  for (const auto & m_setting : m_settings)
    tmpSettings.push_back(m_setting.second);
  sort(tmpSettings.begin(), tmpSettings.end(), SortBySettingsOrder());

  std::vector<CSetting *> settings;
  for (std::vector<PeripheralDeviceSetting>::const_iterator it = tmpSettings.begin(); it != tmpSettings.end(); ++it)
    settings.push_back(it->m_setting);
  return settings;
}

void CPeripheral::AddSetting(const std::string &strKey, const CSetting *setting, int order)
{
  if (!setting)
  {
    CLog::Log(LOGERROR, "%s - invalid setting", __FUNCTION__);
    return;
  }

  if (!HasSetting(strKey))
  {
    PeripheralDeviceSetting deviceSetting = { NULL, order };
    switch(setting->GetType())
    {
    case SettingTypeBool:
      {
        const CSettingBool *mappedSetting = (const CSettingBool *) setting;
        CSettingBool *boolSetting = new CSettingBool(strKey, *mappedSetting);
        if (boolSetting)
        {
          boolSetting->SetVisible(mappedSetting->IsVisible());
          deviceSetting.m_setting = boolSetting;
        }
      }
      break;
    case SettingTypeInteger:
      {
        const CSettingInt *mappedSetting = (const CSettingInt *) setting;
        CSettingInt *intSetting = new CSettingInt(strKey, *mappedSetting);
        if (intSetting)
        {
          intSetting->SetVisible(mappedSetting->IsVisible());
          deviceSetting.m_setting = intSetting;
        }
      }
      break;
    case SettingTypeNumber:
      {
        const CSettingNumber *mappedSetting = (const CSettingNumber *) setting;
        CSettingNumber *floatSetting = new CSettingNumber(strKey, *mappedSetting);
        if (floatSetting)
        {
          floatSetting->SetVisible(mappedSetting->IsVisible());
          deviceSetting.m_setting = floatSetting;
        }
      }
      break;
    case SettingTypeString:
      {
        const CSettingString *mappedSetting = (const CSettingString *) setting;
        CSettingString *stringSetting = new CSettingString(strKey, *mappedSetting);
        if (stringSetting)
        {
          stringSetting->SetVisible(mappedSetting->IsVisible());
          deviceSetting.m_setting = stringSetting;
        }
      }
      break;
    default:
      //TODO add more types if needed
      break;
    }

    if (deviceSetting.m_setting != NULL)
      m_settings.insert(make_pair(strKey, deviceSetting));
  }
}

bool CPeripheral::HasSetting(const std::string &strKey) const
{
  std::map<std::string, PeripheralDeviceSetting>:: const_iterator it = m_settings.find(strKey);
  return it != m_settings.end();
}

bool CPeripheral::HasSettings() const
{
  return !m_settings.empty();
}

bool CPeripheral::HasConfigurableSettings() const
{
  bool bReturn(false);
  std::map<std::string, PeripheralDeviceSetting>::const_iterator it = m_settings.begin();
  while (it != m_settings.end() && !bReturn)
  {
    if ((*it).second.m_setting->IsVisible())
    {
      bReturn = true;
      break;
    }

    ++it;
  }

  return bReturn;
}

bool CPeripheral::GetSettingBool(const std::string &strKey) const
{
  std::map<std::string, PeripheralDeviceSetting>::const_iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeBool)
  {
    CSettingBool *boolSetting = (CSettingBool *) (*it).second.m_setting;
    if (boolSetting)
      return boolSetting->GetValue();
  }

  return false;
}

int CPeripheral::GetSettingInt(const std::string &strKey) const
{
  std::map<std::string, PeripheralDeviceSetting>::const_iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeInteger)
  {
    CSettingInt *intSetting = (CSettingInt *) (*it).second.m_setting;
    if (intSetting)
      return intSetting->GetValue();
  }

  return 0;
}

float CPeripheral::GetSettingFloat(const std::string &strKey) const
{
  std::map<std::string, PeripheralDeviceSetting>::const_iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeNumber)
  {
    CSettingNumber *floatSetting = (CSettingNumber *) (*it).second.m_setting;
    if (floatSetting)
      return (float)floatSetting->GetValue();
  }

  return 0;
}

const std::string CPeripheral::GetSettingString(const std::string &strKey) const
{
  std::map<std::string, PeripheralDeviceSetting>::const_iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeString)
  {
    CSettingString *stringSetting = (CSettingString *) (*it).second.m_setting;
    if (stringSetting)
      return stringSetting->GetValue();
  }

  return "";
}

bool CPeripheral::SetSetting(const std::string &strKey, bool bValue)
{
  bool bChanged(false);
  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeBool)
  {
    CSettingBool *boolSetting = (CSettingBool *) (*it).second.m_setting;
    if (boolSetting)
    {
      bChanged = boolSetting->GetValue() != bValue;
      boolSetting->SetValue(bValue);
      if (bChanged && m_bInitialised)
        m_changedSettings.insert(strKey);
    }
  }
  return bChanged;
}

bool CPeripheral::SetSetting(const std::string &strKey, int iValue)
{
  bool bChanged(false);
  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeInteger)
  {
    CSettingInt *intSetting = (CSettingInt *) (*it).second.m_setting;
    if (intSetting)
    {
      bChanged = intSetting->GetValue() != iValue;
      intSetting->SetValue(iValue);
      if (bChanged && m_bInitialised)
        m_changedSettings.insert(strKey);
    }
  }
  return bChanged;
}

bool CPeripheral::SetSetting(const std::string &strKey, float fValue)
{
  bool bChanged(false);
  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.find(strKey);
  if (it != m_settings.end() && (*it).second.m_setting->GetType() == SettingTypeNumber)
  {
    CSettingNumber *floatSetting = (CSettingNumber *) (*it).second.m_setting;
    if (floatSetting)
    {
      bChanged = floatSetting->GetValue() != fValue;
      floatSetting->SetValue(fValue);
      if (bChanged && m_bInitialised)
        m_changedSettings.insert(strKey);
    }
  }
  return bChanged;
}

void CPeripheral::SetSettingVisible(const std::string &strKey, bool bSetTo)
{
  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.find(strKey);
  if (it != m_settings.end())
    (*it).second.m_setting->SetVisible(bSetTo);
}

bool CPeripheral::IsSettingVisible(const std::string &strKey) const
{
  std::map<std::string, PeripheralDeviceSetting>::const_iterator it = m_settings.find(strKey);
  if (it != m_settings.end())
    return (*it).second.m_setting->IsVisible();
  return false;
}

bool CPeripheral::SetSetting(const std::string &strKey, const std::string &strValue)
{
  bool bChanged(false);
  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.find(strKey);
  if (it != m_settings.end())
  {
    if ((*it).second.m_setting->GetType() == SettingTypeString)
    {
      CSettingString *stringSetting = (CSettingString *) (*it).second.m_setting;
      if (stringSetting)
      {
        bChanged = !StringUtils::EqualsNoCase(stringSetting->GetValue(), strValue);
        stringSetting->SetValue(strValue);
        if (bChanged && m_bInitialised)
          m_changedSettings.insert(strKey);
      }
    }
    else if ((*it).second.m_setting->GetType() == SettingTypeInteger)
      bChanged = SetSetting(strKey, (int) (strValue.empty() ? 0 : atoi(strValue.c_str())));
    else if ((*it).second.m_setting->GetType() == SettingTypeNumber)
      bChanged = SetSetting(strKey, (float) (strValue.empty() ? 0 : atof(strValue.c_str())));
    else if ((*it).second.m_setting->GetType() == SettingTypeBool)
      bChanged = SetSetting(strKey, strValue == "1");
  }
  return bChanged;
}

void CPeripheral::PersistSettings(bool bExiting /* = false */)
{
  CXBMCTinyXML doc;
  TiXmlElement node("settings");
  doc.InsertEndChild(node);
  for (std::map<std::string, PeripheralDeviceSetting>::const_iterator itr = m_settings.begin(); itr != m_settings.end(); ++itr)
  {
    TiXmlElement nodeSetting("setting");
    nodeSetting.SetAttribute("id", itr->first.c_str());
    std::string strValue;
    switch ((*itr).second.m_setting->GetType())
    {
    case SettingTypeString:
      {
        CSettingString *stringSetting = (CSettingString *) (*itr).second.m_setting;
        if (stringSetting)
          strValue = stringSetting->GetValue();
      }
      break;
    case SettingTypeInteger:
      {
        CSettingInt *intSetting = (CSettingInt *) (*itr).second.m_setting;
        if (intSetting)
          strValue = StringUtils::Format("%d", intSetting->GetValue());
      }
      break;
    case SettingTypeNumber:
      {
        CSettingNumber *floatSetting = (CSettingNumber *) (*itr).second.m_setting;
        if (floatSetting)
          strValue = StringUtils::Format("%.2f", floatSetting->GetValue());
      }
      break;
    case SettingTypeBool:
      {
        CSettingBool *boolSetting = (CSettingBool *) (*itr).second.m_setting;
        if (boolSetting)
          strValue = StringUtils::Format("%d", boolSetting->GetValue() ? 1:0);
      }
      break;
    default:
      break;
    }
    nodeSetting.SetAttribute("value", strValue.c_str());
    doc.RootElement()->InsertEndChild(nodeSetting);
  }

  doc.SaveFile(m_strSettingsFile);

  if (!bExiting)
  {
    for (const auto & m_changedSetting : m_changedSettings)
      OnSettingChanged(m_changedSetting);
  }
  m_changedSettings.clear();
}

void CPeripheral::LoadPersistedSettings()
{
  CXBMCTinyXML doc;
  if (doc.LoadFile(m_strSettingsFile))
  {
    const TiXmlElement *setting = doc.RootElement()->FirstChildElement("setting");
    while (setting)
    {
      std::string    strId = XMLUtils::GetAttribute(setting, "id");
      std::string strValue = XMLUtils::GetAttribute(setting, "value");
      SetSetting(strId, strValue);

      setting = setting->NextSiblingElement("setting");
    }
  }
}

void CPeripheral::ResetDefaultSettings()
{
  ClearSettings();
  g_peripherals.GetSettingsFromMapping(*this);

  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.begin();
  while (it != m_settings.end())
  {
    m_changedSettings.insert((*it).first);
    ++it;
  }

  PersistSettings();
}

void CPeripheral::ClearSettings()
{
  std::map<std::string, PeripheralDeviceSetting>::iterator it = m_settings.begin();
  while (it != m_settings.end())
  {
    delete (*it).second.m_setting;
    ++it;
  }
  m_settings.clear();
}

bool CPeripheral::operator ==(const PeripheralScanResult& right) const
{
  return StringUtils::EqualsNoCase(m_strLocation, right.m_strLocation);
}

bool CPeripheral::operator !=(const PeripheralScanResult& right) const
{
  return !(*this == right);
}
