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
#include "mitkHistogramMatching.h"

int mitkHistogramMatchingTest(int /*argc*/, char* /*argv*/[])
{
  //Create Image out of nowhere
  mitk::Image::Pointer image;
  mitk::PixelType pt( mitk::MakeScalarPixelType<int>());
  unsigned int dim[]={100,100,20};

  std::cout << "Creating image: ";
  image = mitk::Image::New();
  //image->DebugOn();
  image->Initialize( mitk::MakeScalarPixelType<int>(), 3, dim);
  int *p = (int*)image->GetData();

  int size = dim[0]*dim[1]*dim[2];
  int i;
  for(i=0; i<size; ++i, ++p)
    *p=i;
  std::cout<<"[PASSED]"<<std::endl;

  //Create second Image out of nowhere
  mitk::Image::Pointer image2;
  mitk::PixelType pt2( mitk::MakeScalarPixelType<int>() );
  unsigned int dim2[]={100,100,20};

  std::cout << "Creating image: ";
  image2 = mitk::Image::New();
  //image->DebugOn();
  image2->Initialize(mitk::MakeScalarPixelType<int>(), 3, dim2);
  int *p2 = (int*)image2->GetData();

  int size2 = dim2[0]*dim2[1]*dim2[2];
  int i2;
  for(i2=0; i2<size2; ++i2, ++p2)
    *p2=i2;
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Constructor: ";
  mitk::HistogramMatching::Pointer histogramMatching = mitk::HistogramMatching::New(); 
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Reference Image: ";
  histogramMatching->SetReferenceImage(image);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set Moving Image: ";
  histogramMatching->SetInput(image2);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set number of match points: ";
  histogramMatching->SetNumberOfMatchPoints(100);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set number of histogram levels: ";
  histogramMatching->SetNumberOfHistogramLevels(8);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Set threshold at mean intensity: ";
  histogramMatching->SetThresholdAtMeanIntensity(true);
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Perform histogram matching: ";
  histogramMatching->Update();
  std::cout<<"[PASSED]"<<std::endl;

  std::cout << "Get the result image: ";
  mitk::Image::Pointer histimage = histogramMatching->GetOutput();
  std::cout<<"[PASSED]"<<std::endl;

  return EXIT_SUCCESS;
}
