/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "berryTestCase.h"

#include <berryConfig.h>
#include <berryLog.h>

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
#include <berryDebugUtil.h>
#endif

#include <QString>

#include <cassert>

berry::TestCase::TestCase(const QString& testName) :
  CppUnit::TestCase(testName.toStdString()), m_LeakDetails(false),
  m_IgnoreLeakage(false)
{

}

void berry::TestCase::LeakDetailsOn()
{
  m_LeakDetails = true;
}

void berry::TestCase::IgnoreLeakingObjects()
{
  BERRY_WARN << "Ignoring Leaking Objects!!";
  m_IgnoreLeakage = true;
}

void berry::TestCase::DoSetUp()
{

}

void berry::TestCase::DoTearDown()
{

}

void berry::TestCase::setUp()
{
  CppUnit::TestCase::setUp();
#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  DebugUtil::ResetObjectSummary();
#endif
  DoSetUp();
}

void berry::TestCase::tearDown()
{
  CppUnit::TestCase::tearDown();
  DoTearDown();
#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
  assert(m_IgnoreLeakage || !DebugUtil::PrintObjectSummary(m_LeakDetails));
#endif

  m_LeakDetails = false;
}

