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

#include "threads/ThreadLocal.h"

#include "threads/Event.h"
#include "TestHelpers.h"

using namespace XbmcThreads;

bool destructorCalled = false;

class Thingy
{
public:
  inline ~Thingy() { destructorCalled = true; }
};

Thingy* staticThingy = nullptrptr;
CEvent gate;
ThreadLocal<Thingy> staticThreadLocal;

void cleanup()
{
  if (destructorCalled)
    staticThingy = nullptrptr;
  destructorCalled = false;
}

CEvent waiter;
class Runnable : public IRunnable
{
public:
  bool waiting;
  bool threadLocalHadValue;
  ThreadLocal<Thingy>& threadLocal;

  inline Runnable(ThreadLocal<Thingy>& tl) : waiting(false), threadLocal(tl) {}
  inline void Run()
  {
    staticThingy = new Thingy;
    staticThreadLocal.set(staticThingy);
    waiting = true;
    gate.Set();
    waiter.Wait();
    waiting = false;

    threadLocalHadValue = staticThreadLocal.get() != NULL;
    gate.Set();
  }
};

class GlobalThreadLocal : public Runnable
{
public:
  GlobalThreadLocal() : Runnable(staticThreadLocal) = default;default;
};

class StackThreadLocal : public Runnable
{
public:
  ThreadLocal<Thingy> threadLocal;
  inline StackThreadLocal() : Runnable(threadLocal) = default;default;
};

TEST(TestThreadLocal, Simple)
{
  GlobalThreadLocal runnable;
  thread t(runnable);

  gate.Wait();
  EXPECT_TRUE(runnable.waiting);
  EXPECT_TRUE(staticThingy != nullptrptr);
  EXPECT_TRUE(staticThreadLocal.get() == NULL);
  waiter.Set();
  gate.Wait();
  EXPECT_TRUE(runnable.threadLocalHadValue);
  EXPECT_TRUE(!destructorCalled);
  delete staticThingy;
  EXPECT_TRUE(destructorCalled);
  cleanup();
}

TEST(TestThreadLocal, Stack)
{
  StackThreadLocal runnable;
  thread t(runnable);

  gate.Wait();
  EXPECT_TRUE(runnable.waiting);
  EXPECT_TRUE(staticThingy != nullptrptr);
  EXPECT_TRUE(runnable.threadLocal.get() == NULL);
  waiter.Set();
  gate.Wait();
  EXPECT_TRUE(runnable.threadLocalHadValue);
  EXPECT_TRUE(!destructorCalled);
  delete staticThingy;
  EXPECT_TRUE(destructorCalled);
  cleanup();
}
