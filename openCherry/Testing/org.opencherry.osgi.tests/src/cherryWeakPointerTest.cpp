/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision: 15805 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include <cherryMessage.h>
#include <cherryObject.h>
#include <cherrySmartPointer.h>
#include <cherryWeakPointer.h>
//#include <cstdlib>

#include "cherryWeakPointerTest.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

namespace cherry
{

struct TestObject: public Object
{

  /** creating a test object in order to test the weakpointer functions */

  TestObject()
  { }

};

WeakPointerTest::WeakPointerTest(const std::string& testName) :
  CppUnit::TestCase(testName)
{

}

CppUnit::Test* WeakPointerTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("WeakPointerTest");

  CppUnit_addTest(suite, WeakPointerTest, TestReferenceCounting);
  CppUnit_addTest(suite, WeakPointerTest, TestConstructorAndLock);
  CppUnit_addTest(suite, WeakPointerTest, TestOperators);

  return suite;
}

void WeakPointerTest::TestReferenceCounting()
{

  SmartPointer<TestObject> ptr_smart(new TestObject());

  WeakPointer<TestObject> ptr_weak(ptr_smart);
  assertEqual(1, ptr_smart->GetReferenceCount());

  /* Testing the Object reference counter after returning a smartpointer
   *  to an object, Reference counter should be increased to 2 */
  SmartPointer<TestObject> ptr_smart2(ptr_weak);
  assertEqual(2, ptr_smart->GetReferenceCount());
}

void WeakPointerTest::TestConstructorAndLock()
{
  SmartPointer<TestObject>* ptr_smart = new SmartPointer<TestObject> (
      new TestObject);
  WeakPointer<TestObject> ptr_weak(*ptr_smart);
  WeakPointer<TestObject> ptr_weak2(ptr_weak);
  {
    SmartPointer<TestObject> ptr_smart1(ptr_weak);
    SmartPointer<TestObject> ptr_smart2(ptr_weak2);
    assertEqual(ptr_smart1.GetPointer(), ptr_smart2.GetPointer());
  }
  assertEqual(1, (*ptr_smart)->GetReferenceCount());
  delete ptr_smart;
  assert(ptr_weak2.Lock() == 0);
}

void WeakPointerTest::TestOperators()
{
  SmartPointer<TestObject> ptr_smart(new TestObject);
  SmartPointer<TestObject> ptr_smart2(new TestObject);
  {
    WeakPointer<TestObject> ptr_weak(ptr_smart);
    WeakPointer<TestObject> ptr_weak2(ptr_smart2);
    ptr_weak = ptr_weak2;
    SmartPointer<TestObject> ptr_smart3(ptr_weak2);
    SmartPointer<TestObject> ptr_smart4(ptr_weak);
    assertEqual(ptr_smart3.GetPointer(), ptr_smart4.GetPointer());
  }
}

}
