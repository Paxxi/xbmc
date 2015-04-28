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

#include "GUIPvrInfo.h"
#include "GUIInfoManager.h"
#include "GUIInfoLabels.h"
#include "pvr/PVRManager.h"

namespace GUIINFO
{
  int CGUIPvrInfo::LabelMask()
  {
    return PVR_MASK;
  }

  std::string CGUIPvrInfo::GetLabel(CFileItem* currentFile, int info, int contextWindow, std::string *fallback)
  {
    std::string strLabel = fallback == nullptr ? "" : *fallback;

    switch (info)
    {
    case PVR_NEXT_RECORDING_CHANNEL:
    case PVR_NEXT_RECORDING_CHAN_ICO:
    case PVR_NEXT_RECORDING_DATETIME:
    case PVR_NEXT_RECORDING_TITLE:
    case PVR_NOW_RECORDING_CHANNEL:
    case PVR_NOW_RECORDING_CHAN_ICO:
    case PVR_NOW_RECORDING_DATETIME:
    case PVR_NOW_RECORDING_TITLE:
    case PVR_BACKEND_NAME:
    case PVR_BACKEND_VERSION:
    case PVR_BACKEND_HOST:
    case PVR_BACKEND_DISKSPACE:
    case PVR_BACKEND_CHANNELS:
    case PVR_BACKEND_TIMERS:
    case PVR_BACKEND_RECORDINGS:
    case PVR_BACKEND_DELETED_RECORDINGS:
    case PVR_BACKEND_NUMBER:
    case PVR_TOTAL_DISKSPACE:
    case PVR_NEXT_TIMER:
    case PVR_PLAYING_DURATION:
    case PVR_PLAYING_TIME:
    case PVR_PLAYING_PROGRESS:
    case PVR_ACTUAL_STREAM_CLIENT:
    case PVR_ACTUAL_STREAM_DEVICE:
    case PVR_ACTUAL_STREAM_STATUS:
    case PVR_ACTUAL_STREAM_SIG:
    case PVR_ACTUAL_STREAM_SNR:
    case PVR_ACTUAL_STREAM_SIG_PROGR:
    case PVR_ACTUAL_STREAM_SNR_PROGR:
    case PVR_ACTUAL_STREAM_BER:
    case PVR_ACTUAL_STREAM_UNC:
    case PVR_ACTUAL_STREAM_VIDEO_BR:
    case PVR_ACTUAL_STREAM_AUDIO_BR:
    case PVR_ACTUAL_STREAM_DOLBY_BR:
    case PVR_ACTUAL_STREAM_CRYPTION:
    case PVR_ACTUAL_STREAM_SERVICE:
    case PVR_ACTUAL_STREAM_MUX:
    case PVR_ACTUAL_STREAM_PROVIDER:
      g_PVRManager.TranslateCharInfo(info, strLabel);
      break;
    default:
      break;
    }

    return strLabel;
  }

  bool CGUIPvrInfo::GetInt(int& value, int info, int contextWindow, const CGUIListItem* item)
  {
    switch (info)
    {
    case PVR_PLAYING_PROGRESS:
    case PVR_ACTUAL_STREAM_SIG_PROGR:
    case PVR_ACTUAL_STREAM_SNR_PROGR:
    case PVR_BACKEND_DISKSPACE_PROGR:
      value = g_PVRManager.TranslateIntInfo(info);
      return true;
    default:
      break;
    }

    return false;
  }

}