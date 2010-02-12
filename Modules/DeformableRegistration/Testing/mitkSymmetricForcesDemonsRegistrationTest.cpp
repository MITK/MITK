/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 15228 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkImage.h"
#include "mitkSymmetricForcesDemonsRegistration.h"

int mitkSymmetricForcesDemonsRegistrationTest(int /*argc*/, char* /*argv*/[])
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

  //Create second Image out of nowhere
  mitk::Image::Pointer image2;
  mitk::PixelType pt2(typeid(int));
  unsigned int dim2[]={100,100,20};

  std::cout << "Creating image: ";
  image2 = mitk::Image::New();
  //image->DebugOn();
  image2->Initialize(mitk::PixelType(typeid(int)), 3, dim2);
  int *p2 = (int*)image2->GetData();

  int size2 = dim2[0]*dim2[1]*dim2[2];
  int i2;
  for(i2=0; i2<size2; ++i2, ++p2)
    *p2=i2;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Constructor: ";
  mitk::SymmetricForcesDemonsRegistration::Pointer symmetricForcesDemonsRegistration = mitk::SymmetricForcesDemonsRegistration::New(); 
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Reference Image: ";
  symmetricForcesDemonsRegistration->SetReferenceImage(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Moving Image: ";
  symmetricForcesDemonsRegistration->SetInput(image2);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set number of iterations: ";
  symmetricForcesDemonsRegistration->SetNumberOfIterations(5);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set standard deviation: ";
  symmetricForcesDemonsRegistration->SetStandardDeviation(1.0);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set save deformation field: ";
  symmetricForcesDemonsRegistration->SetSaveDeformationField(false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set deformation field file name: ";
  symmetricForcesDemonsRegistration->SetDeformationFieldFileName("TestField.mhd");
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set save result image: ";
  symmetricForcesDemonsRegistration->SetSaveResult(false);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set result image file name: ";
  symmetricForcesDemonsRegistration->SetResultFileName("TestResult.mhd");
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Perform registration: ";
  symmetricForcesDemonsRegistration->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Get the result image: ";
  mitk::Image::Pointer resultImage = symmetricForcesDemonsRegistration->GetOutput();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Get the deformation field: ";
  itk::Image<class itk::Vector<float, 3>,3>::Pointer deformationField = symmetricForcesDemonsRegistration->GetDeformationField();
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}