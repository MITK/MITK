/*=========================================================================
 
 Program:   BlueBerry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "berryTestCase.h"

#include <berryConfig.h>
#include <berryLog.h>

#ifdef BLUEBERRY_DEBUG_SMARTPOINTER
#include <berryDebugUtil.h>
#endif

berry::TestCase::TestCase(const std::string& testName) :
  CppUnit::TestCase(testName), m_LeakDetails(false),
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

