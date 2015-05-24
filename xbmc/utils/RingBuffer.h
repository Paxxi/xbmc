#pragma once
/*
 *      Copyright (C) 2010-2013 Team XBMC
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

#include "threads/CriticalSection.h"

template <typename SIZE_TYPE>
class CRingBuffer
{
  CCriticalSection m_critSection;
  char *m_buffer;
  SIZE_TYPE m_size;
  SIZE_TYPE m_readPtr;
  SIZE_TYPE m_writePtr;
  SIZE_TYPE m_fillCount;
public:
  CRingBuffer();
  ~CRingBuffer();
  bool Create(SIZE_TYPE size);
  void Destroy();
  void Clear();
  bool ReadData(char *buf, SIZE_TYPE size);
  bool ReadData(CRingBuffer &rBuf, SIZE_TYPE size);
  bool WriteData(const char *buf, SIZE_TYPE size);
  bool WriteData(CRingBuffer &rBuf, SIZE_TYPE size);
  bool SkipBytes(int skipSize);
  bool Append(CRingBuffer &rBuf);
  bool Copy(CRingBuffer &rBuf);
  char *getBuffer();
  SIZE_TYPE getSize();
  SIZE_TYPE getReadPtr() const;
  SIZE_TYPE getWritePtr();
  SIZE_TYPE getMaxReadSize();
  SIZE_TYPE getMaxWriteSize();
};

#if defined(TARGET_WINDOWS)
#pragma warning(disable : 4661)
#endif

template class CRingBuffer<size_t>;