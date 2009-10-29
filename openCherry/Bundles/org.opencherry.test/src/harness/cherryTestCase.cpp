/*=========================================================================
 
 Program:   openCherry Platform
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

#include "cherryTestCase.h"

#include <cherryConfig.h>
#include <cherryLog.h>

#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
#include <cherryDebugUtil.h>
#endif

cherry::TestCase::TestCase(const std::string& testName) :
  CppUnit::TestCase(testName), m_LeakDetails(false),
  m_IgnoreLeakage(false)
{

}

void cherry::TestCase::LeakDetailsOn()
{
  m_LeakDetails = true;
}

void cherry::TestCase::IgnoreLeakingObjects()
{
  CHERRY_WARN << "Ignoring Leaking Objects!!";
  m_IgnoreLeakage = true;
}

void cherry::TestCase::DoSetUp()
{

}

void cherry::TestCase::DoTearDown()
{

}

void cherry::TestCase::setUp()
{
  CppUnit::TestCase::setUp();
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  DebugUtil::ResetObjectSummary();
#endif
  DoSetUp();
}

void cherry::TestCase::tearDown()
{
  CppUnit::TestCase::tearDown();
  DoTearDown();
#ifdef OPENCHERRY_DEBUG_SMARTPOINTER
  assert(m_IgnoreLeakage || !DebugUtil::PrintObjectSummary(m_LeakDetails));
#endif

  m_LeakDetails = false;
}

