#include "mitkContour.h"

#include <fstream>
int mitkContourTest(int argc, char* argv[])
{
  mitk::Contour::Pointer contour;
  std::cout << "Testing mitk::Contour::New(): ";
  contour = mitk::Contour::New();
  if (contour.IsNull()) {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  } 
  

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
