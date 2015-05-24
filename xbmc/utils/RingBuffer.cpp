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

#include "RingBuffer.h"
#include "threads/SingleLock.h"

#include <cstring>
#include <cstdlib>
#include <algorithm>

/* Constructor */
template <typename SIZE_TYPE>
CRingBuffer<typename SIZE_TYPE>::CRingBuffer()
  : m_buffer{nullptr}
  , m_size{0}
  , m_readPtr{0}
  , m_writePtr{0}
  , m_fillCount{0}
{
}

/* Destructor */
template <typename SIZE_TYPE>
CRingBuffer<typename SIZE_TYPE>::~CRingBuffer()
{
  Destroy();
}

/* Create a ring buffer with the specified 'size' */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::Create(SIZE_TYPE size)
{
  CSingleLock lock(m_critSection);
  m_buffer = static_cast<char*>(malloc(size));
  if (m_buffer != nullptr)
  {
    m_size = size;
    return true;
  }
  return false;
}

/* Free the ring buffer and set all values to NULL or 0 */
template <typename SIZE_TYPE>
void CRingBuffer<typename SIZE_TYPE>::Destroy()
{
  CSingleLock lock(m_critSection);
  if (m_buffer != nullptr)
  {
    free(m_buffer);
    m_buffer = nullptr;
  }
  m_size = 0;
  m_readPtr = 0;
  m_writePtr = 0;
  m_fillCount = 0;
}

/* Clear the ring buffer */
template <typename SIZE_TYPE>
void CRingBuffer<typename SIZE_TYPE>::Clear()
{
  CSingleLock lock(m_critSection);
  m_readPtr = 0;
  m_writePtr = 0;
  m_fillCount = 0;
}

/* Read in data from the ring buffer to the supplied buffer 'buf'. The amount
 * read in is specified by 'size'.
 */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::ReadData(char *buf, SIZE_TYPE size)
{
  CSingleLock lock(m_critSection);
  if (size > m_fillCount)
  {
    return false;
  }
  if (size + m_readPtr > m_size)
  {
    SIZE_TYPE chunk = m_size - m_readPtr;
    memcpy(buf, m_buffer + m_readPtr, chunk);
    memcpy(buf + chunk, m_buffer, size - chunk);
    m_readPtr = size - chunk;
  }
  else
  {
    memcpy(buf, m_buffer + m_readPtr, size);
    m_readPtr += size;
  }
  if (m_readPtr == m_size)
    m_readPtr = 0;
  m_fillCount -= size;
  return true;
}

/* Read in data from the ring buffer to another ring buffer object specified by
 * 'rBuf'.
 */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::ReadData(CRingBuffer &rBuf, SIZE_TYPE size)
{
  CSingleLock lock(m_critSection);
  if (rBuf.getBuffer() == nullptr)
    rBuf.Create(size);

  bool bOk = size <= rBuf.getMaxWriteSize() && size <= getMaxReadSize();
  if (bOk)
  {
    SIZE_TYPE chunksize = std::min(size, m_size - m_readPtr);
    bOk = rBuf.WriteData(&getBuffer()[m_readPtr], chunksize);
    if (bOk && chunksize < size)
      bOk = rBuf.WriteData(&getBuffer()[0], size - chunksize);
    if (bOk)
      SkipBytes(size);
  }

  return bOk;
}

/* Write data to ring buffer from buffer specified in 'buf'. Amount read in is
 * specified by 'size'.
 */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::WriteData(const char *buf, SIZE_TYPE size)
{
  CSingleLock lock(m_critSection);
  if (size > m_size - m_fillCount)
  {
    return false;
  }
  if (size + m_writePtr > m_size)
  {
    SIZE_TYPE chunk = m_size - m_writePtr;
    memcpy(m_buffer + m_writePtr, buf, chunk);
    memcpy(m_buffer, buf + chunk, size - chunk);
    m_writePtr = size - chunk;
  }
  else
  {
    memcpy(m_buffer + m_writePtr, buf, size);
    m_writePtr += size;
  }
  if (m_writePtr == m_size)
    m_writePtr = 0;
  m_fillCount += size;
  return true;
}

/* Write data to ring buffer from another ring buffer object specified by
 * 'rBuf'.
 */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::WriteData(CRingBuffer &rBuf, SIZE_TYPE size)
{
  CSingleLock lock(m_critSection);
  if (m_buffer == nullptr)
    Create(size);

  bool bOk = size <= rBuf.getMaxReadSize() && size <= getMaxWriteSize();
  if (bOk)
  {
    SIZE_TYPE readpos = rBuf.getReadPtr();
    SIZE_TYPE chunksize = std::min(size, rBuf.getSize() - readpos);
    bOk = WriteData(&rBuf.getBuffer()[readpos], chunksize);
    if (bOk && chunksize < size)
      bOk = WriteData(&rBuf.getBuffer()[0], size - chunksize);
  }

  return bOk;
}

/* Skip bytes in buffer to be read */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::SkipBytes(int skipSize)
{
  CSingleLock lock(m_critSection);
  if (skipSize < 0)
  {
    return false; // skipping backwards is not supported
  }

  SIZE_TYPE size = skipSize;
  if (size > m_fillCount)
  {
    return false;
  }
  if (size + m_readPtr > m_size)
  {
    SIZE_TYPE chunk = m_size - m_readPtr;
    m_readPtr = size - chunk;
  }
  else
  {
    m_readPtr += size;
  }
  if (m_readPtr == m_size)
    m_readPtr = 0;
  m_fillCount -= size;
  return true;
}

/* Append all content from ring buffer 'rBuf' to this ring buffer */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::Append(CRingBuffer &rBuf)
{
  return WriteData(rBuf, rBuf.getMaxReadSize());
}

/* Copy all content from ring buffer 'rBuf' to this ring buffer overwriting any existing data */
template <typename SIZE_TYPE>
bool CRingBuffer<typename SIZE_TYPE>::Copy(CRingBuffer &rBuf)
{
  Clear();
  return Append(rBuf);
}

/* Our various 'get' methods */
template <typename SIZE_TYPE>
char *CRingBuffer<typename SIZE_TYPE>::getBuffer()
{
  return m_buffer;
}

template <typename SIZE_TYPE>
SIZE_TYPE CRingBuffer<typename SIZE_TYPE>::getSize()
{
  CSingleLock lock(m_critSection);
  return m_size;
}

template <typename SIZE_TYPE>
SIZE_TYPE CRingBuffer<typename SIZE_TYPE>::getReadPtr() const
{
  return m_readPtr;
}

template <typename SIZE_TYPE>
SIZE_TYPE CRingBuffer<typename SIZE_TYPE>::getWritePtr()
{
  CSingleLock lock(m_critSection);
  return m_writePtr;
}

template <typename SIZE_TYPE>
SIZE_TYPE CRingBuffer<typename SIZE_TYPE>::getMaxReadSize()
{
  CSingleLock lock(m_critSection);
  return m_fillCount;
}

template <typename SIZE_TYPE>
SIZE_TYPE CRingBuffer<typename SIZE_TYPE>::getMaxWriteSize()
{
  CSingleLock lock(m_critSection);
  return m_size - m_fillCount;
}
