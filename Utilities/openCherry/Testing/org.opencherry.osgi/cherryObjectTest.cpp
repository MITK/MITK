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
#include <cstdlib>

class TestObject: public cherry::Object
{
public:
  TestObject() :
    Object()
  {
  }
};

/**
 * just used for adding Functions to the Object DestroyerListener
 */
struct ObjectListener
{
  bool m_ObjectDeleted;

  ObjectListener() :
    m_ObjectDeleted(false)
  {

  }

  void DestroyListener()
  {
    m_ObjectDeleted = true;
  }

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

  {
    /**
     * Functiontest AddDestroyListener (...)
     *
     */
    TestObject* obj = new TestObject();

    ObjectListener objectListener;

    obj->AddDestroyListener(
        cherry::MessageDelegate<ObjectListener>(&objectListener, &ObjectListener::DestroyListener));
    delete obj;
    CHERRY_TEST_CONDITION(objectListener.m_ObjectDeleted, "Object::AddDestroyListener()");
  }

  {
    /**
     * Functiontest RemoveDestroyListener (...)
     *
     */
    TestObject* obj = new TestObject();

    ObjectListener objectListener;

    obj->AddDestroyListener(
        cherry::MessageDelegate<ObjectListener>(&objectListener, &ObjectListener::DestroyListener));
    obj->RemoveDestroyListener(
        cherry::MessageDelegate<ObjectListener>(&objectListener, &ObjectListener::DestroyListener));
    delete obj;
    CHERRY_TEST_CONDITION(objectListener.m_ObjectDeleted == false, "Object::RemoveDestroyListener()");
  }

  CHERRY_TEST_END()

}
