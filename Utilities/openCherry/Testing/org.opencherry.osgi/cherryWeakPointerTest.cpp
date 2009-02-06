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

#include <cherryTestingMacros.h>
#include <cherryMessage.h>
#include <cherryObject.h>
#include <cherrySmartPointer.h>
#include <cherryWeakPointer.h>

struct ObjectTest: public cherry::Object
{

  /** creating a test object in order to test the weakpointer functions */

  ObjectTest() :
    Object()
  {
  }
  void TestWeakpointer()
  {

  }

};

int cherryWeakPointerTest(int /*argc*/, char* /*argv*/[])
{

  CHERRY_TEST_BEGIN("Weakpointer")

    // test reference counting
    {

      cherry::SmartPointer<ObjectTest> ptr_smart(new ObjectTest);

      cherry::WeakPointer<ObjectTest> ptr_weak(ptr_smart);
      CHERRY_TEST_CONDITION( ptr_smart->GetReferenceCount() == 1,"no reference counting");

      /* Testing the Object reference counter after returning a smartpointer
       *  to an object, Reference counter should be increased to 2 */
      cherry::SmartPointer<ObjectTest> ptr_smart2(ptr_weak);
      CHERRY_TEST_CONDITION( ptr_smart->GetReferenceCount() == 2,"increased reference count");
    }

    // test copy constructor and WeakPointer::Lock() method
    {
      cherry::SmartPointer<ObjectTest>* ptr_smart =
          new cherry::SmartPointer<ObjectTest>(new ObjectTest);
      cherry::WeakPointer<ObjectTest> ptr_weak(*ptr_smart);
      cherry::WeakPointer<ObjectTest> ptr_weak2(ptr_weak);
      {
        cherry::SmartPointer<ObjectTest> ptr_smart1(ptr_weak);
        cherry::SmartPointer<ObjectTest> ptr_smart2(ptr_weak2);
        CHERRY_TEST_CONDITION(ptr_smart1.GetPointer() == ptr_smart2.GetPointer(),"copy constructor");
      }
      CHERRY_TEST_CONDITION((*ptr_smart)->GetReferenceCount() == 1, "refcount = 1";)
      delete ptr_smart;
      CHERRY_TEST_CONDITION(ptr_weak2.Lock() == 0, "WeakPointer::Lock()");
    }

    // test operator=(const WeakPointer&)
    {
      cherry::SmartPointer<ObjectTest> ptr_smart(new ObjectTest);
      cherry::SmartPointer<ObjectTest> ptr_smart2(new ObjectTest);
      {
        cherry::WeakPointer<ObjectTest> ptr_weak(ptr_smart);
        cherry::WeakPointer<ObjectTest> ptr_weak2(ptr_smart2);
        ptr_weak = ptr_weak2;
        cherry::SmartPointer<ObjectTest> ptr_smart3(ptr_weak2);
        cherry::SmartPointer<ObjectTest> ptr_smart4(ptr_weak);
        CHERRY_TEST_CONDITION(ptr_smart3.GetPointer() == ptr_smart4.GetPointer(),"assignment operator");
      }

    }

  CHERRY_TEST_END()

}

