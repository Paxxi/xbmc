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
#include "system.h"
#include "Visualisation.h"
#include "GUIInfoManager.h"
#include "ServiceBroker.h"
#include "guiinfo/GUIInfoLabels.h"
#include "Application.h"
#include "filesystem/SpecialProtocol.h"
#include "guilib/GraphicContext.h"
#include "guilib/WindowIDs.h"
#include "music/tags/MusicInfoTag.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "windowing/WindowingFactory.h"
#include "utils/log.h"
#include "utils/URIUtils.h"
#include "utils/StringUtils.h"
#include "cores/AudioEngine/Interfaces/AE.h"

using namespace MUSIC_INFO;
using namespace ADDON;

CAudioBuffer::CAudioBuffer(int iSize)
{
  m_iLen = iSize;
  m_pBuffer = new float[iSize];
}

CAudioBuffer::~CAudioBuffer()
{
  delete [] m_pBuffer;
}

const float* CAudioBuffer::Get() const
{
  return m_pBuffer;
}

void CAudioBuffer::Set(const float* psBuffer, int iSize)
{
  if (iSize<0)
    return;
  memcpy(m_pBuffer, psBuffer, iSize * sizeof(float));
  for (int i = iSize; i < m_iLen; ++i) m_pBuffer[i] = 0;
}

CVisualisation::CVisualisation(CAddonInfo addonInfo)
  : CAddonDll(std::move(addonInfo))
{
  memset(&m_struct, 0, sizeof(m_struct));
}

bool CVisualisation::Create(int x, int y, int w, int h, void *device)
{
#ifdef HAS_DX
  m_struct.props.device = g_Windowing.Get3D11Context();
#else
  m_struct.props.device = NULL;
#endif
  m_struct.props.x = x;
  m_struct.props.y = y;
  m_struct.props.width = w;
  m_struct.props.height = h;
  m_struct.props.pixelRatio = g_graphicsContext.GetResInfo().fPixelRatio;

  m_struct.props.name = strdup(Name().c_str());
  m_struct.props.presets = strdup(CSpecialProtocol::TranslatePath(Path()).c_str());
  m_struct.props.profile = strdup(CSpecialProtocol::TranslatePath(Profile()).c_str());
  m_struct.props.submodule = NULL;

  m_struct.toKodi.kodiInstance = this;

  if (CAddonDll::Create(ADDON_INSTANCE_VISUALIZATION, &m_struct, &m_struct.props) == ADDON_STATUS_OK)
  {
    // Start the visualisation
    std::string strFile = URIUtils::GetFileName(g_application.CurrentFile());
    CLog::Log(LOGDEBUG, "Visualisation::Start()\n");
    m_struct.toAddon.Start(m_iChannels, m_iSamplesPerSec, m_iBitsPerSample, strFile.c_str());

    m_hasPresets = GetPresets();

    if (GetSubModules())
      m_struct.props.submodule = strdup(CSpecialProtocol::TranslatePath(m_submodules.front()).c_str());
    else
      m_struct.props.submodule = NULL;

    CreateBuffers();

    CServiceBroker::GetActiveAE().RegisterAudioCallback(this);

    return true;
  }
  return false;
}

void CVisualisation::Start(int iChannels, int iSamplesPerSec, int iBitsPerSample, const std::string &strSongName)
{
  // notify visz. that new song has been started
  // pass it the nr of audio channels, sample rate, bits/sample and offcourse the songname
  if (Initialized())
  {
    m_struct.toAddon.Start(iChannels, iSamplesPerSec, iBitsPerSample, strSongName.c_str());
  }
}

void CVisualisation::AudioData(const float* pAudioData, int iAudioDataLength, float *pFreqData, int iFreqDataLength)
{
  // pass audio data to visz.
  // audio data: is short audiodata [channel][iAudioDataLength] containing the raw audio data
  // iAudioDataLength = length of audiodata array
  // pFreqData = fft-ed audio data
  // iFreqDataLength = length of pFreqData
  if (Initialized())
  {
    m_struct.toAddon.AudioData(pAudioData, iAudioDataLength, pFreqData, iFreqDataLength);
  }
}

void CVisualisation::Render()
{
  // ask visz. to render itself
  if (Initialized())
  {
    m_struct.toAddon.Render();
  }
}

void CVisualisation::Stop()
{
  CServiceBroker::GetActiveAE().UnregisterAudioCallback(this);
  if (Initialized())
  {
    m_struct.toAddon.Stop();
  }
}

void CVisualisation::GetInfo(VIS_INFO *info)
{
  if (Initialized())
  {
    m_struct.toAddon.GetInfo(info);
  }
}

bool CVisualisation::OnAction(VIS_ACTION action, void *param)
{
  if (!Initialized())
    return false;

  // see if vis wants to handle the input
  // returns false if vis doesnt want the input
  // returns true if vis handled the input
  if (action != VIS_ACTION_NONE && m_struct.toAddon.OnAction)
  {
    // if this is a VIS_ACTION_UPDATE_TRACK action, copy relevant
    // tags from CMusicInfoTag to VisTag
    if ( action == VIS_ACTION_UPDATE_TRACK && param )
    {
      const CMusicInfoTag* tag = (const CMusicInfoTag*)param;
      std::string artist(tag->GetArtistString());
      std::string albumArtist(tag->GetAlbumArtistString());
      std::string genre(StringUtils::Join(tag->GetGenre(), g_advancedSettings.m_musicItemSeparator));

      VisTrack track;
      track.title       = tag->GetTitle().c_str();
      track.artist      = artist.c_str();
      track.album       = tag->GetAlbum().c_str();
      track.albumArtist = albumArtist.c_str();
      track.genre       = genre.c_str();
      track.comment     = tag->GetComment().c_str();
      track.lyrics      = tag->GetLyrics().c_str();
      track.trackNumber = tag->GetTrackNumber();
      track.discNumber  = tag->GetDiscNumber();
      track.duration    = tag->GetDuration();
      track.year        = tag->GetYear();
      track.rating      = tag->GetUserrating();

      return m_struct.toAddon.OnAction(action, &track);
    }
    return m_struct.toAddon.OnAction((int)action, param);
  }
  return false;
}

void CVisualisation::OnInitialize(int iChannels, int iSamplesPerSec, int iBitsPerSample)
{
  if (!Initialized())
    return ;
  CLog::Log(LOGDEBUG, "OnInitialize() started");

  m_iChannels = iChannels;
  m_iSamplesPerSec = iSamplesPerSec;
  m_iBitsPerSample = iBitsPerSample;
  UpdateTrack();

  CLog::Log(LOGDEBUG, "OnInitialize() done");
}

void CVisualisation::OnAudioData(const float* pAudioData, int iAudioDataLength)
{
  if (!Initialized())
    return ;

  // FIXME: iAudioDataLength should never be less than 0
  if (iAudioDataLength<0)
    return;

  // Save our audio data in the buffers
  std::unique_ptr<CAudioBuffer> pBuffer ( new CAudioBuffer(iAudioDataLength) );
  pBuffer->Set(pAudioData, iAudioDataLength);
  m_vecBuffers.push_back( pBuffer.release() );

  if ( (int)m_vecBuffers.size() < m_iNumBuffers) return ;

  std::unique_ptr<CAudioBuffer> ptrAudioBuffer ( m_vecBuffers.front() );
  m_vecBuffers.pop_front();
  // Fourier transform the data if the vis wants it...
  if (m_bWantsFreq)
  {
    const float *psAudioData = ptrAudioBuffer->Get();

    if (!m_transform)
      m_transform.reset(new RFFT(AUDIO_BUFFER_SIZE/2, false)); // half due to stereo

    m_transform->calc(psAudioData, m_fFreq);

    // Transfer data to our visualisation
    AudioData(psAudioData, iAudioDataLength, m_fFreq, AUDIO_BUFFER_SIZE/2); // half due to complex-conjugate
  }
  else
  { // Transfer data to our visualisation
    AudioData(ptrAudioBuffer->Get(), iAudioDataLength, NULL, 0);
  }
  return ;
}

void CVisualisation::CreateBuffers()
{
  ClearBuffers();

  // Get the number of buffers from the current vis
  VIS_INFO info;
  m_struct.toAddon.GetInfo(&info);
  m_iNumBuffers = info.iSyncDelay + 1;
  m_bWantsFreq = (info.bWantsFreq != 0);
  if (m_iNumBuffers > MAX_AUDIO_BUFFERS)
    m_iNumBuffers = MAX_AUDIO_BUFFERS;
  if (m_iNumBuffers < 1)
    m_iNumBuffers = 1;
}

void CVisualisation::ClearBuffers()
{
  m_bWantsFreq = false;
  m_iNumBuffers = 0;

  while (!m_vecBuffers.empty())
  {
    CAudioBuffer* pAudioBuffer = m_vecBuffers.front();
    delete pAudioBuffer;
    m_vecBuffers.pop_front();
  }
  for (float & j : m_fFreq)
  {
    j = 0.0f;
  }
}

bool CVisualisation::UpdateTrack()
{
  bool handled = false;
  if (Initialized())
  {
    // get the current album art filename
    m_AlbumThumb = CSpecialProtocol::TranslatePath(g_infoManager.GetImage(MUSICPLAYER_COVER, WINDOW_INVALID));

    // get the current track tag
    const CMusicInfoTag* tag = g_infoManager.GetCurrentSongTag();

    if (m_AlbumThumb == "DefaultAlbumCover.png")
      m_AlbumThumb = "";
    else
      CLog::Log(LOGDEBUG,"Updating visualisation albumart: %s", m_AlbumThumb.c_str());

    // inform the visualisation of the current album art
    if (OnAction( VIS_ACTION_UPDATE_ALBUMART, (void*)( m_AlbumThumb.c_str() ) ) )
      handled = true;

    // inform the visualisation of the current track's tag information
    if ( tag && OnAction( VIS_ACTION_UPDATE_TRACK, (void*)tag ) )
      handled = true;
  }
  return handled;
}

bool CVisualisation::GetPresetList(std::vector<std::string> &vecpresets)
{
  vecpresets = m_presets;
  return !m_presets.empty();
}

bool CVisualisation::GetPresets()
{
  m_presets.clear();
  char **presets = NULL;
  unsigned int entries = m_struct.toAddon.GetPresets(&presets);

  if (presets && entries > 0)
  {
    for (unsigned i=0; i < entries; i++)
    {
      if (presets[i])
      {
        m_presets.push_back(presets[i]);
      }
    }
  }
  return (!m_presets.empty());
}

bool CVisualisation::GetSubModuleList(std::vector<std::string> &vecmodules)
{
  vecmodules = m_submodules;
  return !m_submodules.empty();
}

bool CVisualisation::GetSubModules()
{
  m_submodules.clear();
  char **modules = NULL;
  unsigned int entries = m_struct.toAddon.GetSubModules(&modules);

  if (modules && entries > 0)
  {
    for (unsigned i=0; i < entries; i++)
    {
      if (modules[i])
      {
        m_submodules.push_back(modules[i]);
      }
    }
  }
  return (!m_submodules.empty());
}

std::string CVisualisation::GetFriendlyName(const std::string& strVisz,
                                            const std::string& strSubModule)
{
  // should be of the format "moduleName (visName)"
  return strSubModule + " (" + strVisz + ")";
}

bool CVisualisation::IsLocked()
{
  if (!m_presets.empty())
  {
    if (!Initialized())
      return false;

    return m_struct.toAddon.IsLocked();
  }
  return false;
}

void CVisualisation::Destroy()
{
  // Free what was allocated in method CVisualisation::Create
  if (m_struct.props.name)
  {
    free((void *) m_struct.props.name);
    m_struct.props.name = nullptr;
  }
  if (m_struct.props.presets)
  {
    free((void *) m_struct.props.presets);
    m_struct.props.presets = nullptr;
  }
  if (m_struct.props.profile)
  {
    free((void *) m_struct.props.profile);
    m_struct.props.profile = nullptr;
  }
  if (m_struct.props.submodule)
  {
    free((void *) m_struct.props.submodule);
    m_struct.props.submodule = nullptr;
  }

  CAddonDll::Destroy();
}

unsigned CVisualisation::GetPreset()
{
  return m_struct.toAddon.GetPreset();
}

std::string CVisualisation::GetPresetName()
{
  if (!m_presets.empty())
    return m_presets[GetPreset()];
  else
    return "";
}

bool CVisualisation::IsInUse() const
{
  return CServiceBroker::GetSettings().GetString(CSettings::SETTING_MUSICPLAYER_VISUALISATION) == ID();
}
