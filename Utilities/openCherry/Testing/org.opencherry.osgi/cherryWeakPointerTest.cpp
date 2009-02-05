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

struct ObjectTest : public cherry::Object {

  

  /** creating a test object in order to test the weakpointer functions */

  ObjectTest() : Object() {}
  void TestWeakpointer (){
    
  }

}; 

int cherryWeakPointerTest(int /*argc*/, char* /*argv*/[]){
  

  CHERRY_TEST_BEGIN("Weakpointer")

  {
    
    cherry::SmartPointer<ObjectTest> ptr_smart(new ObjectTest);

    cherry::WeakPointer<ObjectTest> ptr_weak (ptr_smart);
    CHERRY_TEST_CONDITION( ptr_smart->GetReferenceCount() == 1,"Object Referencecounter");

    /** Testing the Object reference counter after returning a smartpointer to an object, Reference counter should be increased to 2 */
    cherry::SmartPointer<ObjectTest> ptr_smart2(ptr_weak.lock());
    CHERRY_TEST_CONDITION( ptr_smart->GetReferenceCount() == 2,"Object Returning smartpointer by using a weakpointer to testobject");
  }

  {

    cherry::WeakPointer<ObjectTest> ptr_weak;
    {
      cherry::SmartPointer<ObjectTest> ptr_smart(new ObjectTest);
      ptr_weak = ptr_smart;
    }
  
  

    /** Testing if the Object reference counter had been increased after creating another smartpointer to the referred object
     *  Deleting refered object and exeption testing
     */
    CHERRY_TEST_FOR_EXCEPTION_BEGIN(Poco::NullPointerException)
    ptr_weak.lock();
    CHERRY_TEST_FOR_EXCEPTION_END(Poco::NullPointerException)
  }
  {
    cherry::SmartPointer<ObjectTest>* ptr_smart = new cherry::SmartPointer<ObjectTest>(new ObjectTest);
    cherry::WeakPointer<ObjectTest> ptr_weak(*ptr_smart);
    cherry::WeakPointer<ObjectTest> ptr_weak2(ptr_weak);
    {
      cherry::SmartPointer<ObjectTest> ptr_smart1(ptr_weak.lock());
      cherry::SmartPointer<ObjectTest> ptr_smart2(ptr_weak2.lock());
      CHERRY_TEST_CONDITION(ptr_smart1.GetPointer() == ptr_smart2.GetPointer(),"Test CopyConstructor(m_Pointer)");
    }
    delete ptr_smart ;
    CHERRY_TEST_FOR_EXCEPTION_BEGIN(Poco::NullPointerException)
    ptr_weak2.lock();
    CHERRY_TEST_FOR_EXCEPTION_END(Poco::NullPointerException)
  }
  {
    cherry::SmartPointer<ObjectTest> ptr_smart(new ObjectTest); 
    cherry::SmartPointer<ObjectTest> ptr_smart2(new ObjectTest); 
    {
    cherry::WeakPointer<ObjectTest> ptr_weak (ptr_smart);
    cherry::WeakPointer<ObjectTest> ptr_weak2 (ptr_smart2);
    ptr_weak = ptr_weak2 ; 
    cherry::SmartPointer<ObjectTest>ptr_smart3(ptr_weak2.lock());
    cherry::SmartPointer<ObjectTest>ptr_smart4(ptr_weak.lock());
    CHERRY_TEST_CONDITION(ptr_smart3.GetPointer() == ptr_smart4.GetPointer(),"Test AssignmentOperator");
    
    }

  }
 

  CHERRY_TEST_END()

   

}


