/*!
\file FileItemList.h
\brief
*/
#pragma once

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
*  along with XBMC; see the file COPYING.  If not, see
*  <http://www.gnu.org/licenses/>.
*
*/


#include "FileItem.h"
/*!
\brief Represents a list of files
\sa CFileItemList, CFileItem
*/
class CFileItemList : public CFileItem
{
public:
  enum CACHE_TYPE { CACHE_NEVER = 0, CACHE_IF_SLOW, CACHE_ALWAYS };

  CFileItemList();
  explicit CFileItemList(const std::string& strPath);
  virtual ~CFileItemList();
  virtual void Archive(CArchive& ar);
  CFileItemPtr operator[] (int iItem);
  const CFileItemPtr operator[] (int iItem) const;
  CFileItemPtr operator[] (const std::string& strPath);
  const CFileItemPtr operator[] (const std::string& strPath) const;
  void Clear();
  void ClearItems();
  void Add(const CFileItemPtr &pItem);
  void AddFront(const CFileItemPtr &pItem, int itemPosition);
  void Remove(CFileItem* pItem);
  void Remove(int iItem);
  CFileItemPtr Get(int iItem);
  const CFileItemPtr Get(int iItem) const;
  const VECFILEITEMS GetList() const { return m_items; }
  CFileItemPtr Get(const std::string& strPath);
  const CFileItemPtr Get(const std::string& strPath) const;
  int Size() const;
  bool IsEmpty() const;
  void Append(const CFileItemList& itemlist);
  void Assign(const CFileItemList& itemlist, bool append = false);
  bool Copy(const CFileItemList& item, bool copyItems = true);
  void Reserve(int iCount);
  void Sort(SortBy sortBy, SortOrder sortOrder, SortAttribute sortAttributes = SortAttributeNone);
  /* \brief Sorts the items based on the given sorting options

  In contrast to Sort (see above) this does not change the internal
  state by storing the sorting method and order used and therefore
  will always execute the sorting even if the list of items has
  already been sorted with the same options before.
  */
  void Sort(SortDescription sortDescription);
  void Randomize();
  void FillInDefaultIcons();
  int GetFolderCount() const;
  int GetFileCount() const;
  int GetSelectedCount() const;
  int GetObjectCount() const;
  void FilterCueItems();
  void RemoveExtensions();
  void SetFastLookup(bool fastLookup);
  bool Contains(const std::string& fileName) const;
  bool GetFastLookup() const { return m_fastLookup; };

  /*! \brief stack a CFileItemList
  By default we stack all items (files and folders) in a CFileItemList
  \param stackFiles whether to stack all items or just collapse folders (defaults to true)
  \sa StackFiles,StackFolders
  */
  void Stack(bool stackFiles = true);

  SortOrder GetSortOrder() const { return m_sortDescription.sortOrder; }
  SortBy GetSortMethod() const { return m_sortDescription.sortBy; }
  /*! \brief load a CFileItemList out of the cache

  The file list may be cached based on which window we're viewing in, as different
  windows will be listing different portions of the same URL (eg viewing music files
  versus viewing video files)

  \param windowID id of the window that's loading this list (defaults to 0)
  \return true if we loaded from the cache, false otherwise.
  \sa Save,RemoveDiscCache
  */
  bool Load(int windowID = 0);

  /*! \brief save a CFileItemList to the cache

  The file list may be cached based on which window we're viewing in, as different
  windows will be listing different portions of the same URL (eg viewing music files
  versus viewing video files)

  \param windowID id of the window that's saving this list (defaults to 0)
  \return true if successful, false otherwise.
  \sa Load,RemoveDiscCache
  */
  bool Save(int windowID = 0);
  void SetCacheToDisc(CACHE_TYPE cacheToDisc) { m_cacheToDisc = cacheToDisc; }
  bool CacheToDiscAlways() const { return m_cacheToDisc == CACHE_ALWAYS; }
  bool CacheToDiscIfSlow() const { return m_cacheToDisc == CACHE_IF_SLOW; }
  /*! \brief remove a previously cached CFileItemList from the cache

  The file list may be cached based on which window we're viewing in, as different
  windows will be listing different portions of the same URL (eg viewing music files
  versus viewing video files)

  \param windowID id of the window whose cache we which to remove (defaults to 0)
  \sa Save,Load
  */
  void RemoveDiscCache(int windowID = 0) const;
  bool AlwaysCache() const;

  void Swap(unsigned int item1, unsigned int item2);

  /*! \brief Update an item in the item list
  \param item the new item, which we match based on path to an existing item in the list
  \return true if the item exists in the list (and was thus updated), false otherwise.
  */
  bool UpdateItem(const CFileItem *item);

  void AddSortMethod(SortBy sortBy, int buttonLabel, const LABEL_MASKS &labelMasks, SortAttribute sortAttributes = SortAttributeNone);
  void AddSortMethod(SortBy sortBy, SortAttribute sortAttributes, int buttonLabel, const LABEL_MASKS &labelMasks);
  void AddSortMethod(SortDescription sortDescription, int buttonLabel, const LABEL_MASKS &labelMasks);
  bool HasSortDetails() const { return m_sortDetails.size() != 0; }
  const std::vector<GUIViewSortDetails> &GetSortDetails() const { return m_sortDetails; }

  /*! \brief Specify whether this list should be sorted with folders separate from files
  By default we sort with folders listed (and sorted separately) except for those sort modes
  which should be explicitly sorted with folders interleaved with files (eg SORT_METHOD_FILES).
  With this set the folder state will be ignored, allowing folders and files to sort interleaved.
  \param sort whether to ignore the folder state.
  */
  void SetSortIgnoreFolders(bool sort) { m_sortIgnoreFolders = sort; };
  bool GetReplaceListing() const { return m_replaceListing; };
  void SetReplaceListing(bool replace);
  void SetContent(const std::string &content) { m_content = content; };
  const std::string &GetContent() const { return m_content; };

  void ClearSortState();
private:
  void Sort(FILEITEMLISTCOMPARISONFUNC func);
  void FillSortFields(FILEITEMFILLFUNC func);
  std::string GetDiscFileCache(int windowID) const;

  /*!
  \brief stack files in a CFileItemList
  \sa Stack
  */
  void StackFiles();

  /*!
  \brief stack folders in a CFileItemList
  \sa Stack
  */
  void StackFolders();

  VECFILEITEMS m_items;
  MAPFILEITEMS m_map;
  bool m_fastLookup;
  SortDescription m_sortDescription;
  bool m_sortIgnoreFolders;
  CACHE_TYPE m_cacheToDisc;
  bool m_replaceListing;
  std::string m_content;

  std::vector<GUIViewSortDetails> m_sortDetails;

  CCriticalSection m_lock;
};
