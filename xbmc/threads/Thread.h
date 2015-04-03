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

// Thread.h: interface for the CThread class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <stdint.h>
#include "Event.h"
#include "threads/ThreadImpl.h"
#include "threads/ThreadLocal.h"
#include "commons/ilog.h"

#ifdef TARGET_DARWIN
#include <mach/mach.h>
#endif

/*! \class IRunnable
 *
 * \brief Interface to implement to let a task run on a separate thread
 */
class IRunnable
{
public:
  /*! \brief  This method must be overridden and is the entry point
   *          called when the thread is launched
   *
   */
  virtual void Run()=0;
  virtual ~IRunnable() {}
};

// minimum as mandated by XTL
#define THREAD_MINSTACKSIZE 0x10000

/**
 * \class CThread
 *
 * \brief Represents a thread object, can either be instantiated directly
 *        or inherited from.
 *
 * TODO: long description
 *
 *
 */
class CThread
{
  static XbmcCommons::ILogger* logger;

protected:
  /*! \brief Constructor used when inheriting from CThread
   *
   * \param[in] Name of the thread
   */
  CThread(const char* ThreadName);

public:
  /*! \brief  Constructor when instantiating a CThread object
   *          this will not actually launch a new thread
   *
   * \param[in] pRunnable   Object inheriting from IRunnable that will be run in the
   *                        new thread
   * \param[in] ThreadName  Name of the thread
   * \sa IRunnable
   * \sa CThread::Create
   */
  CThread(IRunnable* pRunnable, const char* ThreadName);
  virtual ~CThread();

  /*! \brief  Create and launch a new thread.
   *
   *  This method is not thread safe and the caller must make
   *  sure that it's not called twice. Failing to do so will
   *  call exit(1) and kill the process without warning
   *
   * \param[in] bAutoDelete Should the thread be deleted once it's finished
   *                        running? defaults to false
   * \param[in] stacksize   Set the threads stack size. Default value is 0
   *                        which lets the system decide the size
   */
  void Create(bool bAutoDelete = false, unsigned stacksize = 0);

  /*! \brief Put the current thread to sleep for some amount of time
   *
   *  The sleep method varies depending on how this is called, it can either
   *  use libc sleep or an Event
   *
   * \param[in] milliseconds  Time to sleep in milliseconds
   */
  void Sleep(unsigned int milliseconds);

  /*! \brief Get the realtime priority value
   *
   *  This is currently only implemented on OSX
   *
   * \return  priority value, range depends on system 
   */
  int GetSchedRRPriority(void);

  /*! \brief Set the realtime priority value to use
   *
   * \param[in] iPriority realtime priority value to use
   * \return true if successful otherwise false
   */
  bool SetPrioritySched_RR(int iPriority);
  bool IsAutoDelete() const;
  virtual void StopThread(bool bWait = true);
  bool IsRunning() const;

  // -----------------------------------------------------------------------------------
  // These are platform specific and can be found in ./platform/[platform]/ThreadImpl.cpp
  // -----------------------------------------------------------------------------------
  bool IsCurrentThread() const;
  int GetMinPriority(void);
  int GetMaxPriority(void);
  int GetNormalPriority(void);
  int GetPriority(void);
  bool SetPriority(const int iPriority);
  bool WaitForThreadExit(unsigned int milliseconds);
  float GetRelativeUsage();  // returns the relative cpu usage of this thread since last call
  int64_t GetAbsoluteUsage();
  // -----------------------------------------------------------------------------------

  static bool IsCurrentThread(const ThreadIdentifier tid);
  static ThreadIdentifier GetCurrentThreadId();
  static CThread* GetCurrentThread();
  static inline void SetLogger(XbmcCommons::ILogger* logger_) { CThread::logger = logger_; }
  static inline XbmcCommons::ILogger* GetLogger() { return CThread::logger; }

  virtual void OnException(){} // signal termination handler
protected:
  virtual void OnStartup(){};
  virtual void OnExit(){};
  virtual void Process();

  volatile bool m_bStop;

  enum WaitResponse { WAIT_INTERRUPTED = -1, WAIT_SIGNALED = 0, WAIT_TIMEDOUT = 1 };

  /**
   * This call will wait on a CEvent in an interruptible way such that if
   *  stop is called on the thread the wait will return with a response
   *  indicating what happened.
   */
  inline WaitResponse AbortableWait(CEvent& event, int timeoutMillis = -1 /* indicates wait forever*/)
  {
    XbmcThreads::CEventGroup group(&event, &m_StopEvent, NULL);
    CEvent* result = timeoutMillis < 0 ? group.wait() : group.wait(timeoutMillis);
    return  result == &event ? WAIT_SIGNALED :
      (result == NULL ? WAIT_TIMEDOUT : WAIT_INTERRUPTED);
  }

private:
  static THREADFUNC staticThread(void *data);
  void Action();

  // -----------------------------------------------------------------------------------
  // These are platform specific and can be found in ./platform/[platform]/ThreadImpl.cpp
  // -----------------------------------------------------------------------------------
  ThreadIdentifier ThreadId() const;
  void SetThreadInfo();
  void TermHandler();
  void SetSignalHandlers();
  void SpawnThread(unsigned stacksize);
  // -----------------------------------------------------------------------------------

  ThreadIdentifier m_ThreadId;
  ThreadOpaque m_ThreadOpaque;
  bool m_bAutoDelete;
  CEvent m_StopEvent;
  CEvent m_TermEvent;
  CEvent m_StartEvent;
  CCriticalSection m_CriticalSection;
  IRunnable* m_pRunnable;
  uint64_t m_iLastUsage;
  uint64_t m_iLastTime;
  float m_fLastUsage;

  std::string m_ThreadName;
};
