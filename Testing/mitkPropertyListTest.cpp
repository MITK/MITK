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
  std::cout<<"[PASSED]"<<std::endl;
  
  mitk::BoolProperty::Pointer boolProp = new mitk::BoolProperty(false);

  std::cout << "Testing SetProperty(): ";
  bool propChanged = propList->SetProperty("test",boolProp); 
  if (!propChanged) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  boolProp->SetValue(true);
  propChanged = propList->SetProperty("test",boolProp); 
  if (!propChanged) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  propChanged = propList->SetProperty("test",boolProp); 
  if (propChanged) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  
  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
