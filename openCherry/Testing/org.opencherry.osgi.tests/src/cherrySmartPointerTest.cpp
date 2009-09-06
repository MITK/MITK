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


#include "cherrySmartPointerTest.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

#include <cherryObject.h>
#include <cherrySmartPointer.h>
#include <cherryWeakPointer.h>

#include <typeinfo>
//#include <cstdlib>

namespace cherry
{

class TestObject : public cherry::Object
{
public:
  TestObject() : Object(), m_Val(0) {}
  TestObject(int value) : m_Val(value) {}

  bool operator==(const Object* o) const
  {
    if (const TestObject* to = dynamic_cast<const TestObject*>(o))
      return m_Val == to->m_Val;

    return false;
  }

private:

  int m_Val;
};


SmartPointerTest::SmartPointerTest(const std::string& testName)
: CppUnit::TestCase(testName)
{}

CppUnit::Test* SmartPointerTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("SmartPointerTest");

  CppUnit_addTest(suite, SmartPointerTest, TestConstructors);
  CppUnit_addTest(suite, SmartPointerTest, TestCasting);
  CppUnit_addTest(suite, SmartPointerTest, TestReferenceCounting);
  CppUnit_addTest(suite, SmartPointerTest, TestOperators);

  return suite;
}


 void SmartPointerTest::TestConstructors()
 {
  {
    // standard constructor
    cherry::SmartPointer<TestObject> testobj;
    assertNullPtr(testobj.GetPointer());
  }


  {
    // standard pointer constructor
    cherry::SmartPointer<TestObject> testobj(new TestObject());
    assertNotNullPtr(testobj.GetPointer());
    assertEqual(1, testobj->GetReferenceCount());

    // copy constructor
    cherry::SmartPointer<TestObject> testobj2 = testobj;
    assertEqual(testobj.GetPointer(), testobj2.GetPointer());
    assertEqual(2, testobj2->GetReferenceCount());
  }

  {
    // WeakPointer constructor
    cherry::WeakPointer<TestObject> ptr_weak;
    try {
      cherry::SmartPointer<TestObject> testobj(ptr_weak);
      failmsg("this should throw a BadWeakPointerException");
    }
    catch (BadWeakPointerException )
    {
      // expected
    }
  }

  {
    // const_cast template constructor
    cherry::SmartPointer<TestObject> testobj(new TestObject());
    cherry::SmartPointer<cherry::Object> obj(testobj);
    assertNotNullPtr(obj.GetPointer());
    assertEqual(2, obj->GetReferenceCount());
    assert(typeid(*(obj.GetPointer())) == typeid(TestObject));
  }
 }

 void SmartPointerTest::TestCasting()
  // cast tests
  {
    cherry::SmartPointer<cherry::Object> obj(new TestObject());
    assertNotNullPtr(obj.GetPointer());
    assertEqual(1, obj->GetReferenceCount());

    cherry::SmartPointer<TestObject> testobj = obj.Cast<TestObject>();
    assertNotNullPtr(testobj.GetPointer());
    assertEqual(2, testobj->GetReferenceCount());
    assertEqual(testobj.GetPointer(), obj.GetPointer());
  }

 void SmartPointerTest::TestReferenceCounting()
  // reference count tests
  {
    cherry::SmartPointer<cherry::Object> obj(new TestObject());
    {
      cherry::SmartPointer<cherry::Object> tmpobj = obj;
    }
    assertEqual(1, obj->GetReferenceCount());
  }

 void SmartPointerTest::TestOperators()
 {
  // operator tests
  {
    cherry::SmartPointer<cherry::Object> obj(new TestObject(1));
    cherry::SmartPointer<TestObject> testobj(new TestObject(2));
    cherry::SmartPointer<cherry::Object> nullobj;

    // boolean conversions
    assert(obj.IsNotNull());
    assert(false == obj.IsNull());
    assert(obj);
    assert(false == !obj);

    // != operators
    assert(obj != 0);

    try
    {
      obj != 1;
      failmsg("should throw a std::invalid_argument exception");
    }
    catch (std::invalid_argument)
    { //expectted }

    }

    // operator!=(R)
    assert(obj != testobj.GetPointer());

    // operator!=(const cherry::SmartPointer<R>&)
    assert(obj != testobj);

    // == operators
    // operator==(int)
    assert((obj == 0) == false);
    try {
      obj == 1;
      failmsg("should throw a std::invalid_argument exception");
    }
    catch (std::invalid_argument)
    { // expected }

    }
    // operator==(R)
    assert((obj == testobj.GetPointer()) == false);
    // operator==(R)
    assert(obj == obj.GetPointer());
    // operator==(R)
    assert((obj == nullobj.GetPointer()) == false);
    // operator==(const cherry::SmartPointer<R>&)
    assert((obj == testobj) == false);
    // operator==(const cherry::SmartPointer<R>&)
    assert(obj == obj);
    // operator==(const cherry::SmartPointer<R>&)
    assert((obj == nullobj) == false);
  }

  {
    // = operators
    cherry::SmartPointer<cherry::Object> obj(new TestObject(1));
    cherry::SmartPointer<TestObject> testobj(new TestObject(2));

    obj = testobj;
    // operator=(const cherry::SmartPointer<R>&)
    assert(obj == testobj);
    assertEqual(2, obj->GetReferenceCount());

    // operator=(Object*)
    obj = testobj.GetPointer();
    assert(obj == testobj);
    assertEqual(2, obj->GetReferenceCount());
  }

}

}
