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


#include <cherryTestingMacros.h>

#include <cherryObject.h>
#include <cherrySmartPointer.h>

class TestObject : public cherry::Object
{
public:
  TestObject() : Object() {}
};


int cherrySmartPointerTest(int /*argc*/, char* /*argv*/[])
{
  CHERRY_TEST_BEGIN("SmartPointer")

  // constructor tests
  {
    // standard constructor
    cherry::SmartPointer<TestObject> testobj;
    CHERRY_TEST_CONDITION(testobj.GetPointer() == 0, "Standard constructor")
  }

  {
    // standard pointer constructor
    cherry::SmartPointer<TestObject> testobj(new TestObject());
    CHERRY_TEST_CONDITION(testobj.GetPointer() != 0 && testobj->GetReferenceCount() == 1, "Pointer constructor")

    // copy constructor
    cherry::SmartPointer<TestObject> testobj2 = testobj;
    CHERRY_TEST_CONDITION(testobj.GetPointer() == testobj2.GetPointer() && testobj2->GetReferenceCount() == 2, "Copy constructor")
  }

  {
    // const_cast template constructor
    cherry::SmartPointer<TestObject> testobj(new TestObject());
    cherry::SmartPointer<cherry::Object> obj(testobj);
    CHERRY_TEST_CONDITION(obj.GetPointer() != 0 && obj->GetReferenceCount() == 2 && typeid(*(obj.GetPointer())) == typeid(TestObject), "Template cast constructor")
  }

  // cast tests
  {
    cherry::SmartPointer<cherry::Object> obj = new TestObject();
    CHERRY_TEST_CONDITION_REQUIRED(obj.GetPointer() != 0 && obj->GetReferenceCount() == 1, "Implicit template cast constructor")
    cherry::SmartPointer<TestObject> testobj = obj.Cast<TestObject>();
    CHERRY_TEST_CONDITION(testobj.GetPointer() != 0 && testobj->GetReferenceCount() == 2 && testobj.GetPointer() == obj.GetPointer(), "Casting")
  }

  // reference count tests
  {
    cherry::SmartPointer<cherry::Object> obj = new TestObject();
    {
      cherry::SmartPointer<cherry::Object> tmpobj = obj;
    }
    CHERRY_TEST_CONDITION(obj->GetReferenceCount() == 1, "Reference counting")
  }

  // operator tests
  {
    cherry::SmartPointer<cherry::Object> obj = new TestObject();
    cherry::SmartPointer<TestObject> testobj = new TestObject();
    cherry::SmartPointer<cherry::Object> nullobj;

    CHERRY_TEST_CONDITION(obj.IsNotNull(), "IsNotNull()")
    CHERRY_TEST_CONDITION(obj.IsNull() == false, "IsNull()")

    // != operators
    CHERRY_TEST_CONDITION(obj != 0, "operator!=(int)")
    CHERRY_TEST_FOR_EXCEPTION_BEGIN(std::invalid_argument)
      obj != 1;
    CHERRY_TEST_FOR_EXCEPTION_END(std::invalid_argument)
    CHERRY_TEST_CONDITION(obj != testobj.GetPointer(), "operator!=(R)")
    CHERRY_TEST_CONDITION(obj != testobj, "operator!=(const cherry::SmartPointer<R>&)")

    // == operators
    CHERRY_TEST_CONDITION((obj == 0) == false, "operator==(int)")
    CHERRY_TEST_FOR_EXCEPTION_BEGIN(std::invalid_argument)
      obj == 1;
    CHERRY_TEST_FOR_EXCEPTION_END(std::invalid_argument)
    CHERRY_TEST_CONDITION((obj == testobj.GetPointer()) == false, "operator==(R)")
    CHERRY_TEST_CONDITION((obj == testobj) == false, "operator==(const cherry::SmartPointer<R>&)")

    CHERRY_TEST_CONDITION(nullobj.CompareTo(testobj) == false, "CompareTo()")
    CHERRY_TEST_CONDITION(obj.CompareTo(testobj) == false, "CompareTo()")
    CHERRY_TEST_CONDITION(testobj.CompareTo(nullobj) == false, "CompareTo()")
  }

  {
    // = operators
    cherry::SmartPointer<cherry::Object> obj = new TestObject();
    cherry::SmartPointer<TestObject> testobj = new TestObject();

    obj = testobj;
    CHERRY_TEST_CONDITION(obj == testobj && obj->GetReferenceCount() == 2, "operator=(const cherry::SmartPointer<R>&)")
    obj = testobj.GetPointer();
    CHERRY_TEST_CONDITION(obj == testobj && obj->GetReferenceCount() == 2, "operator=(Object*)")

  }

  CHERRY_TEST_END()
}
