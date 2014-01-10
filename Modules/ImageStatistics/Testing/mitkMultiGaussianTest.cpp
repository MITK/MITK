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
#include <itkDOMNodeXMLReader.h>
#include <itkImage.h>

// Commandline:(for exmaple) mitkMultiGaussianTest C:/temp/output C:/temp/inputFile.xml
//
//For Example: inputFile.xml
//<testcase>
//  <testimage image-rows="20" image-columns="20" image-slices="20" numberOfGaussians="1" spacingX="1" spacingY="1" spacingZ="1" entireHotSpotInImage="1">
//    <gaussian centerIndexX="10" centerIndexY="10" centerIndexZ="10" deviationX="6" deviationY="6" deviationZ="6" altitude="200"/>
//  </testimage>
//<segmentation numberOfLabels="1" hotspotRadiusInMM="6.2035">
//  <roi label="1" maximumSizeX="20" minimumSizeX="0" maximumSizeY="20" minimumSizeY="0" maximumSizeZ="20" minimumSizeZ="0"/>
// </segmentation>
//</testcase>
//
//For Example: output.xml
//  <testcase>
//  <testimage image-rows="20" image-columns="20" image-slices="20" numberOfGaussians="1" spacingX="1" spacingY="1" spacingZ="1" entireHotSpotInImage="1">
//    <gaussian centerIndexX="50" centerIndexY="50" centerIndexZ="50" deviationX="5" deviationY="5" deviationZ="5" altitude="200"/>
//    <gaussian centerIndexX="46" centerIndexY="46" centerIndexZ="46" deviationX="40" deviationY="40" deviationZ="40" altitude="170"/>
//  </testimage>
// <segmentation numberOfLabels="1" hotspotRadiusInMM="8">
//  <roi hotspotRadiusInMM="6.2035" maximumSizeX="122" minimumSizeX="7" maximumSizeY="122" minimumSizeY="7" maximumSizeZ="60" minimumSizeZ="4"/>
// </segmentation>
//  <statistic hotspotIndexX="50" hotspotIndexY="50" hotspotIndexZ="25" peak="291.067" mean="291.067" maximumIndexX="50" maximumIndexY="50" maximumIndexZ="25" maximum="367.469" minimumIndexX="55" minimumIndexY="53" minimumIndexZ="26" minimum="254.939"/>
//</testcase>
//
//bin\Release\ImageStatisticsTestDriver.exe mitkMultiGaussianTest C:/temp/TestImage/image 1 12 12 10 1 1 5 20 200 "2.5" "2.5" 3
//
// bin\Release\ImageStatisticsTestDriver.exe mitkMultiGaussianTest C:/temp/HotSpotTestImage/ImageNeu30 5 30 30 15 1 5 15 20 200 1 1 2

int mitkMultiGaussianTest(int argc, char* argv[])
{
  //if ( argc != 14 || argc != 3 )
  //{
  //  std::cerr << " 14 arguments expected: [outputFilename, numberOfImages, imageSizeX, imageSizeY, imageSizeZ, numberOfGaussians, minWidthOfGaussian, maxWidthOfGaussian, minAltitudeOfGaussian, maxAltitudeOfGaussian, spacingX, spacingY, spacingZ ] \n OR \n 3 argument expected : [outputFilename, input.xml] \n " << std::endl;
  //  return EXIT_FAILURE;
  //}
  //// always start with this!
  //MITK_TEST_BEGIN("mitkMultiGaussianTest");
  //MITK_TEST_CONDITION_REQUIRED(argc == 14, "Test called with 14 parameters");

  if( argc == 14)
  {


    const unsigned int                                                   Dimension = 3;
    typedef double                                                       PixelType;
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
    regionOfInterestMax.SetElement(0, size[0] - static_cast<int>((radius)/spacing[0] + 0.9999) - 1);
    regionOfInterestMax.SetElement(1, size[1] - static_cast<int>((radius)/spacing[1] + 0.9999) - 1);
    regionOfInterestMax.SetElement(2, size[2] - static_cast<int>((radius)/spacing[2] + 0.9999) - 1);
    regionOfInterestMin.SetElement(0, 0 + static_cast<int>((radius)/spacing[0]+ 0.9999));
    regionOfInterestMin.SetElement(1, 0 + static_cast<int>((radius)/spacing[1]+ 0.9999));
    regionOfInterestMin.SetElement(2, 0 + static_cast<int>((radius)/spacing[2]+ 0.9999));

    srand (time(NULL));
    int numberAddGaussian = numberOfGaussians;
    unsigned int k = 0;
    unsigned int count = 0;
    while(k != numberOfImages && count < 500)
      // for(unsigned  int k = 1; k <= numberOfImages; ++k)
    {
      ++count;
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
      ss << radius;
      domROI->SetAttribute("hotspotRadiusInMM", ss.str());
      ss.str("");
      ss << regionOfInterestMax[0];
      domROI->SetAttribute("maximumSizeX", ss.str());
      ss.str("");
      ss << regionOfInterestMin[0];
      domROI->SetAttribute("minimumSizeX", ss.str());
      ss.str("");
      ss << regionOfInterestMax[1];
      domROI->SetAttribute("maximumSizeY", ss.str());
      ss.str("");
      ss << regionOfInterestMin[1];
      domROI->SetAttribute("minimumSizeY", ss.str());
      ss.str("");
      ss << regionOfInterestMax[2];
      domROI->SetAttribute("maximumSizeZ", ss.str());
      ss.str("");
      ss << regionOfInterestMin[2];
      domROI->SetAttribute("minimumSizeZ", ss.str());

      //peak and peak coordinate
      domStatistics = itk::DOMNode::New();
      domStatistics->SetName("statistic");
      domTestCase->AddChildAtEnd(domStatistics);
      sphereMidpt = gaussianGenerator->GetSphereMidpoint();
      ss.str("");
      ss << sphereMidpt[0];
      domStatistics->SetAttribute("hotspotIndexX", ss.str());
      ss.str("");
      ss << sphereMidpt[1];
      domStatistics->SetAttribute("hotspotIndexY", ss.str());
      ss.str("");
      ss << sphereMidpt[2];
      domStatistics->SetAttribute("hotspotIndexZ", ss.str());
      ss.str("");
      ss << gaussianGenerator->GetMaxMeanValue();
      domStatistics->SetAttribute("peak", ss.str());

      //optimize the mean value in the sphere
      //  gaussianGenerator->OptimizeMeanValue();
      ss.str("");
      ss << gaussianGenerator->GetMaxMeanValue();
      domStatistics->SetAttribute("mean", ss.str());


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


      // test if the midpoint of the sphere is not the same as the maximum's index and saves only such a case
      if( sphereMidpt[0]!= maxValueIndexInSphere[0] && sphereMidpt[1]!= maxValueIndexInSphere[1] && sphereMidpt[2]!= maxValueIndexInSphere[2])
      {
        k++;
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
    }
  }


  //--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
  // Read the parmaeter from a .xml File.
  // In the inputFile.xml we find the characteristics of the Gaussian and the ROI's. Hier we can have more then one ROI -> we find the hot spot for each of the ROI's; we can set the entire HotSpot to be in the ROI or just its midpoint, but not necessary the whole HotSpot.
  else
  {
    const unsigned int                                                   Dimension = 3;
    typedef double    PixelType;
    typedef itk::DOMNode::Pointer                                        DOMNodeType;
    typedef  itk::Image<PixelType, Dimension>                            ImageType;
    typedef itk::MultiGaussianImageSource< ImageType >                   MultiGaussianImageSource;
    std::string                                                          outputFilename = argv[1], name;
    int                                                                  numberOfImages;
    double                                                               centerX, centerY, centerZ, sigmaX, sigmaY, sigmaZ, altitude, hotSpotRadiusInMM;
    unsigned int                                                         numberOfGaussians, minWidthOfGaussian, maxWidthOfGaussian, minAltitudeOfGaussian, maxAltitudeOfGaussian, numberOfLabels;
    itk::MultiGaussianImageSource< ImageType >::VectorType               centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec,  ROImaxSizeX, ROIminSizeX, ROImaxSizeY, ROIminSizeY, ROImaxSizeZ, ROIminSizeZ, label;
    itk::MultiGaussianImageSource< ImageType >::ItkVectorType            regionOfInterestMax, regionOfInterestMin;
    itk::MultiGaussianImageSource< ImageType >::IndexType                sphereMidpt, maxValueIndexInSphere, minValueIndexInSphere;
    itk::MultiGaussianImageSource< ImageType >::ItkVectorType            X, Y, Z, Sig, Alt;
    MultiGaussianImageSource::Pointer                                    gaussianGenerator;
    itk::DOMNodeXMLWriter::Pointer                                       xmlWriter;
    itk::MultiGaussianImageSource< ImageType >::SpacingValueArrayType    spacing;
    DOMNodeType                                                          domTestCase, domTestImage, domGaussian, domSegmentation, domStatistics, domROI;
    ImageType::SizeValueType                                             size[3];
    std::stringstream                                                    ss;
    double                                                               radius = pow(itk::Math::one_over_pi * 0.75 , 1.0 / 3.0) * 10;
    char *                                                               fileNamePointer;
    std::string                                                          attributeValue;
    double                                                               value;
    bool                                                                 entireHotSpotInImage;
    int                                                                  maxSize, minSize;

    std::string filename = argv[2];
    itk::DOMNodeXMLReader::Pointer xmlReader = itk::DOMNodeXMLReader::New();
    xmlReader->SetFileName( filename );
    xmlReader->Update();
    itk::DOMNode::Pointer domRoot = xmlReader->GetOutput();
    typedef std::vector<itk::DOMNode*> NodeList;

    // read test image parameters, fill result structure
    NodeList testimages;
    domRoot->GetChildren("testimage", testimages);
    MITK_TEST_CONDITION_REQUIRED( testimages.size() == 1, "One test image defined" )
      itk::DOMNode* testimage = testimages[0];

    attributeValue = testimage->GetAttribute("image-rows");
    std::stringstream(attributeValue) >> size[0];
    attributeValue = testimage->GetAttribute("image-columns");
    std::stringstream(attributeValue) >> size[1];
    attributeValue = testimage->GetAttribute("image-slices");
    std::stringstream(attributeValue) >> size[2];

    attributeValue = testimage->GetAttribute( "numberOfGaussians" );
    std::stringstream(attributeValue) >> numberOfGaussians;

    attributeValue = testimage->GetAttribute( "spacingX" );
    std::stringstream(attributeValue) >> spacing[0];
    attributeValue = testimage->GetAttribute( "spacingY" );
    std::stringstream(attributeValue) >> spacing[1];
    attributeValue = testimage->GetAttribute( "spacingZ" );
    std::stringstream(attributeValue) >> spacing[2];
    attributeValue = testimage->GetAttribute( "entireHotSpotInImage" );
    std::stringstream(attributeValue) >> entireHotSpotInImage;

    std::cout << "Read size parameters (x,y,z): " << size[0] << ", " << size[1] << ", " << size[2] << "\n" << std::endl;
    std::cout << "Read spacing parameters (x,y,z): " << spacing[0] << ", " << spacing[1] << ", " << spacing[2]<< "\n" << std::endl;

    NodeList gaussians;
    testimage->GetChildren("gaussian", gaussians);
    itk::DOMNode* gaussian;
    for(int i = 0; i < numberOfGaussians ; ++i)
    {
      gaussian = gaussians[i];

      attributeValue = gaussian->GetAttribute( "centerIndexX" );
      std::stringstream(attributeValue) >> value;
      centerXVec.push_back(value);

      attributeValue = gaussian->GetAttribute( "centerIndexY" );
      std::stringstream(attributeValue) >> value;
      centerYVec.push_back(value);

      attributeValue = gaussian->GetAttribute( "centerIndexZ" );
      std::stringstream(attributeValue) >> value;
      centerZVec.push_back(value);

      std::cout << "Read center of Gaussian (x,y,z): " << centerXVec[i] << ", " << centerYVec[i] << ", " << centerZVec[i] << "\n" << std::endl;

      attributeValue = gaussian->GetAttribute( "deviationX" );
      std::stringstream(attributeValue) >> value;
      sigmaXVec.push_back(value);

      attributeValue = gaussian->GetAttribute( "deviationY" );
      std::stringstream(attributeValue) >> value;
      sigmaYVec.push_back(value);

      attributeValue = gaussian->GetAttribute( "deviationZ" );
      std::stringstream(attributeValue) >> value;
      sigmaZVec.push_back(value);

      std::cout << "Read deviation of Gaussian (x,y,z): " << sigmaXVec[i] << ", " << sigmaYVec[i] << ", " << sigmaZVec[i] << "\n" << std::endl;

      attributeValue = gaussian->GetAttribute( "altitude" );
      std::stringstream(attributeValue) >> value;
      altitudeVec.push_back(value);
      std::cout << "Read altitude: " << altitudeVec[i] << "\n" << std::endl;
    }



    // read ROI's parameter
    NodeList segmentations;
    domRoot->GetChildren("segmentation", segmentations);
    MITK_TEST_CONDITION_REQUIRED( segmentations.size() == 1, "One ROI image defined" )
      itk::DOMNode* segmentation = segmentations[0];

    attributeValue = segmentation->GetAttribute("numberOfLabels");
    std::stringstream(attributeValue) >> numberOfLabels;
    attributeValue = segmentation->GetAttribute("hotspotRadiusInMM");
    std::stringstream(attributeValue) >> hotSpotRadiusInMM;


    std::cout << "Read number of labels: " << numberOfLabels << "\n" << std::endl;
    std::cout << "Read radius in mm : " << hotSpotRadiusInMM << "\n" << std::endl;

    NodeList rois;
    segmentation->GetChildren("roi", rois);
    itk::DOMNode* roi;
    // for each label i take the ROI and set it to be the i'th  element of ROImaxSize* and ROIminSize* ( * = X, Y, Z)
    for(int i = 0; i < numberOfLabels ; ++i)
    {
      roi = rois[i];

      attributeValue = roi->GetAttribute( "label" );
      std::stringstream(attributeValue) >> value;
      label.push_back(value);

      attributeValue = roi->GetAttribute( "maximumSizeX" );
      std::stringstream(attributeValue) >> value;
      ROImaxSizeX.push_back(value);

      attributeValue = roi->GetAttribute( "minimumSizeX" );
      std::stringstream(attributeValue) >> value;
      ROIminSizeX.push_back(value);

      attributeValue = roi->GetAttribute( "maximumSizeY" );
      std::stringstream(attributeValue) >> value;
      ROImaxSizeY.push_back(value);

      attributeValue = roi->GetAttribute( "minimumSizeY" );
      std::stringstream(attributeValue) >> value;
      ROIminSizeY.push_back(value);

      attributeValue = roi->GetAttribute( "maximumSizeZ" );
      std::stringstream(attributeValue) >> value;
      ROImaxSizeZ.push_back(value);

      attributeValue = roi->GetAttribute( "minimumSizeZ" );
      std::stringstream(attributeValue) >> value;
      ROIminSizeZ.push_back(value);

      std::cout << "Read ROI with label nummber: " << label[i] << "with min and max values in the x-, y-, z-Achse: [" << ROIminSizeX[i] << ROImaxSizeX[i] <<"], [" << ROIminSizeY[i] << ROImaxSizeY[i] <<"], [" << ROIminSizeZ[i] << ROImaxSizeZ[i] <<"]\n" << std::endl;
    }

    //write test image parameter

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
    ss.str("");
    ss << entireHotSpotInImage;
    domTestImage->SetAttribute("entireHotSpotInImage", ss.str());

    domTestCase->AddChildAtBegin(domTestImage);

    int numberAddGaussian = numberOfGaussians;

    for( unsigned int i = 0; i < numberAddGaussian; ++i)
    {

      domGaussian =  itk::DOMNode::New() ;
      domGaussian->SetName("gaussian");
      domTestImage->AddChildAtEnd(domGaussian);
      // generate the midpoint and the daviation in mm
      centerX = centerXVec[i];
      ss.str("");
      ss << centerX;
      domGaussian->SetAttribute("centerIndexX", ss.str());

      centerY = centerYVec[i];
      ss.str("");
      ss << centerY;
      domGaussian->SetAttribute("centerIndexY", ss.str());

      centerZ = centerZVec[i];
      ss.str("");
      ss << centerZ;
      domGaussian->SetAttribute("centerIndexZ", ss.str());

      sigmaX = sigmaXVec[i];
      ss.str("");
      ss << sigmaX;
      domGaussian->SetAttribute("deviationX", ss.str());

      sigmaY = sigmaYVec[i];
      ss.str("");
      ss << sigmaY;
      domGaussian->SetAttribute("deviationY", ss.str());

      sigmaZ = sigmaZVec[i];
      ss.str("");
      ss << sigmaZ;
      domGaussian->SetAttribute("deviationZ", ss.str());

      altitude = altitudeVec[i];
      ss.str("");
      ss << altitude;
      domGaussian->SetAttribute("altitude", ss.str());

    }

    radius = hotSpotRadiusInMM;
    // set the parameter for the gaussianGenerator
    gaussianGenerator = MultiGaussianImageSource::New();
    gaussianGenerator->SetSize( size );
    gaussianGenerator->SetSpacing( spacing );
    gaussianGenerator->SetRadiusStepNumber(8);
    gaussianGenerator->SetRadius(radius);
    gaussianGenerator->SetNumberOfGausssians(numberOfGaussians);
    gaussianGenerator->AddGaussian(centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec);

    domSegmentation = itk::DOMNode::New();
    domSegmentation->SetName("segmentation");
    domTestCase->AddChildAtEnd(domSegmentation);


    ss.str("");
    ss << numberOfLabels;
    domSegmentation->SetAttribute("numberOfLabels", ss.str());
    ss.str("");
    ss << hotSpotRadiusInMM;
    domSegmentation->SetAttribute("hotspotRadiusInMM", ss.str());


    // set the region of interest for each label i
    for (unsigned int i = 0; i < numberOfLabels; ++i)
    {
      // Set region of interest in index values. The entire HotSpot is in the image.
      if(entireHotSpotInImage)
      {

        // x axis region of interest------------------------------------------------------
        maxSize = size[0] - static_cast<int>((radius)/spacing[0] + 0.9999);
        minSize = 0.0 + static_cast<int>((radius)/spacing[0]+ 0.9999);
        if( minSize >= maxSize )
        {
          std::cout << "The sphere is larger then the image in the x axis!" << std::endl;

        }
        // the maximum in the x-Axis
        regionOfInterestMax.SetElement( 0, ( ROImaxSizeX[i] < maxSize ) ? ROImaxSizeX[i] : maxSize );
        // the minimum in the x-Axis
        regionOfInterestMin.SetElement( 0, ( ROIminSizeX[i] > minSize ) ? ROIminSizeX[i] : minSize );

        // y axis region of interest------------------------------------------------------
        maxSize = size[1] - static_cast<int>((radius)/spacing[1] + 0.9999);
        minSize = 0.0 + static_cast<int>((radius)/spacing[1]+ 0.9999);
        if( minSize >= maxSize )
        {
          std::cout << "The sphere is larger then the image in the y axis!" << std::endl;
        }
        // the maximum in the y-Axis
        regionOfInterestMax.SetElement( 1, ( ROImaxSizeY[i] < maxSize ) ? ROImaxSizeY[i] : maxSize );
        // the minimum in the y-Axis
        regionOfInterestMin.SetElement( 1, ( ROIminSizeY[i] > minSize ) ? ROIminSizeY[i] : minSize );

        // z axis region of interest------------------------------------------------------
        maxSize = size[2] - static_cast<int>((radius)/spacing[1] + 0.9999);
        minSize = 0.0 + static_cast<int>((radius)/spacing[1]+ 0.9999);
        if( minSize >= maxSize )
        {
          std::cout << "The sphere is larger then the image in the z axis!" << std::endl;
        }
        // the maximum in the z-Axis
        regionOfInterestMax.SetElement( 2, ( ROImaxSizeZ[i] < maxSize ) ? ROImaxSizeZ[i] : maxSize );
        // the minimum in the z-Axis
        regionOfInterestMin.SetElement( 2, ( ROIminSizeZ[i] > minSize ) ? ROIminSizeZ[i] : minSize );

      }
      // Set region of interest in index values. The midpoint of the HotSpot is in the image, but not necessary the whole HotSpot
      else
      {
        // x axis region of interest------------------------------------------------------
        regionOfInterestMax.SetElement( 0, ROImaxSizeX[i] );
        regionOfInterestMin.SetElement( 0, ROIminSizeX[i] );
        // y axis region of interest------------------------------------------------------
        regionOfInterestMax.SetElement( 1, ROImaxSizeY[i] );
        regionOfInterestMin.SetElement( 1, ROIminSizeY[i] );
        // z axis region of interest------------------------------------------------------
        regionOfInterestMax.SetElement( 2, ROImaxSizeZ[i] );
        regionOfInterestMin.SetElement( 2, ROIminSizeZ[i] );

      }


        gaussianGenerator->SetRegionOfInterest(regionOfInterestMin, regionOfInterestMax);
        gaussianGenerator->Update();


        //write region of interest for the .xml file
        domROI = itk::DOMNode::New();
        domROI->SetName("roi");
        domSegmentation->AddChildAtEnd(domROI);

        ss.str("");
        ss << label[i];
        domROI->SetAttribute("label", ss.str());
        ss.str("");
        ss << ROImaxSizeX[i];
        domROI->SetAttribute("maximumSizeX", ss.str());
        ss.str("");
        ss << ROIminSizeX[i];
        domROI->SetAttribute("minimumSizeX", ss.str());
        ss.str("");
        ss << ROImaxSizeY[i];
        domROI->SetAttribute("maximumSizeY", ss.str());
        ss.str("");
        ss << ROIminSizeY[i];
        domROI->SetAttribute("minimumSizeY", ss.str());
        ss.str("");
        ss << ROImaxSizeZ[i];
        domROI->SetAttribute("maximumSizeZ", ss.str());
        ss.str("");
        ss << ROIminSizeZ[i];
        domROI->SetAttribute("minimumSizeZ", ss.str());


        // Calculate the mean value and the midpoint of the wanted sphere.
        gaussianGenerator->CalculateTheMidPointMeanValueWithOctree();

        //peak and peak coordinate
        domStatistics = itk::DOMNode::New();
        domStatistics->SetName("statistic");
        domTestCase->AddChildAtEnd(domStatistics);
        sphereMidpt = gaussianGenerator->GetSphereMidpoint();
        ss.str("");
        ss << sphereMidpt[0];
        domStatistics->SetAttribute("hotspotIndexX", ss.str());
        ss.str("");
        ss << sphereMidpt[1];
        domStatistics->SetAttribute("hotspotIndexY", ss.str());
        ss.str("");
        ss << sphereMidpt[2];
        domStatistics->SetAttribute("hotspotIndexZ", ss.str());
        ss.str("");
        ss << gaussianGenerator->GetMaxMeanValue();
        domStatistics->SetAttribute("peak", ss.str());

        //optimize the mean value in the sphere
        // gaussianGenerator->OptimizeMeanValue();
        ss.str("");
        ss << gaussianGenerator->GetMaxMeanValue();
        domStatistics->SetAttribute("mean", ss.str());


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


    }


    /*

    //The midpoint of the HotSpot is in the image, but not necessary the whole HotSpot
    else
    {
      // set the region of interest for each label i
      for (unsigned int i = 0; i < numberOfLabels; ++i)
      {
        for (unsigned int k = 0 ; k < 3; ++k)
        {

          maxSize = ROImaxSizeX[i];
          minSize = ROIminSizeX[i];
          if(minSize > maxSize)
          {
            std::cout << "The sphere is larger then the region of interest! Set the roi to be the whole image " << std::endl;
            maxSize = size[k];
            minSize = 0.0;
          }
          // the maximum in the k-Axis
          regionOfInterestMax.SetElement(k, (maxSize < size[k]) ? maxSize : size[k] - 1.0 );
          // the minimum in the k-Axis
          regionOfInterestMin.SetElement(k, (minSize > 0.0) ? minSize : 0.0 );
        }


        gaussianGenerator->SetRegionOfInterest(regionOfInterestMin, regionOfInterestMax);
        gaussianGenerator->Update();
        //TODO
        // gaussianGenerator->GenerateCuboidSegmentationInSphere();
        //gaussianGenerator->CalculateMidpointAndMeanValue();
        gaussianGenerator->CalculateTheMidPointMeanValueInCuboid();

        //region of interest
        domROI = itk::DOMNode::New();
        domROI->SetName("roi");
        domSegmentation->AddChildAtEnd(domROI);

        ss.str("");
        ss << label[i];
        domROI->SetAttribute("label", ss.str());
        ss.str("");
        ss << regionOfInterestMax[0];
        domROI->SetAttribute("maximumSizeX", ss.str());
        ss.str("");
        ss << regionOfInterestMin[0];
        domROI->SetAttribute("minimumSizeX", ss.str());
        ss.str("");
        ss << regionOfInterestMax[1];
        domROI->SetAttribute("maximumSizeY", ss.str());
        ss.str("");
        ss << regionOfInterestMin[1];
        domROI->SetAttribute("minimumSizeY", ss.str());
        ss.str("");
        ss << regionOfInterestMax[2];
        domROI->SetAttribute("maximumSizeZ", ss.str());
        ss.str("");
        ss << regionOfInterestMin[2];
        domROI->SetAttribute("minimumSizeZ", ss.str());

        //peak and peak coordinate
        domStatistics = itk::DOMNode::New();
        domStatistics->SetName("statistic");
        domTestCase->AddChildAtEnd(domStatistics);
        sphereMidpt = gaussianGenerator->GetSphereMidpoint();
        ss.str("");
        ss << sphereMidpt[0];
        domStatistics->SetAttribute("hotspotIndexX", ss.str());
        ss.str("");
        ss << sphereMidpt[1];
        domStatistics->SetAttribute("hotspotIndexY", ss.str());
        ss.str("");
        ss << sphereMidpt[2];
        domStatistics->SetAttribute("hotspotIndexZ", ss.str());
        ss.str("");
        ss << gaussianGenerator->GetMaxMeanValue();
        domStatistics->SetAttribute("peak", ss.str());

        //optimize the mean value in the sphere
        // gaussianGenerator->OptimizeMeanValue();
        ss.str("");
        ss << gaussianGenerator->GetMaxMeanValue();
        domStatistics->SetAttribute("mean", ss.str());


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

      }*/





    // .xml (Data)
    ss.str("");
    ss << outputFilename << ".xml";
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
    ss << outputFilename << ".nrrd";
    name = ss.str();
    fileNamePointer = (char*) name.c_str();
    writer->SetFileName( fileNamePointer);
    writer->SetInput( gaussianImage );
    writer->Update();

    //  gaussianGenerator -> WriteXMLToTestTheCuboid();



  }
  //  MITK_TEST_END()

}
