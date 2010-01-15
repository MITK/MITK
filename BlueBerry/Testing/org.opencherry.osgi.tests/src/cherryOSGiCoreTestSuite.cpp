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


#include <CppUnit/TestCase.h>
#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

#include "cherryOSGiCoreTestSuite.h"

#include "cherryObjectTest.h"
#include "cherrySmartPointerTest.h"
#include "cherryWeakPointerTest.h"
#include "cherryMessageTest.h"

namespace cherry {

OSGiCoreTestSuite::OSGiCoreTestSuite()
: CppUnit::TestSuite("OSGiCoreTestSuite")
{
  addTest(ObjectTest::Suite());
  addTest(SmartPointerTest::Suite());
  addTest(WeakPointerTest::Suite());
  addTest(MessageTest::Suite());
}

}
