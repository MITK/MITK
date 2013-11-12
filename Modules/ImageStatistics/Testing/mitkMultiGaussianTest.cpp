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

#include "mitkTestingMacros.h"
#include "itkMultiGaussianImageSource.h"

#include <iostream>
#include <fstream>

int mitkMultiGaussianTest(int, char* [])
{
  // always start with this!
  MITK_TEST_BEGIN("mitkMultiGaussianTest")

  typedef double                                        PixelType;
  const unsigned int Dimension = 3;
  typedef itk::Image<PixelType, Dimension>              ImageType;
  typedef itk::MultiGaussianImageSource< ImageType >    MultiGaussianImageSource;
  itk::MultiGaussianImageSource< ImageType >::VectorType centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec;
  MultiGaussianImageSource::Pointer gaussianGenerator         = MultiGaussianImageSource::New();
  ImageType::SizeValueType size[3];
  size[0] = 50;
  size[1] = 50;
  size[2] = 50;
  srand (time(NULL));
  unsigned int numberOfGaussians = 7;
  unsigned int minWidthOfGaussian = (size[0] +  size[1] + size[2]) / 27;     // A ninth of the mean image size
  unsigned int maxWidthOfGaussian = (size[0] +  size[1] + size[2]) / 9;     // One-third of the mean image size
  unsigned int minAltitudeOfGaussian = 5;
  unsigned int maxAltitudeOfGaussian = 200;
  double centerX, centerY, centerZ, sigmaX, sigmaY, sigmaZ, altitude;

  gaussianGenerator->SetSize( size );
  gaussianGenerator->SetSpacing( 1 );
  gaussianGenerator->SetRadiusStepNumber(5);
  gaussianGenerator->SetRadius(pow(itk::Math::one_over_pi * 0.75 , 1.0 / 3.0) * 10);
  gaussianGenerator->SetNumberOfGausssians(numberOfGaussians);
 // std::ofstream myfile;
 // myfile.open ("C:/temp/tempParameter3.txt");
 // myfile << " CentX \t" << "Y \t" << "Z \t"  << "SigX \t" << "Y \t" << "Z \t" << "Altit\n";

  int numberAddGaussian = numberOfGaussians;
  for( unsigned int i = 0; i < numberAddGaussian; ++i)
  {
    centerX = rand() % size[0];
    centerY = rand() % size[1];
    centerZ = rand() % size[2];
    sigmaX = minWidthOfGaussian + rand() % (maxWidthOfGaussian - minWidthOfGaussian);
    sigmaY = minWidthOfGaussian + rand() % (maxWidthOfGaussian - minWidthOfGaussian);
    sigmaZ = minWidthOfGaussian + rand() % (maxWidthOfGaussian - minWidthOfGaussian);
    altitude = minAltitudeOfGaussian + rand() % (maxAltitudeOfGaussian - minAltitudeOfGaussian);
    //gaussianGenerator->AddGaussian(centerX, centerY, centerZ, sigmaX, sigmaY, sigmaZ, altitude);
    centerXVec.push_back(centerX);
    centerYVec.push_back(centerY);
    centerZVec.push_back(centerZ);
    sigmaXVec.push_back(sigmaX);
    sigmaYVec.push_back(sigmaY);
    sigmaZVec.push_back(sigmaZ);
    altitudeVec.push_back(altitude);
  //  myfile <<i << " " << centerX << "\t" << centerY << "\t" << centerZ
  //         << "\t \t"<< sigmaX << "\t" << sigmaY <<"\t" <<sigmaZ << "\t \t" << altitude <<"\n";
  }

  gaussianGenerator->AddGaussian(centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec);
  gaussianGenerator->Update();
  gaussianGenerator->CalculateMidpointAndMeanValue();
  std::cout << "Sphere radius   is: " << gaussianGenerator->GetRadius() << std::endl;
  std::cout << "Sphere midpoint is: " << gaussianGenerator->GetSphereMidpoint() << std::endl;
  std::cout << "Mean value is:      " << gaussianGenerator->GetMaxMeanValue() << std::endl;
  ImageType::Pointer gaussianImage = gaussianGenerator->GetOutput();

  //File writer
  typedef  itk::ImageFileWriter< ImageType  > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( "C:/temp/tempImage33.nrrd" );
  writer->SetInput( gaussianImage );
  writer->Update();

  MITK_TEST_END()
}