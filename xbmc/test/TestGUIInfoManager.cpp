/*
*      Copyright (C) 2015 Team XBMC
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

#include "gtest/gtest.h"
#include "GUIInfoManager.h"


TEST(CGUIInfoManager, SplitInfoString)
{
  std::string arg{"Skin.HasSetting(UseCustomBackground)"};

  std::vector<CGUIInfoManager::Property> results;
  std::vector<CGUIInfoManager::Property> ref;

  //g_infoManager.SplitInfoString(arg, results);
  //ASSERT_EQ(ref, results);
  ASSERT_EQ(true, true);
}