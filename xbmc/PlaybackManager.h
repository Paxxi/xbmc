﻿#pragma once
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
#include "ApplicationPlayer.h"
#include "video/Bookmark.h"
#include "video/PlayerController.h"
#include "threads/Thread.h"
#include "PlayListPlayer.h"

#define VOLUME_MINIMUM 0.0f        // -60dB
#define VOLUME_MAXIMUM 1.0f        // 0dB
#define VOLUME_DYNAMIC_RANGE 90.0f // 60dB
#define VOLUME_CONTROL_STEPS 90    // 90 steps
#include <xbmc/music/karaoke/karaokelyricsmanager.h>
#include "utils/Stopwatch.h"

namespace PLAYLIST{
  class CPlayList;
}

// replay gain settings struct for quick access by the player multiple
// times per second (saves doing settings lookup)
struct ReplayGainSettings
{
  int iPreAmp;
  int iNoGainPreAmp;
  int iType;
  bool bAvoidClipping;
};

class CBackgroundPlayer : public CThread
{
public:
  CBackgroundPlayer(const CFileItem &item, int iPlayList);
  virtual ~CBackgroundPlayer();
  virtual void Process();
protected:
  CFileItem *m_item;
  int       m_iPlayList;
};

class CFileItem;

class CPlaybackManager : public IPlayerCallback
{
private:
  CPlaybackManager();
  CPlaybackManager(const CPlaybackManager&) = delete;
  CPlaybackManager const& operator=(CPlaybackManager const&) = delete;
  virtual ~CPlaybackManager() { };

public:
  /*! \brief static method to get the current instance of the class. Creates a new instance the first time it's called.
  */
  static CPlaybackManager& Get();
  
  virtual void OnPlayBackEnded();
  virtual void OnPlayBackStarted();
  virtual void OnPlayBackPaused();
  virtual void OnPlayBackResumed();
  virtual void OnPlayBackStopped();
  virtual void OnQueueNextItem();
  virtual void OnPlayBackSeek(int iTime, int seekOffset);
  virtual void OnPlayBackSeekChapter(int iChapter);
  virtual void OnPlayBackSpeedChanged(int iSpeed);

  const std::string& CurrentFile();
  CFileItem& CurrentFileItem();
  CFileItem& CurrentUnstackedItem();

  PLAYERCOREID GetCurrentPlayer();

  void LoadVideoSettings(const CFileItem& item);

  bool PlayMedia(const CFileItem& item, int iPlaylist = PLAYLIST_MUSIC);
  bool PlayMediaSync(const CFileItem& item, int iPlaylist = PLAYLIST_MUSIC);
  bool ProcessAndStartPlaylist(const std::string& strPlayList, PLAYLIST::CPlayList& playlist, int iPlaylist, int track = 0);
  PlayBackRet PlayFile(const CFileItem& item, bool bRestart = false);
  void SaveFileState(bool bForeground = false);
  void UpdateFileState();

  void StopPlaying();
  void Restart(bool bSamePosition = true);
  void DelayedPlayerRestart();
  void CheckDelayedPlayerRestart();
  void CheckPlayingProgress();
  bool IsPlayingFullScreenVideo() const;
  bool IsStartingPlayback() const { return m_bPlaybackStarting; }

  float GetVolume(bool percentage = true) const;
  void SetVolume(float iValue, bool isPercentage = true);
  bool IsMuted() const;
  bool IsMutedInternal() const { return m_muted; }
  void ToggleMute(void);
  void SetMute(bool mute);
  void ShowVolumeBar(const CAction *action = nullptr);
  int GetSubtitleDelay() const;
  int GetAudioDelay() const;

  /*!
  \brief Returns the total time in fractional seconds of the currently playing media

  Beware that this method returns fractional seconds whereas IPlayer::GetTotalTime() returns milliseconds.
  */
  double GetTotalTime() const;
  /*!
  \brief Returns the current time in fractional seconds of the currently playing media

  Beware that this method returns fractional seconds whereas IPlayer::GetTime() returns milliseconds.
  */
  double GetTime() const;
  float GetPercentage() const;

  // Get the percentage of data currently cached/buffered (aq/vq + FileCache) from the input stream if applicable.
  float GetCachePercentage() const;

  void SeekPercentage(float percent);
  void SeekTime(double dTime = 0.0);

  bool m_bPlaybackStarting;
  typedef enum
  {
    PLAY_STATE_NONE = 0,
    PLAY_STATE_STARTING,
    PLAY_STATE_PLAYING,
    PLAY_STATE_STOPPED,
    PLAY_STATE_ENDED,
  } PlayState;
  PlayState m_ePlayState;

  ReplayGainSettings& GetReplayGainSettings() { return m_replayGainSettings; }
private:
  CFileItemPtr m_itemCurrentFile;
  CFileItemList* m_currentStack;
  CFileItemPtr m_stackFileItemToUpdate;

  CBookmark& m_progressTrackingVideoResumeBookmark;
  CFileItemPtr m_progressTrackingItem;
  bool m_progressTrackingPlayCountUpdate;

  int m_currentStackPosition;
  int m_nextPlaylistItem;

  bool m_muted;
  float m_volumeLevel;

  bool m_bStop;

  CApplicationPlayer* m_pPlayer;
  CPlayerController *m_playerController;
  ReplayGainSettings m_replayGainSettings;

  CCriticalSection m_playStateMutex;

  CKaraokeLyricsManager* m_pKaraokeMgr;

  PLAYERCOREID m_eForcedNextPlayer;
  std::string m_strPlayListFile;

  CStopWatch m_restartPlayerTimer;

  void Mute();
  void UnMute();

  void SetHardwareVolume(float hardwareVolume);

  void VolumeChanged() const;

  PlayBackRet PlayStack(const CFileItem& item, bool bRestart);
};
