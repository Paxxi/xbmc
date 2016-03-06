/*
*      Copyright (C) 2005-2016 Team Kodi
*      http://kodi.tv
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

#include "InfoTagPicture.h"
#include "pictures/PictureInfoTag.h"
#include "utils/StringUtils.h"
#include "settings/AdvancedSettings.h"

namespace XBMCAddon
{
  namespace xbmc
  {
    InfoTagPicture::InfoTagPicture()
    {
      std::unique_ptr<CPictureInfoTag> infoTag;
    }

    InfoTagPicture::InfoTagPicture(const CPictureInfoTag& tag)
    {
      std::unique_ptr<CPictureInfoTag> infoTag;
      *infoTag = tag;
    }

    InfoTagPicture::~InfoTagPicture()
    {
    }

    String InfoTagPicture::getInfo(const String& info)
    {
      return infoTag->GetInfo(CPictureInfoTag::TranslateString(info));
    }
  }
}
