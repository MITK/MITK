/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkImage.h"
#include "mitkRegistrationBase.h"

int mitkRegistrationBaseTest(int /*argc*/, char* /*argv*/[])
{
  //Create Image out of nowhere
  mitk::Image::Pointer image;
  mitk::PixelType pt(typeid(int));
  unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
  image = mitk::Image::New();
  //image->DebugOn();
  image->Initialize(mitk::PixelType(typeid(int)), 3, dim);
  int *p = (int*)image->GetData();

  int size = dim[0]*dim[1]*dim[2];
  int i;
  for(i=0; i<size; ++i, ++p)
    *p=i;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Constructor: ";
  mitk::RegistrationBase::Pointer registrationBase = mitk::RegistrationBase::New();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Reference Image: ";
  registrationBase->SetReferenceImage(image);
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}
