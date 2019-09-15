/*
 *  Copyright (C) 2010-2018 Team Kodi
 *  This file is part of Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#pragma once

#include "threads/CriticalSection.h"

class CRingBuffer
{
  CCriticalSection m_critSection;
  char* m_buffer;
  size_t m_size;
  size_t m_readPtr;
  size_t m_writePtr;
  size_t m_fillCount;

public:
  CRingBuffer();
  ~CRingBuffer();
  bool Create(size_t size);
  void Destroy();
  void Clear();
  bool ReadData(char* buf, size_t size);
  bool ReadData(CRingBuffer& rBuf, size_t size);
  bool WriteData(const char* buf, size_t size);
  bool WriteData(CRingBuffer& rBuf, size_t size);
  bool SkipBytes(int skipSize);
  bool Append(CRingBuffer& rBuf);
  bool Copy(CRingBuffer& rBuf);
  char* getBuffer();
  size_t getSize();
  size_t getReadPtr() const;
  size_t getWritePtr();
  size_t getMaxReadSize();
  size_t getMaxWriteSize();
};
