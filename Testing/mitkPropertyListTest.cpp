#include "mitkPropertyList.h"
#include "mitkProperties.h"

#include <fstream>
int mitkPropertyListTest(int argc, char* argv[])
{
  mitk::PropertyList::Pointer propList;
  std::cout << "Testing mitk::PropertyList::New(): ";
  propList = mitk::PropertyList::New();
  if (propList.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 
  mitk::BoolProperty::Pointer boolProp = new mitk::BoolProperty(false);
  mitk::BoolProperty::Pointer boolProp2 = new mitk::BoolProperty(false);
  std::cout << "Testing BoolProperty ==: ";
  if (! (*boolProp2 == *boolProp) ) {
    
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
    } 
  std::cout<<"[PASSED]"<<std::endl;
  unsigned long tBefore,tAfter; 
  
  std::cout << "Testing SetProperty() with new key value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",boolProp); 
  tAfter = propList->GetMTime();
  if (! ( tAfter > tBefore) ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing SetProperty() with new property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",new mitk::BoolProperty(true)); 
  tAfter = propList->GetMTime();
  if (!  (tAfter > tBefore) ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing SetProperty() with unchanged property value: ";
  tBefore = propList->GetMTime();
  propList->SetProperty("test",new mitk::BoolProperty(true)); 
  tAfter = propList->GetMTime();
  if ( tBefore == tAfter ) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing MTime correctness when changing property value: ";
  boolProp = new mitk::BoolProperty(true);
  propList->SetProperty("test",boolProp); 
  std::cout << "prop->MTIme() before: >" << boolProp->GetMTime() << "< ";
  std::cout << (tBefore = propList->GetMTime());
  boolProp->SetValue(true);
  std::cout << "prop->MTIme() after: >" << boolProp->GetMTime() << "< ";
  std::cout << (tAfter = propList->GetMTime());
  boolProp->SetValue(false);
  std::cout << "prop->MTIme() afterAll: >" << boolProp->GetMTime() << "< ";
  unsigned long tAfterAll = propList->GetMTime();
  
  
  if (tBefore != tAfter || tAfterAll <= tAfter) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
