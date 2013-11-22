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
#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <itkDOMNode.h>
#include <itkDOMNodeXMLWriter.h>
#include <itkImage.h>

//bin\Release\ImageStatisticsTestDriver.exe mitkMultiGaussianTest C:/temp/TestImage/image 1 12 12 10 1 1 5 20 200 "2.5" "2.5" 3

int mitkMultiGaussianTest(int argc, char* argv[])
{
  if ( argc !=14 )
    {
    std::cerr << " 14 arguments expected: [outputFilename, numberOfImages, imageSizeX, imageSizeY, imageSizeZ, numberOfGaussians, minWidthOfGaussian, maxWidthOfGaussian, minAltitudeOfGaussian, maxAltitudeOfGaussian, spacingX, spacingY, spacingZ ] " << std::endl;
    return EXIT_FAILURE;
    }
  // always start with this!
  MITK_TEST_BEGIN("mitkMultiGaussianTest");
  MITK_TEST_CONDITION_REQUIRED(argc == 14, "Test called with 14 parameters");

  const unsigned int                                                   Dimension = 3;
  typedef double    PixelType;
  typedef itk::DOMNode::Pointer                                        DOMNodeType;
  typedef  itk::Image<PixelType, Dimension>                            ImageType;
  typedef itk::MultiGaussianImageSource< ImageType >                   MultiGaussianImageSource;
  std::string                                                          outputFilename = argv[1], name;
  int                                                                  numberOfImages;
  double                                                               centerX, centerY, centerZ, sigmaX, sigmaY, sigmaZ, altitude;
  unsigned int                                                         numberOfGaussians, minWidthOfGaussian, maxWidthOfGaussian, minAltitudeOfGaussian, maxAltitudeOfGaussian;
  itk::MultiGaussianImageSource< ImageType >::VectorType               centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec;
  itk::MultiGaussianImageSource< ImageType >::ItkVectorType            regionOfInterestMax, regionOfInterestMin;
  itk::MultiGaussianImageSource< ImageType >::IndexType                sphereMidpt, maxValueIndexInSphere, minValueIndexInSphere;
  itk::MultiGaussianImageSource< ImageType >::ItkVectorType            X, Y, Z, Sig, Alt;
  MultiGaussianImageSource::Pointer                                    gaussianGenerator;
  itk::DOMNodeXMLWriter::Pointer                                       xmlWriter;
  itk::MultiGaussianImageSource< ImageType >::SpacingValueArrayType    spacing;
  DOMNodeType                                                          domTestCase, domTestImage, domGaussian, domStatistics, domROI;
  ImageType::SizeValueType                                             size[3];
  std::stringstream                                                    ss;
  double                                                               radius = pow(itk::Math::one_over_pi * 0.75 , 1.0 / 3.0) * 10;
  char *                                                               fileNamePointer;

  if ( ! (std::istringstream(argv[2]) >> numberOfImages) ) numberOfImages = 0;
  if ( ! (std::istringstream(argv[3]) >> size[0]) ) size[0] = 0;
  if ( ! (std::istringstream(argv[4]) >> size[1]) ) size[1] = 0;
  if ( ! (std::istringstream(argv[5]) >> size[2]) ) size[2] = 0;
  if ( ! (std::istringstream(argv[6]) >> numberOfGaussians) ) numberOfGaussians = 0;
  if ( ! (std::istringstream(argv[7]) >> minWidthOfGaussian) ) minWidthOfGaussian = (size[0] +  size[1] + size[2]) / 27;
  if ( ! (std::istringstream(argv[8]) >> maxWidthOfGaussian) ) maxWidthOfGaussian = (size[0] +  size[1] + size[2]) / 9;
  if ( ! (std::istringstream(argv[9]) >> minAltitudeOfGaussian) ) minAltitudeOfGaussian = 5;
  if ( ! (std::istringstream(argv[10]) >> maxAltitudeOfGaussian) ) maxAltitudeOfGaussian = 200;
  if ( ! (std::istringstream(argv[11]) >> spacing[0]) ) spacing[0] = 1;
  if ( ! (std::istringstream(argv[12]) >> spacing[1]) ) spacing[1] = 1;
  if ( ! (std::istringstream(argv[13]) >> spacing[2]) ) spacing[2] = 1;

  // Set region of interest in pixels
  regionOfInterestMax.SetElement(0, size[0] - static_cast<int>((radius)/spacing[0]) - 1);
  regionOfInterestMax.SetElement(1, size[1] - static_cast<int>((radius)/spacing[1]) - 1);
  regionOfInterestMax.SetElement(2, size[2] - static_cast<int>((radius)/spacing[2]) - 1);
  regionOfInterestMin.SetElement(0, 0 + static_cast<int>((radius+1)/spacing[0])+1);
  regionOfInterestMin.SetElement(1, 0 + static_cast<int>((radius+1)/spacing[1])+1);
  //regionOfInterestMin.SetElement(2, 0 + static_cast<int>((radius+1)/spacing[2])+1);
  regionOfInterestMin.SetElement(0, 0 + static_cast<int>((radius)/spacing[0])+1);
  regionOfInterestMin.SetElement(1, 0 + static_cast<int>((radius)/spacing[1])+1);
  regionOfInterestMin.SetElement(2, 0 + static_cast<int>((radius)/spacing[2])+1);

  srand (time(NULL));
  int numberAddGaussian = numberOfGaussians;
  for(unsigned  int k = 1; k <= numberOfImages; ++k)
  {
    gaussianGenerator = MultiGaussianImageSource::New();
    gaussianGenerator->SetSize( size );
    gaussianGenerator->SetSpacing( spacing );
    gaussianGenerator->SetRadiusStepNumber(8);
    gaussianGenerator->SetRadius(radius);
    gaussianGenerator->SetNumberOfGausssians(numberOfGaussians);
    gaussianGenerator->SetRegionOfInterest(regionOfInterestMin, regionOfInterestMax);
    // DOM Node Writer
    xmlWriter = itk::DOMNodeXMLWriter::New();
    domTestCase =  itk::DOMNode::New();
    domTestCase->SetName("testcase");
    domTestImage =  itk::DOMNode::New();
    domTestImage->SetName("testimage");
    ss.str("");
    ss << size[0];
    domTestImage->SetAttribute("image-rows", ss.str());
    ss.str("");
    ss << size[1];
    domTestImage->SetAttribute("image-columns", ss.str());
    ss.str("");
    ss << size[2];
    domTestImage->SetAttribute("image-slices", ss.str());
    ss.str("");
    ss << numberOfGaussians;
    domTestImage->SetAttribute("numberOfGaussians", ss.str());
    ss.str("");
    ss << spacing[0];
    domTestImage->SetAttribute("spacingX", ss.str());
    ss.str("");
    ss << spacing[1];
    domTestImage->SetAttribute("spacingY", ss.str());
    ss.str("");
    ss << spacing[2];
    domTestImage->SetAttribute("spacingZ", ss.str());
    domTestCase->AddChildAtBegin(domTestImage);

    for( unsigned int i = 0; i < numberAddGaussian; ++i)
    {

      domGaussian =  itk::DOMNode::New() ;
      domGaussian->SetName("gaussian");
      domTestImage->AddChildAtEnd(domGaussian);
      // generate the midpoint and the daviation in mm
      centerX = rand() % static_cast<int>(size[0] * spacing[0]);
      ss.str("");
      ss << centerX;
      domGaussian->SetAttribute("centerIndexX", ss.str());

      centerY = rand() % static_cast<int>(size[1] * spacing[1]);
      ss.str("");
      ss << centerY;
      domGaussian->SetAttribute("centerIndexY", ss.str());

      centerZ = rand() % static_cast<int>(size[2] * spacing[2]);
      ss.str("");
      ss << centerZ;
      domGaussian->SetAttribute("centerIndexZ", ss.str());

      sigmaX = minWidthOfGaussian + rand() % (maxWidthOfGaussian - minWidthOfGaussian);
      ss.str("");
      ss << sigmaX;
      domGaussian->SetAttribute("deviationX", ss.str());

      sigmaY = minWidthOfGaussian + rand() % (maxWidthOfGaussian - minWidthOfGaussian);
      ss.str("");
      ss << sigmaY;
      domGaussian->SetAttribute("deviationY", ss.str());

      sigmaZ = minWidthOfGaussian + rand() % (maxWidthOfGaussian - minWidthOfGaussian);
      ss.str("");
      ss << sigmaZ;
      domGaussian->SetAttribute("deviationZ", ss.str());

      altitude = minAltitudeOfGaussian + rand() % (maxAltitudeOfGaussian - minAltitudeOfGaussian);
      ss.str("");
      ss << altitude;
      domGaussian->SetAttribute("altitude", ss.str());

      centerXVec.push_back(centerX);
      centerYVec.push_back(centerY);
      centerZVec.push_back(centerZ);
      sigmaXVec.push_back(sigmaX);
      sigmaYVec.push_back(sigmaY);
      sigmaZVec.push_back(sigmaZ);
      altitudeVec.push_back(altitude);

    }

    gaussianGenerator->AddGaussian(centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec);
    centerXVec.clear();
    centerYVec.clear();
    centerZVec.clear();
    sigmaXVec.clear();
    sigmaYVec.clear();
    sigmaZVec.clear();
    altitudeVec.clear();
    try {
    gaussianGenerator->Update();
    gaussianGenerator->CalculateMidpointAndMeanValue();
    } catch (std::exception& e)
    {
      std::cout << "Error: " << e.what() << std::endl;
    }

    //region of interest
    domROI = itk::DOMNode::New();
    domROI->SetName("roi");
    domTestCase->AddChildAtEnd(domROI);
    ss.str("");
    ss << regionOfInterestMax[0];
    domROI->SetAttribute("maximumX", ss.str());
    ss.str("");
    ss << regionOfInterestMin[0];
    domROI->SetAttribute("minimumX", ss.str());
    ss.str("");
    ss << regionOfInterestMax[1];
    domROI->SetAttribute("maximumY", ss.str());
    ss.str("");
    ss << regionOfInterestMin[1];
    domROI->SetAttribute("minimumY", ss.str());
    ss.str("");
    ss << regionOfInterestMax[2];
    domROI->SetAttribute("maximumZ", ss.str());
    ss.str("");
    ss << regionOfInterestMin[2];
    domROI->SetAttribute("minimumZ", ss.str());

    //peak and peak coordinate
    domStatistics = itk::DOMNode::New();
    domStatistics->SetName("statistic");
    domTestCase->AddChildAtEnd(domStatistics);
    sphereMidpt = gaussianGenerator->GetSphereMidpoint();
    ss.str("");
    ss << sphereMidpt[0];
    domStatistics->SetAttribute("peakIndexX", ss.str());
    ss.str("");
    ss << sphereMidpt[1];
    domStatistics->SetAttribute("peakIndexY", ss.str());
    ss.str("");
    ss << sphereMidpt[2];
    domStatistics->SetAttribute("peakIndexZ", ss.str());
    ss.str("");
    ss << gaussianGenerator->GetMaxMeanValue();
    domStatistics->SetAttribute("peak", ss.str());

    //optimize the mean value in the sphere
    gaussianGenerator->OptimizeMeanValue();
    ss.str("");
    ss << gaussianGenerator->GetMaxMeanValue();
    domStatistics->SetAttribute("peakOptimized", ss.str());


    //maximum and maximum coordinate
    gaussianGenerator->CalculateMaxAndMinInSphere();
    maxValueIndexInSphere = gaussianGenerator->GetMaxValueIndexInSphere();
    ss.str("");
    ss << maxValueIndexInSphere[0];
    domStatistics->SetAttribute("maximumIndexX", ss.str());
    ss.str("");
    ss << maxValueIndexInSphere[1];
    domStatistics->SetAttribute("maximumIndexY", ss.str());
    ss.str("");
    ss << maxValueIndexInSphere[2];
    domStatistics->SetAttribute("maximumIndexZ", ss.str());
    ss.str("");
    ss << gaussianGenerator->GetMaxValueInSphere();
    domStatistics->SetAttribute("maximum", ss.str());

    //minimum and minimum coordinate
    minValueIndexInSphere = gaussianGenerator->GetMinValueIndexInSphere();
    ss.str("");
    ss << minValueIndexInSphere[0];
    domStatistics->SetAttribute("minimumIndexX", ss.str());
    ss.str("");
    ss << minValueIndexInSphere[1];
    domStatistics->SetAttribute("minimumIndexY", ss.str());
    ss.str("");
    ss << minValueIndexInSphere[2];
    domStatistics->SetAttribute("minimumIndexZ", ss.str());
    ss.str("");
    ss << gaussianGenerator->GetMinValueInSphere();
    domStatistics->SetAttribute("minimum", ss.str());

    // .xml (Data)
    ss.str("");
    if(k < 10){
      ss << outputFilename <<"00"<< k <<".xml";
    }else if(k < 100){
      ss << outputFilename <<"0"<< k <<".xml";
    }else{   ss << outputFilename << k <<".xml";}
    name = ss.str();
    fileNamePointer = (char*) name.c_str();
    xmlWriter->SetFileName( fileNamePointer);
    xmlWriter->SetInput( domTestCase );
    xmlWriter->Update();
    ImageType::Pointer gaussianImage = gaussianGenerator->GetOutput();

    //.nrrd (Image)
    typedef  itk::ImageFileWriter< ImageType  > WriterType;
    WriterType::Pointer writer = WriterType::New();
    ss.str("");
    if(k < 10)
    {
      ss << outputFilename <<"00"<< k <<".nrrd";
    }else if(k < 100)
    {
      ss << outputFilename <<"0"<< k <<".nrrd";
    }else
    {
      ss << outputFilename << k <<".nrrd";
    }
    name = ss.str();
    fileNamePointer = (char*) name.c_str();
    writer->SetFileName( fileNamePointer);
    writer->SetInput( gaussianImage );
    writer->Update();
  }
 MITK_TEST_END()
}