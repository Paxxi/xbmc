/*
 *  Copyright (C) 2013-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#include "SettingPath.h"

#include "settings/lib/SettingsManager.h"
#include "utils/StringUtils.h"
#include "utils/XBMCTinyXML.h"
#include "utils/XMLUtils.h"
#include "utils/log.h"

#define XML_ELM_DEFAULT "default"
#define XML_ELM_CONSTRAINTS "constraints"

CSettingPath::CSettingPath(const std::string& id, CSettingsManager* settingsManager /* = nullptr */)
  : CSettingString(id, settingsManager)
{
}

CSettingPath::CSettingPath(const std::string& id,
                           int label,
                           const std::string& value,
                           CSettingsManager* settingsManager /* = nullptr */)
  : CSettingString(id, label, value, settingsManager)
{
}

CSettingPath::CSettingPath(const std::string& id, const CSettingPath& setting)
  : CSettingString(id, setting)
{
  copy(setting);
}

SettingPtr CSettingPath::Clone(const std::string& id) const
{
  return std::make_shared<CSettingPath>(id, *this);
}

bool CSettingPath::Deserialize(const TiXmlNode* node, bool update /* = false */)
{
  CExclusiveLock lock(m_critical);

  if (!CSettingString::Deserialize(node, update))
    return false;

  if (m_control != nullptr &&
      (m_control->GetType() != "button" ||
       (m_control->GetFormat() != "path" && m_control->GetFormat() != "file")))
  {
    CLog::Log(LOGERROR, "CSettingPath: invalid <control> of \"%s\"", m_id.c_str());
    return false;
  }

  auto constraints = node->FirstChild(XML_ELM_CONSTRAINTS);
  if (constraints != nullptr)
  {
    // get writable
    XMLUtils::GetBoolean(constraints, "writable", m_writable);

    // get sources
    auto sources = constraints->FirstChild("sources");
    if (sources != nullptr)
    {
      m_sources.clear();
      auto source = sources->FirstChild("source");
      while (source != nullptr)
      {
        std::string strSource = source->FirstChild()->ValueStr();
        if (!strSource.empty())
          m_sources.push_back(strSource);

        source = source->NextSibling("source");
      }
    }

    // get masking
    auto masking = constraints->FirstChild("masking");
    if (masking != nullptr)
      m_masking = masking->FirstChild()->ValueStr();
  }

  return true;
}

bool CSettingPath::SetValue(const std::string& value)
{
  // for backwards compatibility to Frodo
  if (StringUtils::EqualsNoCase(value, "select folder") ||
      StringUtils::EqualsNoCase(value, "select writable folder"))
    return CSettingString::SetValue("");

  return CSettingString::SetValue(value);
}

void CSettingPath::copy(const CSettingPath& setting)
{
  CSettingString::Copy(setting);

  CExclusiveLock lock(m_critical);
  m_writable = setting.m_writable;
  m_sources = setting.m_sources;
  m_hideExtension = setting.m_hideExtension;
  m_masking = setting.m_masking;
}
