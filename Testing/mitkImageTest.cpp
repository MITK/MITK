#include "mitkImage.h"

#include <fstream>
int mitkImageTest(int argc, char* argv[])
{
	//Create Image out of nowhere
	mitk::Image::Pointer imgMem;
	mitk::PixelType pt(typeid(int));
	imgMem=mitk::Image::New();
	  unsigned int dim[]={10,10,10};
	  imgMem->Initialize(mitk::PixelType(typeid(int)), 3, dim);

  std::cout << "Testing IsInitialized(): ";
  if(imgMem->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
