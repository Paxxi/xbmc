/*
 *      Copyright (C) 2005-2015 Team Kodi
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
 *  along with Kodi; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "TextureManager.h"

#include <utility>
#include <cassert>

#include "addons/Skin.h"
#include "filesystem/Directory.h"
#include "filesystem/File.h"
#include "GraphicContext.h"
#include "system.h"
#include "Texture.h"
#include "threads/SingleLock.h"
#include "URL.h"
#include "utils/log.h"
#include "utils/StringUtils.h"
#include "utils/URIUtils.h"
#include "utils/ScopeGuard.h"

#ifdef _DEBUG_TEXTURES
#include "utils/TimeUtils.h"
#endif
#if defined(TARGET_DARWIN_IOS)
#include "windowing/WindowingFactory.h" // for g_Windowing in CGUITextureManager::FreeUnusedTextures
#endif
#include "FFmpegImage.h"

CTextureArray::CTextureArray(int width, int height, int loops,  bool texCoordsArePixels)
  : m_width(width)
  , m_height(height)
  , m_orientation(0)
  , m_loops(loops)
  , m_texWidth(0)
  , m_texHeight(0)
  , m_texCoordsArePixels(false)
{
}

CTextureArray::CTextureArray()
{
  Reset();
}

std::size_t CTextureArray::size() const
{
  return m_textures.size();
}


void CTextureArray::Reset()
{
  m_textures.clear();
  m_delays.clear();
  m_width = 0;
  m_height = 0;
  m_loops = 0;
  m_orientation = 0;
  m_texWidth = 0;
  m_texHeight = 0;
  m_texCoordsArePixels = false;
}

void CTextureArray::Add(CBaseTexture *texture, int delay)
{
  if (!texture)
    return;

  m_textures.push_back(texture);
  m_delays.push_back(delay);

  m_texWidth = texture->GetTextureWidth();
  m_texHeight = texture->GetTextureHeight();
  m_texCoordsArePixels = false;
}

void CTextureArray::Set(CBaseTexture *texture, int width, int height)
{
  assert(!m_textures.size()); // don't try and set a texture if we already have one!
  m_width = width;
  m_height = height;
  m_orientation = texture ? texture->GetOrientation() : 0;
  Add(texture, 2);
}

void CTextureArray::Free()
{
  CSingleLock lock(g_graphicsContext);
  for (auto tex : m_textures)
    delete tex;

  m_textures.clear();
  m_delays.clear();

  Reset();
}


/************************************************************************/
/*                                                                      */
/************************************************************************/

CTextureMap::CTextureMap()
  : m_referenceCount(0)
  , m_memUsage(0)
{
}

CTextureMap::CTextureMap(const std::string& textureName, int width, int height, int loops)
  : m_texture(width, height, loops)
  , m_textureName(textureName)
  , m_referenceCount(0)
  , m_memUsage(0)
{
}

CTextureMap::~CTextureMap()
{
  FreeTexture();
}

bool CTextureMap::Release()
{
  if (!m_texture.m_textures.size())
    return true;
  if (!m_referenceCount)
    return true;

  m_referenceCount--;
  if (!m_referenceCount)
  {
    return true;
  }
  return false;
}

const std::string& CTextureMap::GetName() const
{
  return m_textureName;
}

const CTextureArray& CTextureMap::GetTexture()
{
  m_referenceCount++;
  return m_texture;
}

void CTextureMap::Dump() const
{
  if (!m_referenceCount)
    return;   // nothing to see here

  CLog::Log(LOGDEBUG, "%s: texture:%s has %" PRIuS" frames %i refcount", __FUNCTION__, m_textureName.c_str(), m_texture.m_textures.size(), m_referenceCount);
}

unsigned int CTextureMap::GetMemoryUsage() const
{
  return m_memUsage;
}

void CTextureMap::Flush()
{
  if (!m_referenceCount)
    FreeTexture();
}


void CTextureMap::FreeTexture()
{
  m_texture.Free();
}

void CTextureMap::SetHeight(int height)
{
  m_texture.m_height = height;
}

void CTextureMap::SetWidth(int width)
{
  m_texture.m_width = width;
}

bool CTextureMap::IsEmpty() const
{
  return m_texture.m_textures.empty();
}

void CTextureMap::Add(CBaseTexture* texture, int delay)
{
  m_texture.Add(texture, delay);

  if (texture)
    m_memUsage += sizeof(CTexture) + (texture->GetTextureWidth() * texture->GetTextureHeight() * 4);
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
CGUITextureManager::CGUITextureManager()
{
}

CGUITextureManager::~CGUITextureManager()
{
  Cleanup();
}

/************************************************************************/
/*                                                                      */
/************************************************************************/
bool CGUITextureManager::CanLoad(const std::string &texturePath)
{
  if (texturePath == "-")
    return false;

  if (!CURL::IsFullPath(texturePath))
    return true;  // assume we have it

  // we can't (or shouldn't) be loading from remote paths, so check these
  return URIUtils::IsHD(texturePath);
}

bool CGUITextureManager::HasTexture(const std::string &textureName, std::string *path, int *bundle, int *size)
{
  // default values
  if (bundle) *bundle = -1;
  if (size) *size = 0;
  if (path) *path = textureName;

  if (textureName.empty())
    return false;

  if (!CanLoad(textureName))
    return false;

  // Check our loaded and bundled textures - we store in bundles using \\.
  auto bundledName = CTextureBundleXBT::Normalize(textureName);
  for (auto& pMap : m_vecTextures)
  {
    if (pMap->GetName() == textureName)
    {
      if (size)
        *size = 1;

      return true;
    }
  }

  for (int i = 0; i < 2; ++i)
  {
    //if (m_TexBundle[i].HasFile(bundledName))
    //{
    //  if (bundle)
    //    *bundle = i;

    //  return true;
    //}
  }

  auto fullPath = GetTexturePath(textureName);
  if (path)
    *path = fullPath;

  return !fullPath.empty();
}

void CGUITextureManager::LoadTextureGif(const CTextureBundleXBT& bundle, const std::string& strTextureName)
{
  CBaseTexture **pTextures = nullptr;
  int nLoops = 0, width = 0, height = 0;
  int* Delay = nullptr;
  int nImages = bundle.LoadAnim(strTextureName, &pTextures, width, height, nLoops, &Delay);
  if (!nImages)
  {
    CLog::Log(LOGERROR, "Texture manager unable to load bundled file: %s", strTextureName.c_str());
    delete[] pTextures;
    delete[] Delay;
  }

  unsigned int maxWidth = 0;
  unsigned int maxHeight = 0;
  auto pMap = new CTextureMap(strTextureName, width, height, nLoops);

  for (auto iImage = 0; iImage < nImages; ++iImage)
  {
    pMap->Add(pTextures[iImage], Delay[iImage]);
    maxWidth = std::max(maxWidth, pTextures[iImage]->GetWidth());
    maxHeight = std::max(maxHeight, pTextures[iImage]->GetHeight());
  }

  pMap->SetWidth(static_cast<int>(maxWidth));
  pMap->SetHeight(static_cast<int>(maxHeight));

  delete[] pTextures;
  delete[] Delay;

  m_vecTextures.emplace_back(pMap);
}

void CGUITextureManager::LoadTextureGifOrPng(const CTextureBundleXBT& bundle,
                                            const std::string& strTextureName, std::string strPath)
{

  std::string mimeType;
  if (StringUtils::EndsWithNoCase(strPath, ".gif"))
    mimeType = "image/gif";
  else if (StringUtils::EndsWithNoCase(strPath, ".apng"))
    mimeType = "image/apng";

  XFILE::CFile file;
  XFILE::auto_buffer buf;
  CFFmpegImage anim(mimeType);

  auto pMap = new CTextureMap(strTextureName, 0, 0, 0);

  if (file.LoadFile(strPath, buf) <= 0 ||
    !anim.Initialize(reinterpret_cast<uint8_t*>(buf.get()), buf.size()))
  {
    CLog::Log(LOGERROR, "Texture manager unable to load file: %s", CURL::GetRedacted(strPath).c_str());
    file.Close();
  }

  unsigned int maxWidth = 0;
  unsigned int maxHeight = 0;
  uint64_t maxMemoryUsage = 91238400;// 1920*1080*4*11 bytes, i.e, a total of approx. 12 full hd frames

  auto frame = anim.ReadFrame();
  while (frame)
  {
    CTexture *glTexture = new CTexture();
    if (glTexture)
    {
      glTexture->LoadFromMemory(anim.Width(), anim.Height(), frame->GetPitch(), XB_FMT_A8R8G8B8, true, frame->m_pImage);
      pMap->Add(glTexture, frame->m_delay);
      maxWidth = std::max(maxWidth, glTexture->GetWidth());
      maxHeight = std::max(maxHeight, glTexture->GetHeight());
    }

    if (pMap->GetMemoryUsage() <= maxMemoryUsage)
    {
      frame = anim.ReadFrame();
    } 
    else
    {
      CLog::Log(LOGDEBUG, "Memory limit (%" PRIu64 " bytes) exceeded, %i frames extracted from file : %s", (maxMemoryUsage/11)*12,pMap->GetTexture().size(), CURL::GetRedacted(strPath).c_str());
      break;
    }
  }

  pMap->SetWidth(static_cast<int>(maxWidth));
  pMap->SetHeight(static_cast<int>(maxHeight));

  file.Close();

  m_vecTextures.emplace_back(pMap);
}

void CGUITextureManager::LoadTexture(const CTextureBundleXBT& bundle, const std::string& strTextureName, std::string strPath)
{
  CBaseTexture *pTexture = nullptr;
  int width = 0, height = 0;
  if (!bundle.LoadTexture(strTextureName, &pTexture, width, height))
  {
    CLog::Log(LOGERROR, "Texture manager unable to load bundled file: %s", strTextureName.c_str());
  }

  pTexture = CBaseTexture::LoadFromFile(strPath);
  if (!pTexture)
    return;
  width = pTexture->GetWidth();
  height = pTexture->GetHeight();

  if (!pTexture)
    return;

  auto pMap = new CTextureMap(strTextureName, width, height, 0);
  pMap->Add(pTexture, 100);
  m_vecTextures.emplace_back(pMap);
}

void CGUITextureManager::Load(const CTextureBundleXBT& bundle, const std::string& strTextureName)
{
  auto strPath = GetTexturePath(strTextureName);

  if (StringUtils::EndsWithNoCase(strPath, ".gif"))
    return LoadTextureGif(bundle, strTextureName);

  if (StringUtils::EndsWithNoCase(strPath, ".gif") ||
           StringUtils::EndsWithNoCase(strPath, ".apng"))
    return LoadTextureGifOrPng(bundle, strTextureName, strPath);

  return LoadTexture(bundle, strTextureName, strPath);
}

CTextureArray CGUITextureManager::GetTexture(const std::string& strTextureName)
{
  static CTextureArray emptyTexture;

  auto iter = std::find_if(m_vecTextures.begin(), m_vecTextures.end(), [&strTextureName](
    const std::unique_ptr<CTextureMap>& tex)
  {
    return tex->GetName() == strTextureName;
  });
  if (iter == m_vecTextures.end())
    return emptyTexture;

  return (*iter)->GetTexture();
}

bool CGUITextureManager::LoadBundle()
{
  if (!LoadBundleInternal(true))
    return LoadBundleInternal(false);

  return true;
}

bool CGUITextureManager::LoadBundleInternal(bool themeBundle)
{
  CTextureBundleXBT bundle;
  bundle.SetThemeBundle(themeBundle);

  if (!bundle.Open())
    return false;

  auto textures = bundle.GetFiles();
  m_vecTextures.reserve(textures.size());

  for (auto& tex : textures)
    Load(bundle, tex.GetPath());

  return true;
}

void CGUITextureManager::Cleanup()
{
}

void CGUITextureManager::Dump() const
{
}

void CGUITextureManager::Flush()
{
}

unsigned int CGUITextureManager::GetMemoryUsage() const
{
  unsigned int memUsage = 0;
  for (const auto& pMap : m_vecTextures)
    memUsage += pMap->GetMemoryUsage();

  return memUsage;
}

void CGUITextureManager::SetTexturePath(const std::string &texturePath)
{
  CSingleLock lock(m_section);
  m_texturePaths.clear();
  AddTexturePath(texturePath);
}

void CGUITextureManager::AddTexturePath(const std::string &texturePath)
{
  CSingleLock lock(m_section);
  if (!texturePath.empty())
    m_texturePaths.push_back(texturePath);
}

void CGUITextureManager::RemoveTexturePath(const std::string &texturePath)
{
  CSingleLock lock(m_section);
  auto it = std::remove(m_texturePaths.begin(), m_texturePaths.end(), texturePath);
  m_texturePaths.erase(it);
}

std::string CGUITextureManager::GetTexturePath(const std::string &textureName, bool directory /* = false */)
{
  if (CURL::IsFullPath(textureName))
    return textureName;

  // texture doesn't include the full path, so check all fallbacks
  CSingleLock lock(m_section);
  for (auto& it : m_texturePaths)
  {
    auto path = URIUtils::AddFileToFolder(it.c_str(), "media", textureName);
    if (directory && XFILE::CDirectory::Exists(path))
        return path;

    if (!directory && XFILE::CFile::Exists(path))
      return path;
  }

  CLog::Log(LOGERROR, "CGUITextureManager::GetTexturePath: could not find texture '%s'", textureName.c_str());
  return std::string();
}

void CGUITextureManager::GetBundledTexturesFromPath(const std::string& texturePath, std::vector<std::string> &items) const
{
  CLog::Log(LOGDEBUG, "yada %s", texturePath.c_str());
  //m_TexBundle[0].GetTexturesFromPath(texturePath, items);
  //if (items.empty())
  //  m_TexBundle[1].GetTexturesFromPath(texturePath, items);
}
