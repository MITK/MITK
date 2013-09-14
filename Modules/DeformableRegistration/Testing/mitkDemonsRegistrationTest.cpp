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
#include "mitkDemonsRegistration.h"
#include "mitkImageWriteAccessor.h"


int mitkDemonsRegistrationTest(int /*argc*/, char* /*argv*/[])
{
  //Create Image out of nowhere
  mitk::Image::Pointer image;
  mitk::PixelType pt(mitk::MakeScalarPixelType<int>());
  unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
  image = mitk::Image::New();
  //image->DebugOn();
  image->Initialize(mitk::MakeScalarPixelType<int>(), 3, dim);


  try
  {
    mitk::ImageWriteAccessor imgB(image);
    int* p = (int*) imgB.GetData();

    int size = dim[0]*dim[1]*dim[2];
    int i;
    for(i=0; i<size; ++i, ++p)
      *p=i;
    std::cout<<"[PASSED]"<<std::endl;
  }
  catch(mitk::Exception& e)
  {
    // we don't have image access, set test to failed
    std::cout<<"[FAILED] creation of the image" <<std::endl;
    return EXIT_FAILURE;
  }


  //Create second Image out of nowhere
  mitk::Image::Pointer image2;
  mitk::PixelType pt2(mitk::MakeScalarPixelType<int>());
  unsigned int dim2[]={100,100,20};

  std::cout << "Creating image: ";
  image2 = mitk::Image::New();
  //image->DebugOn();
  image2->Initialize(mitk::MakeScalarPixelType<int>(), 3, dim2);

  try
  {
    mitk::ImageWriteAccessor imgB(image2);
    int* p2 = (int*) imgB.GetData();

    int size2 = dim2[0]*dim2[1]*dim2[2];
    int i2;
    for(i2=0; i2<size2; ++i2, ++p2)
      *p2=i2;
    std::cout<<"[PASSED]"<<std::endl;
  }
  catch(mitk::Exception& e)
  {
    // we don't have image access, set test to failed
    std::cout<<"[FAILED] creation of the image2" <<std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "Constructor: ";
  mitk::DemonsRegistration::Pointer demonsRegistration = mitk::DemonsRegistration::New();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Reference Image: ";
  demonsRegistration->SetReferenceImage(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Moving Image: ";
  demonsRegistration->SetInput(image2);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set number of iterations: ";
  demonsRegistration->SetNumberOfIterations(5);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set standard deviation: ";
  demonsRegistration->SetStandardDeviation(1.0);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set save deformation field: ";
  demonsRegistration->SetSaveDeformationField(false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set deformation field file name: ";
  demonsRegistration->SetDeformationFieldFileName("TestField.mhd");
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set save result image: ";
  demonsRegistration->SetSaveResult(false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set result image file name: ";
  demonsRegistration->SetResultFileName("TestResult.mhd");
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Perform registration: ";
  demonsRegistration->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Get the result image: ";
  mitk::Image::Pointer resultImage = demonsRegistration->GetOutput();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Get the deformation field: ";
  itk::Image<class itk::Vector<float, 3>,3>::Pointer deformationField = demonsRegistration->GetDeformationField();
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}
