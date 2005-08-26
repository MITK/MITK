
#include "mitkManualSegmentationToSurfaceFilter.h"

#include <fstream>

int mitkManualSegmentationToSurfaceFilterTest(int argc, char* argv[])
{
  mitk::ManualSegmentationToSurfaceFilter::Pointer filter;
  std::cout << "Testing mitk::ManualSegmentationToSurfaceFilter::New(): ";
  filter = mitk::ManualSegmentationToSurfaceFilter::New();
  if (filter.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  else {
  std::cout<<"[PASSED]"<<std::endl;
  }

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}