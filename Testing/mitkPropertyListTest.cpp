#include "mitkPropertyList.h"

#include <fstream>
int mitkPropertyListTest(int argc, char* argv[])
{
  mitk::PropertyList::Pointer propList;

  std::cout << "Testing New(): ";
  propList = mitk::PropertyList::New();
  if (propList.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
