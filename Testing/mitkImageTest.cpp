/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkImage.h"

#include <fstream>
int mitkImageTest(int argc, char* argv[])
{
	//Create Image out of nowhere
	mitk::Image::Pointer imgMem;
	mitk::PixelType pt(typeid(int));
	unsigned int dim[]={100,100,20};

  std::cout << "Testing creation of Image: ";
	imgMem=mitk::Image::New();
  if(imgMem.IsNull())
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing Initialize(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions): ";
	imgMem->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IsInitialized(): ";
  if(imgMem->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing GetData(): ";
  int *p = (int*)imgMem->GetData();
  if(p==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Filling image: ";
  int i;
  int size = dim[0]*dim[1]*dim[2];
  for(i=0; i<size; ++i, ++p)
    *p=i;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Getting it again and compare with filled values: ";
  int *p2 = (int*)imgMem->GetData();
  if(p2==NULL)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  for(i=0; i<size; ++i, ++p2)
  {
    if(*p2!=i)
    {
      std::cout<<"[FAILED]"<<std::endl;
      return EXIT_FAILURE;
    }
  }
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Testing IsInitialized(): ";
  if(imgMem->IsInitialized()==false)
  {
    std::cout<<"[FAILED]"<<std::endl;
    return EXIT_FAILURE;
  }
  std::cout<<"[PASSED]"<<std::endl;
  
  std::cout << "Testing GetSliceData(): ";
  
  

  //int *data = new int[size];
  //std::cout << "Testing SetVolume(): ";
  //imgMem->SetVolume(data);
  //std::cout<<"[PASSED]"<<std::endl;


  std::cout<<"[TEST DONE]"<<std::endl;
  return EXIT_SUCCESS;
}
