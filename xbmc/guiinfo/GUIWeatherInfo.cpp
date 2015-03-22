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

#include "GUIWeatherInfo.h"
#include "GUIInfoLabels.h"
#include "utils/Weather.h"
#include "utils/StringUtils.h"
#include "LangInfo.h"
#include "utils/URIUtils.h"
#include "settings/Settings.h"

namespace GUIINFO
{

int CGUIWeatherInfo::LabelMask()
{
  return WEATHER_MASK;
}

std::string CGUIWeatherInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback)
{
  std::string strLabel{*fallback};

  switch (info)
  {
  case WEATHER_CONDITIONS:
    strLabel = g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_COND);
    StringUtils::Trim(strLabel);
    break;
  case WEATHER_TEMPERATURE:
    strLabel = StringUtils::Format("%s%s",
                                   g_weatherManager.GetInfo(WEATHER_LABEL_CURRENT_TEMP).c_str(),
                                   g_langInfo.GetTemperatureUnitString().c_str());
    break;
  case WEATHER_LOCATION:
    strLabel = g_weatherManager.GetInfo(WEATHER_LABEL_LOCATION);
    break;
  case WEATHER_FANART_CODE:
    strLabel = URIUtils::GetFileName(g_weatherManager.GetInfo(WEATHER_IMAGE_CURRENT_ICON));
    URIUtils::RemoveExtension(strLabel);
    break;
  case WEATHER_PLUGIN:
    strLabel = CSettings::Get().GetString("weather.addon");
    break;
  default:
    break;
  }

  return strLabel;
}
} //end namespace GUIINFO