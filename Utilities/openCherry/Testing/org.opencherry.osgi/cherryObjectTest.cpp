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


/**
* just used for adding Functions to the Object DestroyerListener 
*/
struct RegisterMe{
  
  RegisterMe () : registered(false){
  
  }

  bool registered ; 
    
  void Register (){
    registered = true ; 
   }
  void UnRegister (){
    registered = false ; 
   }
  void Output (){
    if ( registered == false ) {
      std::cout << "  nothing happened yet  " << std::endl;
    }
    else {
      std::cout << " Function Register had been called " << registered << std::endl ; 
    }
  }
  bool getRegisteredStatus() const {
    return registered; 
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
  
  

  
  /** 
  * Functiontest AddDestroyListener (...)
  *
  */
  TestObject* AddDestroyListenerTest = new TestObject();

  
  RegisterMe testRegister; 
  
  AddDestroyListenerTest->AddDestroyListener(cherry::MessageDelegate<RegisterMe>(&testRegister, &RegisterMe::Register));
  /** Ouput should be " nothing happened yet " because the underlying object isn't destroyed and therefore no functions are called by the DestroyListener */
  AddDestroyListenerTest->Delete();
  /** testobject is deleted, DestroyListener should have called the registered Functions */ 
  
  CHERRY_TEST_CONDITION(testRegister.getRegisteredStatus()," AddDestroyListenerTest ") ; 


  /** 
  * Functiontest RemoveDestroyListener (...)
  *
  */

  TestObject* RemoveDestroyListenerTest = new TestObject();

  
  RegisterMe testRegister2; 

  /** Functiontest AddDestroyListener */
  RemoveDestroyListenerTest->AddDestroyListener(cherry::MessageDelegate<RegisterMe>(&testRegister2, &RegisterMe::Register));
  /** Ouput should be " nothing happened yet " because the underlying object isn't destroyed and therefore no functions are called by the DestroyListener */
  RemoveDestroyListenerTest->RemoveDestroyListener(cherry::MessageDelegate<RegisterMe>(&testRegister2, &RegisterMe::Register));
  
  
  RemoveDestroyListenerTest->Delete();
  /** testobject is deleted, DestroyListener should have called the registered Functions */ 
  
  
 
  CHERRY_TEST_CONDITION(testRegister2.getRegisteredStatus() == false," RemoveDestroyListenerTest ") ; 
 
  CHERRY_TEST_END()
  
  
}
