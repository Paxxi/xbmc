#include "PlaybackManager.h"
#include "FileItem.h"
#include <xbmc/PlayListPlayer.h>
#include <xbmc/settings/AdvancedSettings.h>
#include "utils/StreamDetails.h"
#include <xbmc/PartyModeManager.h>
#include "guilib/GUIWindowManager.h"
#include <xbmc/interfaces/python/XBPython.h>
#include <xbmc/GUIInfoManager.h>
#include <xbmc/interfaces/AnnouncementManager.h>
#include <xbmc/interfaces/json-rpc/JSONUtils.h>
#include "filesystem/PluginDirectory.h"

CPlaybackManager& CPlaybackManager::Get()
{
  static CPlaybackManager playbackManager;
  return playbackManager;
}

const std::string& CPlaybackManager::CurrentFile()
{
  return m_itemCurrentFile->GetPath();
}

CFileItem& CPlaybackManager::CurrentFileItem()
{
  return *m_itemCurrentFile;
}

CFileItem& CPlaybackManager::CurrentUnstackedItem()
{
  if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
    return *(*m_currentStack)[m_currentStackPosition];
  else
    return *m_itemCurrentFile;
}

bool CPlaybackManager::PlayMedia(const CFileItem& item, int iPlaylist)
{
  //If item is a plugin, expand out now and run ourselves again
  if (item.IsPlugin())
  {
    CFileItem item_new(item);
    if (XFILE::CPluginDirectory::GetPluginResult(item.GetPath(), item_new))
      return PlayMedia(item_new, iPlaylist);
    return false;
  }
  if (item.IsSmartPlayList())
  {
    CFileItemList items;
    CUtil::GetRecursiveListing(item.GetPath(), items, "", DIR_FLAG_NO_FILE_DIRS);
    if (items.Size())
    {
      CSmartPlaylist smartpl;
      //get name and type of smartplaylist, this will always succeed as GetDirectory also did this.
      smartpl.OpenAndReadName(item.GetURL());
      CPlayList playlist;
      playlist.Add(items);
      return ProcessAndStartPlaylist(smartpl.GetName(), playlist, (smartpl.GetType() == "songs" || smartpl.GetType() == "albums") ? PLAYLIST_MUSIC : PLAYLIST_VIDEO);
    }
  }
  else if (item.IsPlayList() || item.IsInternetStream())
  {
    CGUIDialogCache* dlgCache = new CGUIDialogCache(5000, g_localizeStrings.Get(10214), item.GetLabel());

    //is or could be a playlist
    unique_ptr<CPlayList> pPlayList(CPlayListFactory::Create(item));
    bool gotPlayList = (pPlayList.get() && pPlayList->Load(item.GetPath()));

    if (dlgCache)
    {
      dlgCache->Close();
      if (dlgCache->IsCanceled())
        return true;
    }

    if (gotPlayList)
    {

      if (iPlaylist != PLAYLIST_NONE)
      {
        int track = 0;
        if (item.HasProperty("playlist_starting_track"))
          track = (int)item.GetProperty("playlist_starting_track").asInteger();
        return ProcessAndStartPlaylist(item.GetPath(), *pPlayList, iPlaylist, track);
      }
      else
      {
        CLog::Log(LOGWARNING, "CPlaybackManager::PlayMedia called to play a playlist %s but no idea which playlist to use, playing first item", item.GetPath().c_str());
        if (pPlayList->size())
          return PlayFile(*(*pPlayList)[0], false) == PLAYBACK_OK;
      }
    }
  }
  else if (item.IsPVR())
  {
    return g_PVRManager.PlayMedia(item);
  }

  //nothing special just play
  return PlayFile(item, false) == PLAYBACK_OK;
}

// PlayStack()
// For playing a multi-file video.  Particularly inefficient
// on startup, as we are required to calculate the length
// of each video, so we open + close each one in turn.
// A faster calculation of video time would improve this
// substantially.
// return value: same with PlayFile()
PlayBackRet CPlaybackManager::PlayStack(const CFileItem& item, bool bRestart)
{
  if (!item.IsStack())
    return PLAYBACK_FAIL;

  CVideoDatabase dbs;

  // case 1: stacked ISOs
  if (CFileItem(CStackDirectory::GetFirstStackedFile(item.GetPath()), false).IsDiscImage())
  {
    CStackDirectory dir;
    CFileItemList movieList;
    if (!dir.GetDirectory(item.GetURL(), movieList) || movieList.IsEmpty())
      return PLAYBACK_FAIL;

    // first assume values passed to the stack
    int selectedFile = item.m_lStartPartNumber;
    int startoffset = item.m_lStartOffset;

    // check if we instructed the stack to resume from default
    if (startoffset == STARTOFFSET_RESUME) // selected file is not specified, pick the 'last' resume point
    {
      if (dbs.Open())
      {
        CBookmark bookmark;
        std::string path = item.GetPath();
        if (item.HasProperty("original_listitem_url") && URIUtils::IsPlugin(item.GetProperty("original_listitem_url").asString()))
          path = item.GetProperty("original_listitem_url").asString();
        if (dbs.GetResumeBookMark(path, bookmark))
        {
          startoffset = (int)(bookmark.timeInSeconds * 75);
          selectedFile = bookmark.partNumber;
        }
        dbs.Close();
      }
      else
        CLog::LogF(LOGERROR, "Cannot open VideoDatabase");
    }

    // make sure that the selected part is within the boundaries
    if (selectedFile <= 0)
    {
      CLog::LogF(LOGWARNING, "Selected part %d out of range, playing part 1", selectedFile);
      selectedFile = 1;
    }
    else if (selectedFile > movieList.Size())
    {
      CLog::LogF(LOGWARNING, "Selected part %d out of range, playing part %d", selectedFile, movieList.Size());
      selectedFile = movieList.Size();
    }

    // set startoffset in movieitem, track stack item for updating purposes, and finally play disc part
    movieList[selectedFile - 1]->m_lStartOffset = startoffset > 0 ? STARTOFFSET_RESUME : 0;
    movieList[selectedFile - 1]->SetProperty("stackFileItemToUpdate", true);
    *m_stackFileItemToUpdate = item;
    return PlayFile(*(movieList[selectedFile - 1]));
  }
  // case 2: all other stacks
  else
  {
    LoadVideoSettings(item);

    // see if we have the info in the database
    // TODO: If user changes the time speed (FPS via framerate conversion stuff)
    //       then these times will be wrong.
    //       Also, this is really just a hack for the slow load up times we have
    //       A much better solution is a fast reader of FPS and fileLength
    //       that we can use on a file to get it's time.
    vector<int> times;
    bool haveTimes(false);
    CVideoDatabase dbs;
    if (dbs.Open())
    {
      haveTimes = dbs.GetStackTimes(item.GetPath(), times);
      dbs.Close();
    }


    // calculate the total time of the stack
    CStackDirectory dir;
    if (!dir.GetDirectory(item.GetURL(), *m_currentStack) || m_currentStack->IsEmpty())
      return PLAYBACK_FAIL;
    long totalTime = 0;
    for (int i = 0; i < m_currentStack->Size(); i++)
    {
      if (haveTimes)
        (*m_currentStack)[i]->m_lEndOffset = times[i];
      else
      {
        int duration;
        if (!CDVDFileInfo::GetFileDuration((*m_currentStack)[i]->GetPath(), duration))
        {
          m_currentStack->Clear();
          return PLAYBACK_FAIL;
        }
        totalTime += duration / 1000;
        (*m_currentStack)[i]->m_lEndOffset = totalTime;
        times.push_back(totalTime);
      }
    }

    double seconds = item.m_lStartOffset / 75.0;

    if (!haveTimes || item.m_lStartOffset == STARTOFFSET_RESUME)
    {  // have our times now, so update the dB
      if (dbs.Open())
      {
        if (!haveTimes && !times.empty())
          dbs.SetStackTimes(item.GetPath(), times);

        if (item.m_lStartOffset == STARTOFFSET_RESUME)
        {
          // can only resume seek here, not dvdstate
          CBookmark bookmark;
          std::string path = item.GetPath();
          if (item.HasProperty("original_listitem_url") && URIUtils::IsPlugin(item.GetProperty("original_listitem_url").asString()))
            path = item.GetProperty("original_listitem_url").asString();
          if (dbs.GetResumeBookMark(path, bookmark))
            seconds = bookmark.timeInSeconds;
          else
            seconds = 0.0f;
        }
        dbs.Close();
      }
    }

    *m_itemCurrentFile = item;
    m_currentStackPosition = 0;
    m_pPlayer->ResetPlayer(); // must be reset on initial play otherwise last player will be used

    if (seconds > 0)
    {
      // work out where to seek to
      for (int i = 0; i < m_currentStack->Size(); i++)
      {
        if (seconds < (*m_currentStack)[i]->m_lEndOffset)
        {
          CFileItem item(*(*m_currentStack)[i]);
          long start = (i > 0) ? (*m_currentStack)[i - 1]->m_lEndOffset : 0;
          item.m_lStartOffset = (long)(seconds - start) * 75;
          m_currentStackPosition = i;
          return PlayFile(item, true);
        }
      }
    }

    return PlayFile(*(*m_currentStack)[0], true);
  }
  return PLAYBACK_FAIL;
}

PlayBackRet CPlaybackManager::PlayFile(const CFileItem& item, bool bRestart)
{
  // Ensure the MIME type has been retrieved for http:// and shout:// streams
  if (item.GetMimeType().empty())
    const_cast<CFileItem&>(item).FillInMimeType();

  if (!bRestart)
  {
    SaveFileState(true);

    // Switch to default options
    CMediaSettings::Get().GetCurrentVideoSettings() = CMediaSettings::Get().GetDefaultVideoSettings();
    // see if we have saved options in the database

    m_pPlayer->SetPlaySpeed(1, g_application.m_muted);
    m_pPlayer->m_iPlaySpeed = 1;     // Reset both CApp's & Player's speed else we'll get confused

    *m_itemCurrentFile = item;
    m_nextPlaylistItem = -1;
    m_currentStackPosition = 0;
    m_currentStack->Clear();

    if (item.IsVideo())
      CUtil::ClearSubtitles();
  }

  if (item.IsDiscStub())
  {
#ifdef HAS_DVD_DRIVE
    // Display the Play Eject dialog if there is any optical disc drive
    if (g_mediaManager.HasOpticalDrive())
    {
      if (CGUIDialogPlayEject::ShowAndGetInput(item))
        // PlayDiscAskResume takes path to disc. No parameter means default DVD drive.
        // Can't do better as CGUIDialogPlayEject calls CMediaManager::IsDiscInDrive, which assumes default DVD drive anyway
        return MEDIA_DETECT::CAutorun::PlayDiscAskResume() ? PLAYBACK_OK : PLAYBACK_FAIL;
    }
    else
#endif
      CGUIDialogOK::ShowAndGetInput(435, 0, 436, 0);

    return PLAYBACK_OK;
  }

  if (item.IsPlayList())
    return PLAYBACK_FAIL;

  if (item.IsPlugin())
  { // we modify the item so that it becomes a real URL
    CFileItem item_new(item);
    if (XFILE::CPluginDirectory::GetPluginResult(item.GetPath(), item_new))
      return PlayFile(item_new, false);
    return PLAYBACK_FAIL;
  }

  // a disc image might be Blu-Ray disc
  if (item.IsBDFile() || item.IsDiscImage())
  {
    //check if we must show the simplified bd menu
    if (!CGUIDialogSimpleMenu::ShowPlaySelection(const_cast<CFileItem&>(item)))
      return PLAYBACK_CANCELED;
  }

#ifdef HAS_UPNP
  if (URIUtils::IsUPnP(item.GetPath()))
  {
    CFileItem item_new(item);
    if (XFILE::CUPnPDirectory::GetResource(item.GetURL(), item_new))
      return PlayFile(item_new, false);
    return PLAYBACK_FAIL;
  }
#endif

  // if we have a stacked set of files, we need to setup our stack routines for
  // "seamless" seeking and total time of the movie etc.
  // will recall with restart set to true
  if (item.IsStack())
    return PlayStack(item, bRestart);

  CPlayerOptions options;

  if (item.HasProperty("StartPercent"))
  {
    double fallback = 0.0f;
    if (item.GetProperty("StartPercent").isString())
      fallback = (double)atof(item.GetProperty("StartPercent").asString().c_str());
    options.startpercent = item.GetProperty("StartPercent").asDouble(fallback);
  }

  PLAYERCOREID eNewCore = EPC_NONE;
  if (bRestart)
  {
    // have to be set here due to playstack using this for starting the file
    options.starttime = item.m_lStartOffset / 75.0;
    if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0 && m_itemCurrentFile->m_lStartOffset != 0)
      m_itemCurrentFile->m_lStartOffset = STARTOFFSET_RESUME; // to force fullscreen switching

    if (m_eForcedNextPlayer != EPC_NONE)
      eNewCore = m_eForcedNextPlayer;
    else if (m_pPlayer->GetCurrentPlayer() == EPC_NONE)
      eNewCore = CPlayerCoreFactory::Get().GetDefaultPlayer(item);
    else
      eNewCore = m_pPlayer->GetCurrentPlayer();
  }
  else
  {
    options.starttime = item.m_lStartOffset / 75.0;
    LoadVideoSettings(item);

    if (item.IsVideo())
    {
      // open the d/b and retrieve the bookmarks for the current movie
      CVideoDatabase dbs;
      dbs.Open();

      if (item.m_lStartOffset == STARTOFFSET_RESUME)
      {
        options.starttime = 0.0f;
        CBookmark bookmark;
        std::string path = item.GetPath();
        if (item.HasVideoInfoTag() && StringUtils::StartsWith(item.GetVideoInfoTag()->m_strFileNameAndPath, "removable://"))
          path = item.GetVideoInfoTag()->m_strFileNameAndPath;
        else if (item.HasProperty("original_listitem_url") && URIUtils::IsPlugin(item.GetProperty("original_listitem_url").asString()))
          path = item.GetProperty("original_listitem_url").asString();
        if (dbs.GetResumeBookMark(path, bookmark))
        {
          options.starttime = bookmark.timeInSeconds;
          options.state = bookmark.playerState;
        }
        /*
        override with information from the actual item if available.  We do this as the VFS (eg plugins)
        may set the resume point to override whatever XBMC has stored, yet we ignore it until now so that,
        should the playerState be required, it is fetched from the database.
        See the note in CGUIWindowVideoBase::ShowResumeMenu.
        */
        if (item.IsResumePointSet())
          options.starttime = item.GetCurrentResumeTime();
        else if (item.HasVideoInfoTag())
        {
          // No resume point is set, but check if this item is part of a multi-episode file
          const CVideoInfoTag *tag = item.GetVideoInfoTag();

          if (tag->m_iBookmarkId > 0)
          {
            CBookmark bookmark;
            dbs.GetBookMarkForEpisode(*tag, bookmark);
            options.starttime = bookmark.timeInSeconds;
            options.state = bookmark.playerState;
          }
        }
      }
      else if (item.HasVideoInfoTag())
      {
        const CVideoInfoTag *tag = item.GetVideoInfoTag();

        if (tag->m_iBookmarkId > 0)
        {
          CBookmark bookmark;
          dbs.GetBookMarkForEpisode(*tag, bookmark);
          options.starttime = bookmark.timeInSeconds;
          options.state = bookmark.playerState;
        }
      }

      dbs.Close();
    }

    if (m_eForcedNextPlayer != EPC_NONE)
      eNewCore = m_eForcedNextPlayer;
    else
      eNewCore = CPlayerCoreFactory::Get().GetDefaultPlayer(item);
  }

  // this really aught to be inside !bRestart, but since PlayStack
  // uses that to init playback, we have to keep it outside
  int playlist = g_playlistPlayer.GetCurrentPlaylist();
  if (item.IsVideo() && playlist == PLAYLIST_VIDEO && g_playlistPlayer.GetPlaylist(playlist).size() > 1)
  { // playing from a playlist by the looks
    // don't switch to fullscreen if we are not playing the first item...
    options.fullscreen = !g_playlistPlayer.HasPlayedFirstFile() && g_advancedSettings.m_fullScreenOnMovieStart && !CMediaSettings::Get().DoesVideoStartWindowed();
  }
  else if (m_itemCurrentFile->IsStack() && m_currentStack->Size() > 0)
  {
    // TODO - this will fail if user seeks back to first file in stack
    if (m_currentStackPosition == 0 || m_itemCurrentFile->m_lStartOffset == STARTOFFSET_RESUME)
      options.fullscreen = g_advancedSettings.m_fullScreenOnMovieStart && !CMediaSettings::Get().DoesVideoStartWindowed();
    else
      options.fullscreen = false;
    // reset this so we don't think we are resuming on seek
    m_itemCurrentFile->m_lStartOffset = 0;
  }
  else
    options.fullscreen = g_advancedSettings.m_fullScreenOnMovieStart && !CMediaSettings::Get().DoesVideoStartWindowed();

  // reset VideoStartWindowed as it's a temp setting
  CMediaSettings::Get().SetVideoStartWindowed(false);

#ifdef HAS_KARAOKE
  //We have to stop parsing a cdg before mplayer is deallocated
  // WHY do we have to do this????
  if (m_pKaraokeMgr)
    m_pKaraokeMgr->Stop();
#endif

  {
    CSingleLock lock(m_playStateMutex);
    // tell system we are starting a file
    m_bPlaybackStarting = true;

    // for playing a new item, previous playing item's callback may already
    // pushed some delay message into the threadmessage list, they are not
    // expected be processed after or during the new item playback starting.
    // so we clean up previous playing item's playback callback delay messages here.
    int previousMsgsIgnoredByNewPlaying[] = {
      GUI_MSG_PLAYBACK_STARTED,
      GUI_MSG_PLAYBACK_ENDED,
      GUI_MSG_PLAYBACK_STOPPED,
      GUI_MSG_PLAYLIST_CHANGED,
      GUI_MSG_PLAYLISTPLAYER_STOPPED,
      GUI_MSG_PLAYLISTPLAYER_STARTED,
      GUI_MSG_PLAYLISTPLAYER_CHANGED,
      GUI_MSG_QUEUE_NEXT_ITEM,
      0
    };
    int dMsgCount = g_windowManager.RemoveThreadMessageByMessageIds(&previousMsgsIgnoredByNewPlaying[0]);
    if (dMsgCount > 0)
      CLog::LogF(LOGDEBUG, "Ignored %d playback thread messages", dMsgCount);
  }

  // We should restart the player, unless the previous and next tracks are using
  // one of the players that allows gapless playback (paplayer, dvdplayer)
  m_pPlayer->ClosePlayerGapless(eNewCore);

  // now reset play state to starting, since we already stopped the previous playing item if there is.
  // and from now there should be no playback callback from previous playing item be called.
  m_ePlayState = PLAY_STATE_STARTING;

  m_pPlayer->CreatePlayer(eNewCore, *this);

  PlayBackRet iResult;
  if (m_pPlayer->HasPlayer())
  {
    /* When playing video pause any low priority jobs, they will be unpaused  when playback stops.
    * This should speed up player startup for files on internet filesystems (eg. webdav) and
    * increase performance on low powered systems (Atom/ARM).
    */
    if (item.IsVideo())
    {
      CJobManager::GetInstance().PauseJobs();
    }

    // don't hold graphicscontext here since player
    // may wait on another thread, that requires gfx
    CSingleExit ex(g_graphicsContext);

    iResult = m_pPlayer->OpenFile(item, options);
  }
  else
  {
    CLog::Log(LOGERROR, "Error creating player for item %s (File doesn't exist?)", item.GetPath().c_str());
    iResult = PLAYBACK_FAIL;
  }

  if (iResult == PLAYBACK_OK)
  {
    if (m_pPlayer->GetPlaySpeed() != 1)
    {
      int iSpeed = m_pPlayer->GetPlaySpeed();
      m_pPlayer->m_iPlaySpeed = 1;
      m_pPlayer->SetPlaySpeed(iSpeed, g_application.m_muted);
    }

    // if player has volume control, set it.
    if (m_pPlayer->ControlsVolume())
    {
      m_pPlayer->SetVolume(m_volumeLevel);
      m_pPlayer->SetMute(m_muted);
    }

    if (m_pPlayer->IsPlayingAudio())
    {
      if (g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
        g_windowManager.ActivateWindow(WINDOW_VISUALISATION);
    }

#ifdef HAS_VIDEO_PLAYBACK
    else if (m_pPlayer->IsPlayingVideo())
    {
      if (g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION)
        g_windowManager.ActivateWindow(WINDOW_FULLSCREEN_VIDEO);

      // if player didn't manange to switch to fullscreen by itself do it here
      if (options.fullscreen && g_renderManager.IsStarted()
        && g_windowManager.GetActiveWindow() != WINDOW_FULLSCREEN_VIDEO)
        SwitchToFullScreen();
    }
#endif
    else
    {
      if (g_windowManager.GetActiveWindow() == WINDOW_VISUALISATION
        || g_windowManager.GetActiveWindow() == WINDOW_FULLSCREEN_VIDEO)
        g_windowManager.PreviousWindow();

    }

#if !defined(TARGET_POSIX)
    g_audioManager.Enable(false);
#endif

    if (item.HasPVRChannelInfoTag())
      g_playlistPlayer.SetCurrentPlaylist(PLAYLIST_NONE);
  }

  CSingleLock lock(m_playStateMutex);
  m_bPlaybackStarting = false;

  if (iResult == PLAYBACK_OK)
  {
    // play state: none, starting; playing; stopped; ended.
    // last 3 states are set by playback callback, they are all ignored during starting,
    // but we recorded the state, here we can make up the callback for the state.
    CLog::LogF(LOGDEBUG, "OpenFile succeed, play state %d", m_ePlayState);
    switch (m_ePlayState)
    {
    case PLAY_STATE_PLAYING:
      OnPlayBackStarted();
      break;
      // FIXME: it seems no meaning to callback started here if there was an started callback
      //        before this stopped/ended callback we recorded. if we callback started here
      //        first, it will delay send OnPlay announce, but then we callback stopped/ended
      //        which will send OnStop announce at once, so currently, just call stopped/ended.
    case PLAY_STATE_ENDED:
      OnPlayBackEnded();
      break;
    case PLAY_STATE_STOPPED:
      OnPlayBackStopped();
      break;
    case PLAY_STATE_STARTING:
      // neither started nor stopped/ended callback be called, that means the item still
      // not started, we need not make up any callback, just leave this and
      // let the player callback do its work.
      break;
    default:
      break;
    }
  }
  else if (iResult == PLAYBACK_FAIL)
  {
    // we send this if it isn't playlistplayer that is doing this
    int next = g_playlistPlayer.GetNextSong();
    int size = g_playlistPlayer.GetPlaylist(g_playlistPlayer.GetCurrentPlaylist()).size();
    if (next < 0
      || next >= size)
      OnPlayBackStopped();
    m_ePlayState = PLAY_STATE_NONE;
  }

  return iResult;
}


void CPlaybackManager::SaveFileState(bool bForeground /* = false */)
{
  if (!CProfilesManager::Get().GetCurrentProfile().canWriteDatabases())
    return;

  CJob* job = new CSaveFileStateJob(*m_progressTrackingItem,
    *m_stackFileItemToUpdate,
    m_progressTrackingVideoResumeBookmark,
    m_progressTrackingPlayCountUpdate,
    CMediaSettings::Get().GetCurrentVideoSettings());

  if (bForeground)
  {
    // Run job in the foreground to make sure it finishes
    job->DoWork();
    delete job;
  }
  else
    CJobManager::GetInstance().AddJob(job, NULL, CJob::PRIORITY_NORMAL);
}

void CPlaybackManager::UpdateFileState()
{
  // Did the file change?
  if (m_progressTrackingItem->GetPath() != "" && m_progressTrackingItem->GetPath() != CurrentFile())
  {
    // Ignore for PVR channels, PerformChannelSwitch takes care of this.
    // Also ignore video playlists containing multiple items: video settings have already been saved in PlayFile()
    // and we'd overwrite them with settings for the *previous* item.
    // TODO: these "exceptions" should be removed and the whole logic of saving settings be revisited and
    // possibly moved out of CPlaybackManager.  See PRs 5842, 5958, http://trac.kodi.tv/ticket/15704#comment:3
    int playlist = g_playlistPlayer.GetCurrentPlaylist();
    if (!m_progressTrackingItem->IsPVRChannel() && !(playlist == PLAYLIST_VIDEO && g_playlistPlayer.GetPlaylist(playlist).size() > 1))
      SaveFileState();

    // Reset tracking item
    m_progressTrackingItem->Reset();
  }
  else
  {
    if (m_pPlayer->IsPlaying())
    {
      if (m_progressTrackingItem->GetPath() == "")
      {
        // Init some stuff
        *m_progressTrackingItem = CurrentFileItem();
        m_progressTrackingPlayCountUpdate = false;
      }

      if ((m_progressTrackingItem->IsAudio() && g_advancedSettings.m_audioPlayCountMinimumPercent > 0 &&
        GetPercentage() >= g_advancedSettings.m_audioPlayCountMinimumPercent) ||
        (m_progressTrackingItem->IsVideo() && g_advancedSettings.m_videoPlayCountMinimumPercent > 0 &&
        GetPercentage() >= g_advancedSettings.m_videoPlayCountMinimumPercent))
      {
        m_progressTrackingPlayCountUpdate = true;
      }

      // Check whether we're *really* playing video else we may race when getting eg. stream details
      if (m_pPlayer->IsPlayingVideo())
      {
        /* Always update streamdetails, except for DVDs where we only update
        streamdetails if total duration > 15m (Should yield more correct info) */
        if (!(m_progressTrackingItem->IsDiscImage() || m_progressTrackingItem->IsDVDFile()) || m_pPlayer->GetTotalTime() > 15 * 60 * 1000)
        {
          CStreamDetails details;
          // Update with stream details from player, if any
          if (m_pPlayer->GetStreamDetails(details))
            m_progressTrackingItem->GetVideoInfoTag()->m_streamDetails = details;

          if (m_progressTrackingItem->IsStack())
            m_progressTrackingItem->GetVideoInfoTag()->m_streamDetails.SetVideoDuration(0, (int)GetTotalTime()); // Overwrite with CApp's totaltime as it takes into account total stack time
        }

        // Update bookmark for save
        m_progressTrackingVideoResumeBookmark.player = CPlayerCoreFactory::Get().GetPlayerName(m_pPlayer->GetCurrentPlayer());
        m_progressTrackingVideoResumeBookmark.playerState = m_pPlayer->GetPlayerState();
        m_progressTrackingVideoResumeBookmark.thumbNailImage.clear();

        if (g_advancedSettings.m_videoIgnorePercentAtEnd > 0 &&
          GetTotalTime() - GetTime() < 0.01f * g_advancedSettings.m_videoIgnorePercentAtEnd * GetTotalTime())
        {
          // Delete the bookmark
          m_progressTrackingVideoResumeBookmark.timeInSeconds = -1.0f;
        }
        else
          if (GetTime() > g_advancedSettings.m_videoIgnoreSecondsAtStart)
          {
            // Update the bookmark
            m_progressTrackingVideoResumeBookmark.timeInSeconds = GetTime();
            m_progressTrackingVideoResumeBookmark.totalTimeInSeconds = GetTotalTime();
          }
          else
          {
            // Do nothing
            m_progressTrackingVideoResumeBookmark.timeInSeconds = 0.0f;
          }
      }
    }
  }
}

void CPlaybackManager::StopPlaying()
{
  int iWin = g_windowManager.GetActiveWindow();
  if (m_pPlayer->IsPlaying())
  {
#ifdef HAS_KARAOKE
    if (m_pKaraokeMgr)
      m_pKaraokeMgr->Stop();
#endif

    m_pPlayer->CloseFile();

    // turn off visualisation window when stopping
    if ((iWin == WINDOW_VISUALISATION
      || iWin == WINDOW_FULLSCREEN_VIDEO)
      && !m_bStop)
      g_windowManager.PreviousWindow();

    g_partyModeManager.Disable();
  }
}

void CPlaybackManager::OnPlayBackEnded()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG, "play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  m_ePlayState = PLAY_STATE_ENDED;
  if (m_bPlaybackStarting)
    return;

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackEnded();
#endif

  CVariant data(CVariant::VariantTypeObject);
  data["end"] = true;
  ANNOUNCEMENT::CAnnouncementManager::Get().Announce(Player, "xbmc", "OnStop", m_itemCurrentFile, data);

  CGUIMessage msg(GUI_MSG_PLAYBACK_ENDED, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CPlaybackManager::OnPlayBackStarted()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG, "play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  m_ePlayState = PLAY_STATE_PLAYING;
  if (m_bPlaybackStarting)
    return;

#ifdef HAS_PYTHON
  // informs python script currently running playback has started
  // (does nothing if python is not loaded)
  g_pythonParser.OnPlayBackStarted();
#endif

  CGUIMessage msg(GUI_MSG_PLAYBACK_STARTED, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CPlaybackManager::OnQueueNextItem()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG, "play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  if (m_bPlaybackStarting)
    return;
  // informs python script currently running that we are requesting the next track
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnQueueNextItem(); // currently unimplemented
#endif

  CGUIMessage msg(GUI_MSG_QUEUE_NEXT_ITEM, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CPlaybackManager::OnPlayBackStopped()
{
  CSingleLock lock(m_playStateMutex);
  CLog::LogF(LOGDEBUG, "play state was %d, starting %d", m_ePlayState, m_bPlaybackStarting);
  m_ePlayState = PLAY_STATE_STOPPED;
  if (m_bPlaybackStarting)
    return;

  // informs python script currently running playback has ended
  // (does nothing if python is not loaded)
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackStopped();
#endif

  CVariant data(CVariant::VariantTypeObject);
  data["end"] = false;
  ANNOUNCEMENT::CAnnouncementManager::Get().Announce(Player, "xbmc", "OnStop", m_itemCurrentFile, data);

  CGUIMessage msg(GUI_MSG_PLAYBACK_STOPPED, 0, 0);
  g_windowManager.SendThreadMessage(msg);
}

void CPlaybackManager::OnPlayBackPaused()
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackPaused();
#endif

  CVariant param;
  param["player"]["speed"] = 0;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  ANNOUNCEMENT::CAnnouncementManager::Get().Announce(Player, "xbmc", "OnPause", m_itemCurrentFile, param);
}

void CPlaybackManager::OnPlayBackResumed()
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackResumed();
#endif

  CVariant param;
  param["player"]["speed"] = 1;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  ANNOUNCEMENT::CAnnouncementManager::Get().Announce(Player, "xbmc", "OnPlay", m_itemCurrentFile, param);
}

void CPlaybackManager::OnPlayBackSpeedChanged(int iSpeed)
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackSpeedChanged(iSpeed);
#endif

  CVariant param;
  param["player"]["speed"] = iSpeed;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  ANNOUNCEMENT::CAnnouncementManager::Get().Announce(Player, "xbmc", "OnSpeedChanged", m_itemCurrentFile, param);
}

void CPlaybackManager::OnPlayBackSeek(int iTime, int seekOffset)
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackSeek(iTime, seekOffset);
#endif

  CVariant param;
  JSONRPC::CJSONUtils::MillisecondsToTimeObject(iTime, param["player"]["time"]);
  JSONRPC::CJSONUtils::MillisecondsToTimeObject(seekOffset, param["player"]["seekoffset"]);;
  param["player"]["playerid"] = g_playlistPlayer.GetCurrentPlaylist();
  param["player"]["speed"] = m_pPlayer->GetPlaySpeed();
  ANNOUNCEMENT::CAnnouncementManager::Get().Announce(Player, "xbmc", "OnSeek", m_itemCurrentFile, param);
  g_infoManager.SetDisplayAfterSeek(2500, seekOffset);
}

void CPlaybackManager::OnPlayBackSeekChapter(int iChapter)
{
#ifdef HAS_PYTHON
  g_pythonParser.OnPlayBackSeekChapter(iChapter);
#endif
}

bool CPlaybackManager::IsPlayingFullScreenVideo() const
{
  return m_pPlayer->IsPlayingVideo() && g_graphicsContext.IsFullScreenVideo();
}