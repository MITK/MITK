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
//<segmentation numberOfLabels="2" hotspotRadiusInMM="6.2035">
//  <roi label="1" maximumSizeX="20" minimumSizeX="12" maximumSizeY="20" minimumSizeY="12" maximumSizeZ="20" minimumSizeZ="12"/>
//  <roi label="2" maximumSizeX="10" minimumSizeX="0" maximumSizeY="10" minimumSizeY="0" maximumSizeZ="10" minimumSizeZ="0"/>
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


bool IsInOtherROI(int,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType);


int mitkMultiGaussianTest(int argc, char* argv[])
{
  // Read the parmaeter from a .xml File.
  // In the inputFile.xml we find the characteristics of the Gaussian and the ROI's. Hier we can have more then one ROI -> we find the hot spot for each of the ROI's; we can set the entire HotSpot to be in the image or just its midpoint, but not necessary the whole HotSpot.

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
      //TODO
      attributeValue = gaussian->GetAttribute( "centerIndexX" );
      std::stringstream(attributeValue) >> value;
      centerXVec.push_back(value * spacing[0]);

      attributeValue = gaussian->GetAttribute( "centerIndexY" );
      std::stringstream(attributeValue) >> value;
      centerYVec.push_back(value * spacing[1]);

      attributeValue = gaussian->GetAttribute( "centerIndexZ" );
      std::stringstream(attributeValue) >> value;
      centerZVec.push_back(value * spacing[2]);

      std::cout << "Read center of Gaussian (x,y,z) in mm: " << centerXVec[i] << ", " << centerYVec[i] << ", " << centerZVec[i] << "\n" << std::endl;

      attributeValue = gaussian->GetAttribute( "deviationX" );
      std::stringstream(attributeValue) >> value;
      sigmaXVec.push_back(value * spacing[0]);

      attributeValue = gaussian->GetAttribute( "deviationY" );
      std::stringstream(attributeValue) >> value;
      sigmaYVec.push_back(value * spacing[1]);

      attributeValue = gaussian->GetAttribute( "deviationZ" );
      std::stringstream(attributeValue) >> value;
      sigmaZVec.push_back(value * spacing[2]);

      std::cout << "Read deviation of Gaussian (x,y,z) in mm: " << sigmaXVec[i] << ", " << sigmaYVec[i] << ", " << sigmaZVec[i] << "\n" << std::endl;

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

    std::cout << "Read number of labels: " << numberOfLabels << std::endl;
    std::cout << "Read radius in mm : " << hotSpotRadiusInMM << std::endl;

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

      std::cout << "Read ROI with label number: " << label[i] << " with min and max values in the x-, y-, z-Achse: [" << ROIminSizeX[i] << " " << ROImaxSizeX[i] <<"], [" << ROIminSizeY[i] << " " << ROImaxSizeY[i] <<"], [" << ROIminSizeZ[i] << " " << ROImaxSizeZ[i] <<"]\n" << std::endl;
    }

    // Check whether the ROI's are correct defined, i.e. whether the ROI's are disjoint
    for(int i = 1; i < numberOfLabels ; ++i)
    {
      // check whether the edges of the i'th ROI is in another ROI included (when yes -> ERROR)
      bool isInOtherROI = IsInOtherROI( i, ROIminSizeX, ROImaxSizeX, ROIminSizeY, ROImaxSizeY, ROIminSizeZ, ROImaxSizeZ );
      if( isInOtherROI)
      {
        std::cout << "The ROI's in the different labels should be disjoint! Please define it correct. " << std::endl;
        return 0;
      }
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

    for( unsigned int i = 0; i < numberOfGaussians; ++i)
    {

      domGaussian =  itk::DOMNode::New() ;
      domGaussian->SetName("gaussian");
      domTestImage->AddChildAtEnd(domGaussian);
      // write the midpoint and the daviation in pixel units
      centerX = centerXVec[i] / spacing[0];
      ss.str("");
      ss <<  centerX; // * spacing[0]; //static_cast<double>( static_cast<int>( centerX / spacing[0] + 0.9999 ) );
      domGaussian->SetAttribute("centerIndexX", ss.str());

      centerY = centerYVec[i] / spacing[1];
      ss.str("");
      ss << centerY; // * spacing[1]; //static_cast<double>( static_cast<int>( centerY / spacing[1] + 0.9999 ) );
      domGaussian->SetAttribute("centerIndexY", ss.str());

      centerZ = centerZVec[i] / spacing[2];
      ss.str("");
      ss << centerZ; //  * spacing[2]; //static_cast<double>( static_cast<int>( centerZ / spacing[2] + 0.9999 ) );
      domGaussian->SetAttribute("centerIndexZ", ss.str());

      sigmaX = sigmaXVec[i] / spacing[0];
      ss.str("");
      ss << sigmaX; //  * spacing[0]; // static_cast<double>( static_cast<int>( sigmaX / spacing[0] + 0.9999 ) );
      domGaussian->SetAttribute("deviationX", ss.str());

      sigmaY = sigmaYVec[i] / spacing[1];
      ss.str("");
      ss << sigmaY; // * spacing[1]; //static_cast<double>( static_cast<int>( sigmaY / spacing[1] + 0.9999 ) );
      domGaussian->SetAttribute("deviationY", ss.str());

      sigmaZ = sigmaZVec[i] / spacing[2];
      ss.str("");
      ss << sigmaZ; // * spacing[2]; //static_cast<double>( static_cast<int>( sigmaZ / spacing[2] + 0.9999 ) );
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
        maxSize = size[0] - static_cast<int>((radius)/spacing[0] + 0.9999) + 1;
        minSize = 0.0 + static_cast<int>((radius)/spacing[0]+ 0.9999) - 1;
        if( minSize >= maxSize )
        {
          std::cout << "The sphere is larger then the image in the x axis!" << std::endl;

        }
        // the maximum in the x-Axis
        regionOfInterestMax.SetElement( 0, ( ROImaxSizeX[i] < maxSize ) ? ROImaxSizeX[i] : maxSize );
        // the minimum in the x-Axis
        regionOfInterestMin.SetElement( 0, ( ROIminSizeX[i] > minSize ) ? ROIminSizeX[i] : minSize );

        // y axis region of interest------------------------------------------------------
        maxSize = size[1] - static_cast<int>((radius)/spacing[1] + 0.9999) + 1;
        minSize = 0.0 + static_cast<int>((radius)/spacing[1]+ 0.9999) - 1;
        if( minSize >= maxSize )
        {
          std::cout << "The sphere is larger then the image in the y axis!" << std::endl;
        }
        // the maximum in the y-Axis
        regionOfInterestMax.SetElement( 1, ( ROImaxSizeY[i] < maxSize ) ? ROImaxSizeY[i] : maxSize );
        // the minimum in the y-Axis
        regionOfInterestMin.SetElement( 1, ( ROIminSizeY[i] > minSize ) ? ROIminSizeY[i] : minSize );

        // z axis region of interest------------------------------------------------------
        maxSize = size[2] - static_cast<int>((radius)/spacing[2] + 0.9999) + 1;
        minSize = 0.0 + static_cast<int>((radius)/spacing[2]+ 0.9999) - 1;
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
      gaussianGenerator->CalculateTheMidpointAndTheMeanValueWithOctree();

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

}



// check whether the edges of the i'th ROI is in another ROI included
bool IsInOtherROI(int i,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType ROIminSizeX,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType ROImaxSizeX,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType ROIminSizeY,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType ROImaxSizeY,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType ROIminSizeZ,
                  itk::MultiGaussianImageSource<itk::Image< double, 3>>::VectorType ROImaxSizeZ  )
{
  bool error = 0;
  std::vector<double> xBound, yBound, zBound;
  xBound.push_back( ROIminSizeX[i] );
  xBound.push_back( ROImaxSizeX[i] );
  yBound.push_back( ROIminSizeY[i] );
  yBound.push_back( ROImaxSizeY[i] );
  zBound.push_back( ROIminSizeZ[i] );
  zBound.push_back( ROImaxSizeZ[i] );
  //for each ROI
  for( unsigned int j = 0; j < i; ++j )
  {
    for( unsigned int x = 0; x < 2; ++x)
    {
      for( unsigned int y = 0; y < 2; ++y)
      {
        for( unsigned int z = 0; z < 2; ++z)
        {
          double edgeXCoord = xBound[x];
          double edgeYCoord = yBound[y];
          double edgeZCoord = zBound[z];
          // check if the edge with coordinate [edgeXCoord; edgeYCoord; edgeZCoord] is inside the j'th ROI
          if ( ROIminSizeX[j] < edgeXCoord && edgeXCoord < ROImaxSizeX[j] &&
               ROIminSizeY[j] < edgeYCoord && edgeYCoord < ROImaxSizeY[j] &&
               ROIminSizeZ[j] < edgeZCoord && edgeZCoord < ROImaxSizeZ[j])
          {
            error = 1;
            return error;
          }
        }
      }
    }
  }
  return error;
}