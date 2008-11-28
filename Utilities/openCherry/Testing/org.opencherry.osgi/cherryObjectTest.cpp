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

class TestObject : public cherry::Object
{
public:
  TestObject() : Object() {}
};

int cherryObjectTest(int /*argc*/, char* /*argv*/[])
{
  CHERRY_TEST_BEGIN("Object")

  // reference count tests
  {
    TestObject* obj = new TestObject();
    CHERRY_TEST_CONDITION(obj->GetReferenceCount() == 0, "Initial ref count")

    obj->Register();
    CHERRY_TEST_CONDITION(obj->GetReferenceCount() == 1, "Increasing ref count")

    obj->SetReferenceCount(3);
    CHERRY_TEST_CONDITION(obj->GetReferenceCount() == 3, "Setting ref count")

    obj->SetReferenceCount(0);
  }

  // destroy listener tests
  {

  }

  CHERRY_TEST_END()
}
