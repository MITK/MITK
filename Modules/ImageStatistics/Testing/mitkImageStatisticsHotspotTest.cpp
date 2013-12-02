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

#include "mitkImageStatisticsCalculator.h"
#include "itkMultiGaussianImageSource.h"
#include "mitkTestingMacros.h"

#include <itkImageRegionIterator.h>

#include <stdexcept>


#include <itkDOMNode.h>
#include <itkDOMReader.h>

struct mitkImageStatisticsHotspotTestClass
{

  /**
    \brief Test parameters for one test case.

    Describes all aspects of a single test case:
     - parameters to generate a test image
     - parameters of a ROI that describes where to calculate statistics
     - expected statistics results
  */
  struct Parameters
  {
    // XML-Tag <testimage>
    int m_ImageRows;                            // XML-Tag "image-rows"
    int m_ImageColumns;                         // XML-Tag "image-columns"
    int m_ImageSlices;                          // XML-Tag "image-slices"

    int m_NumberOfGaussian;                     // XML-Tag "numberOfGaussians"

    float m_Spacing[3];                         // XML-Tag "spacingX", "spacingY", "spacingZ"

    // XML-Tag <gaussian>
    std::vector<int> m_CenterX;                 // XML-Tag "centerIndexX"
    std::vector<int> m_CenterY;                 // XML-Tag "centerIndexY"
    std::vector<int> m_CenterZ;                 // XML-Tag "centerIndexZ"

    std::vector<int> m_SigmaX;                  // XML-Tag "deviationX"
    std::vector<int> m_SigmaY;                  // XML-Tag "deviationY"
    std::vector<int> m_SigmaZ;                  // XML-Tag "deviationZ"

    std::vector<int> m_Altitude;                // XML-Tag "altitude"

    // XML-Tag <roi>
    int m_MaxSizeX;                             // XML-Tag "maximumSizeX"
    int m_MinSizeX;                             // XML-Tag "minimumSizeX"
    int m_MaxSizeY;                             // XML-Tag "maximumSizeY"
    int m_MinSizeY;                             // XML-Tag "minimumSizeY"
    int m_MaxSizeZ;                             // XML-Tag "maximumSizeZ"
    int m_MinSizeZ;                             // XML-Tag "minimumSizeZ"

    float m_hotspotRadiusInMM;                  // XML-Tag "hotspotRadiusInMM"

    //XML-Tag <statistic>
    float m_HotspotMin;                         // XML-Tag "minimum"
    float m_HotspotMax;                         // XML-Tag "maximum"
    float m_HotspotMean;                        // XML-Tag "mean"

    vnl_vector<int> m_HotspotMaxIndex;          // XML-Tag "maximumIndexX", XML-Tag "maximumIndexY", XML-Tag "maximumIndexZ"
    vnl_vector<int> m_HotspotMinIndex;          // XML-Tag "minimumIndexX", XML-Tag "minimumIndexY", XML-Tag "minimumIndexZ"
    vnl_vector<int> m_HotspotIndex;             // XML-Tag "hotspotIndexX", XML-Tag "hotspotIndexY", XML-Tag "hotspotIndexZ"
  };

  /**
    \brief Find/Convert integer attribute in itk::DOMNode.
  */
  static int GetIntegerAttribute(itk::DOMNode* domNode, const std::string& tag)
  {
    assert(domNode);
    MITK_TEST_CONDITION_REQUIRED( domNode->HasAttribute(tag), "Tag '" << tag << "' is defined in test parameters" );
    std::string attributeValue = domNode->GetAttribute(tag);

    int resultValue;
    try
    {
      //MITK_TEST_OUTPUT( << "Converting tag value '" << attributeValue << "' for tag '" << tag << "' to integer");
      std::stringstream(attributeValue) >> resultValue;
      return resultValue;
    }
    catch(std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Convert tag value '" << attributeValue << "' for tag '" << tag << "' to integer");
      return 0; // just to satisfy compiler
    }
  }
  /**
    \brief Find/Convert double attribute in itk::DOMNode.
  */
  static double GetDoubleAttribute(itk::DOMNode* domNode, const std::string& tag)
  {
    assert(domNode);
    MITK_TEST_CONDITION_REQUIRED( domNode->HasAttribute(tag), "Tag '" << tag << "' is defined in test parameters" );
    std::string attributeValue = domNode->GetAttribute(tag);

    double resultValue;
    try
    {
      //MITK_TEST_OUTPUT( << "Converting tag value '" << attributeValue << "' for tag '" << tag << "' to double");
      std::stringstream(attributeValue) >> resultValue;
      return resultValue;
    }
    catch(std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Convert tag value '" << attributeValue << "' for tag '" << tag << "' to double");
      return 0.0; // just to satisfy compiler
    }
  }


  /**
    \brief Read XML file describging the test parameters.

    Reads XML file given in first commandline parameter in order
    to construct a Parameters structure. The XML file should be
    structurs as the following example, i.e. we describe the
    three test aspects of Parameters in three different tags,
    with all the details described as tag attributes.

\verbatim
<testcase>
  <!--
    Test case 1:
    oben links heller als dunkel (TODO!)
  -->

  <testimage
    x="1.42"
    y="2.32"
    />

  <roi
    u="3.0"
    v="4.0"
    />

  <statistics
    u="9.0"
    v="7.1"
    />
</testcase>
\verbatim

    The different parameters are interpreted as follows:
     ... TODO TODO TODO ...

  */
  static Parameters ParseParameters(int argc, char* argv[])
  {
    // - parse parameters
    // - fill ALL values of result structure
    // - if necessary, provide c'tor and default parameters to Parameters

    MITK_TEST_CONDITION_REQUIRED(argc == 2, "Test is invoked with exactly 1 parameter (XML parameters file)");
    MITK_INFO << "Reading parameters from file '" << argv[1] << "'";
    std::string filename = argv[1];

    Parameters result;

    itk::DOMNodeXMLReader::Pointer xmlReader = itk::DOMNodeXMLReader::New();
    xmlReader->SetFileName( filename );
    try
    {
      xmlReader->Update();
      itk::DOMNode::Pointer domRoot = xmlReader->GetOutput();
      typedef std::vector<itk::DOMNode*> NodeList;
      // read test image parameters, fill result structure
      NodeList testimages;
      domRoot->GetChildren("testimage", testimages);
      MITK_TEST_CONDITION_REQUIRED( testimages.size() == 1, "One test image defined" )
      itk::DOMNode* testimage = testimages[0];

      result.m_ImageRows = GetIntegerAttribute( testimage, "image-rows" );
      result.m_ImageColumns = GetIntegerAttribute( testimage, "image-columns" );
      result.m_ImageSlices = GetIntegerAttribute( testimage, "image-slices" );

      result.m_NumberOfGaussian = GetIntegerAttribute( testimage, "numberOfGaussians" );

      result.m_Spacing[0] = GetDoubleAttribute(testimage, "spacingX");
      result.m_Spacing[1] = GetDoubleAttribute(testimage, "spacingY");
      result.m_Spacing[2] = GetDoubleAttribute(testimage, "spacingZ");

      MITK_TEST_OUTPUT( << "Read size parameters (x,y,z): " << result.m_ImageRows << "," << result.m_ImageColumns << "," << result.m_ImageSlices);
      MITK_TEST_OUTPUT( << "Read spacing parameters (x,y,z): " << result.m_Spacing[0] << "," << result.m_Spacing[1] << "," << result.m_Spacing[2]);

      NodeList gaussians;
      testimage->GetChildren("gaussian", gaussians);
      MITK_TEST_CONDITION_REQUIRED( gaussians.size() >= 1, "At least one gaussian is defined" )

      result.m_CenterX.resize(result.m_NumberOfGaussian);
      result.m_CenterY.resize(result.m_NumberOfGaussian);
      result.m_CenterZ.resize(result.m_NumberOfGaussian);

      result.m_SigmaX.resize(result.m_NumberOfGaussian);
      result.m_SigmaY.resize(result.m_NumberOfGaussian);
      result.m_SigmaZ.resize(result.m_NumberOfGaussian);

      result.m_Altitude.resize(result.m_NumberOfGaussian);


      for(int i = 0; i < result.m_NumberOfGaussian ; ++i)
      {
        itk::DOMNode* gaussian = gaussians[i];

        result.m_CenterX[i] = GetIntegerAttribute(gaussian, "centerIndexX");
        result.m_CenterY[i] = GetIntegerAttribute(gaussian, "centerIndexY");
        result.m_CenterZ[i] = GetIntegerAttribute(gaussian, "centerIndexZ");

        result.m_SigmaX[i] = GetIntegerAttribute(gaussian, "deviationX");
        result.m_SigmaY[i] = GetIntegerAttribute(gaussian, "deviationY");
        result.m_SigmaZ[i] = GetIntegerAttribute(gaussian, "deviationZ");

        result.m_Altitude[i] = GetIntegerAttribute(gaussian, "altitude");
      }


      // read ROI parameters, fill result structure
      NodeList rois;
      domRoot->GetChildren("roi", rois);
      MITK_TEST_CONDITION_REQUIRED( rois.size() == 1, "One ROI defined" )
      itk::DOMNode* roi = rois[0];

      result.m_MaxSizeX = GetIntegerAttribute(roi, "maximumSizeX");
      result.m_MinSizeX = GetIntegerAttribute(roi, "minimumSizeX");
      result.m_MaxSizeY = GetIntegerAttribute(roi, "maximumSizeY");
      result.m_MinSizeY = GetIntegerAttribute(roi, "minimumSizeY");
      result.m_MaxSizeZ = GetIntegerAttribute(roi, "maximumSizeZ");
      result.m_MinSizeZ = GetIntegerAttribute(roi, "minimumSizeZ");

      result.m_hotspotRadiusInMM = GetDoubleAttribute(roi, "hotspotRadiusInMM");

      // read statistic parameters, fill result structure
      NodeList statistics;
      domRoot->GetChildren("statistic", statistics);
      MITK_TEST_CONDITION_REQUIRED( statistics.size() == 1, "One statistic defined" )
      itk::DOMNode* statistic = statistics[0];

      result.m_HotspotMin = GetDoubleAttribute(statistic, "minimum");
      result.m_HotspotMax = GetDoubleAttribute(statistic, "maximum");
      result.m_HotspotMean = GetDoubleAttribute(statistic, "mean");

      result.m_HotspotMinIndex.set_size(3);
      result.m_HotspotMinIndex[0] = GetIntegerAttribute(statistic, "minimumIndexX");
      result.m_HotspotMinIndex[1] = GetIntegerAttribute(statistic, "minimumIndexY");
      result.m_HotspotMinIndex[2] = GetIntegerAttribute(statistic, "minimumIndexZ");

      result.m_HotspotMaxIndex.set_size(3);
      result.m_HotspotMaxIndex[0] = GetIntegerAttribute(statistic, "maximumIndexX");
      result.m_HotspotMaxIndex[1] = GetIntegerAttribute(statistic, "maximumIndexY");
      result.m_HotspotMaxIndex[2] = GetIntegerAttribute(statistic, "maximumIndexZ");

      result.m_HotspotIndex.set_size(3);
      result.m_HotspotIndex[0] = GetIntegerAttribute(statistic, "hotspotIndexX");
      result.m_HotspotIndex[1] = GetIntegerAttribute(statistic, "hotspotIndexY");
      result.m_HotspotIndex[2] = GetIntegerAttribute(statistic, "hotspotIndexZ");


      return result;
    }
    catch (std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Reading test parameters from XML file. Error message: " << e.what());
    }

    if (false /* and all parameters nicely found */)
    {
      return result;
    }
    else
    {
      throw std::invalid_argument("Test called with invalid parameters.."); // TODO provide details if possible
    }
  }

  /**
    \brief Generate an image that contains a couple of 3D gaussian distributions.

    Uses the given parameters to produce a test image using class TODO... bla
  */

  static mitk::Image::Pointer BuildTestImage(const Parameters& testParameters)
  {
    // evaluate parameters, create corresponding image
    mitk::Image::Pointer result;

    typedef double PixelType;
    const unsigned int Dimension = 3;
    typedef itk::Image<PixelType, Dimension> ImageType;
    ImageType::Pointer image = ImageType::New();
    typedef itk::MultiGaussianImageSource< ImageType > MultiGaussianImageSource;
    MultiGaussianImageSource::Pointer gaussianGenerator = MultiGaussianImageSource::New();
    ImageType::SizeValueType size[3];
    size[0] = testParameters.m_ImageColumns;
    size[1] = testParameters.m_ImageRows;
    size[2] = testParameters.m_ImageSlices;

    itk::MultiGaussianImageSource<ImageType>::VectorType centerXVec, centerYVec, centerZVec, sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec;

    for(int i = 0; i < testParameters.m_NumberOfGaussian; ++i)
    {
      centerXVec.push_back(testParameters.m_CenterX[i]);
      centerYVec.push_back(testParameters.m_CenterY[i]);
      centerZVec.push_back(testParameters.m_CenterZ[i]);

      sigmaXVec.push_back(testParameters.m_SigmaX[i]);
      sigmaYVec.push_back(testParameters.m_SigmaY[i]);
      sigmaZVec.push_back(testParameters.m_SigmaZ[i]);

      altitudeVec.push_back(testParameters.m_Altitude[i]);
    }

    ImageType::SpacingType spacing;

    for(int i = 0; i < Dimension; ++i)
      spacing[i] = testParameters.m_Spacing[i];

    gaussianGenerator->SetSize( size );
    gaussianGenerator->SetSpacing( spacing );
    gaussianGenerator->SetRadiusStepNumber(5);
    gaussianGenerator->SetRadius(pow(itk::Math::one_over_pi * 0.75 , 1.0 / 3.0) * 10);
    gaussianGenerator->SetNumberOfGausssians(testParameters.m_NumberOfGaussian);

    gaussianGenerator->AddGaussian(centerXVec, centerYVec, centerZVec,
      sigmaXVec, sigmaYVec, sigmaZVec, altitudeVec);

    gaussianGenerator->Update();

    image = gaussianGenerator->GetOutput();

    mitk::CastToMitkImage(image, result);

    return result;
  }

  /**
    \brief Calculates hotspot statistics for given test image and ROI parameters.

    Uses ImageStatisticsCalculator to find a hotspot in a defined ROI within the given image.
  */
  static mitk::ImageStatisticsCalculator::Statistics CalculateStatistics(mitk::Image* image, const Parameters& testParameters)
  {
mitk::ImageStatisticsCalculator::Statistics result;
    const unsigned int Dimension = 3;
    typedef itk::Image<unsigned short, Dimension> MaskImageType;
    MaskImageType::Pointer mask = MaskImageType::New();

    MaskImageType::SizeType size;
    MaskImageType::SpacingType spacing;
    MaskImageType::IndexType start;

    mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();

    for(int i = 0; i < Dimension; ++i)
    {
      start[i] = 0;
      spacing[i] = testParameters.m_Spacing[i];
    }

    size[0] = testParameters.m_ImageColumns;
    size[1] = testParameters.m_ImageRows;
    size[2] = testParameters.m_ImageSlices;

    MaskImageType::RegionType region;
    region.SetIndex(start);
    region.SetSize(size);

    mask->SetSpacing(spacing);
    mask->SetRegions(region);
    mask->Allocate();

    typedef itk::ImageRegionIteratorWithIndex<MaskImageType> MaskImageIteratorType;
    MaskImageIteratorType maskIt(mask, region);

    for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
    {
      MaskImageType::IndexType index = maskIt.GetIndex();

       if((index[0] >= testParameters.m_MinSizeX && index[0] <= testParameters.m_MaxSizeX) &&
          (index[1] >= testParameters.m_MinSizeY && index[1] <= testParameters.m_MaxSizeY) &&
          (index[2] >= testParameters.m_MinSizeZ && index[2] <= testParameters.m_MaxSizeZ))
          maskIt.Set(1);
       else
        maskIt.Set(0);
    }

    mitk::Image::Pointer mitkMaskImage;
    mitk::CastToMitkImage(mask, mitkMaskImage);

    statisticsCalculator->SetImage(image);
    statisticsCalculator->SetImageMask(mitkMaskImage);
    statisticsCalculator->SetMaskingModeToImage();
    statisticsCalculator->SetHotspotRadius(testParameters.m_hotspotRadiusInMM);
    statisticsCalculator->SetCalculateHotspot(true);
    statisticsCalculator->ComputeStatistics();
    result = statisticsCalculator->GetStatistics();

    // create calculator object
    // fill parameters (mask, planar figure, etc.)
    // execute calculation
    // retrieve result and return from function
    // handle errors w/o crash!

    return result;
  }

  /**
    \brief Compares calculated against actual statistics values.

    Checks validness of all statistics aspects. Lets test fail if any aspect is not sufficiently equal.
  */
  static void ValidateStatistics(const mitk::ImageStatisticsCalculator::Statistics& statistics, const Parameters& testParameters)
  {
    // check all expected test result against actual results

    double eps = 0.001;

    // float comparisons, allow tiny differences
    MITK_TEST_CONDITION( ::fabs(testParameters.m_HotspotMean - statistics.HotspotMean) < eps, "Mean value of hotspot in XML-File: " << testParameters.m_HotspotMean << " (Mean value of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.HotspotMean << ")" );
    MITK_TEST_CONDITION( ::fabs(testParameters.m_HotspotMax- statistics.HotspotMax) < eps, "Maximum of hotspot in XML-File:  " << testParameters.m_HotspotMax << " (Maximum of hotspot calculated in mitkImageStatisticsCalculator: "  << statistics.HotspotMax << ")" );
    MITK_TEST_CONDITION( ::fabs(testParameters.m_HotspotMin - statistics.HotspotMin) < eps, "Minimum of hotspot in XML-File: " << testParameters.m_HotspotMin << " (Minimum of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.HotspotMin << ")" );

    MITK_TEST_CONDITION( statistics.HotspotIndex == testParameters.m_HotspotIndex, "Index of hotspot in XML-File: " << testParameters.m_HotspotIndex << " (Index of hotspot calculated in mitkImageStatisticsCalculator: " << statistics.HotspotIndex << ")" );
    MITK_TEST_CONDITION( statistics.HotspotMaxIndex == testParameters.m_HotspotMaxIndex, "Index of HotspotMaximum in XML-File: " << testParameters.m_HotspotMaxIndex << " (Index of HotspotMaximum calculated in mitkImageStatisticsCalculator: " << statistics.HotspotMaxIndex << ")" );
    MITK_TEST_CONDITION( statistics.HotspotMinIndex == testParameters.m_HotspotMinIndex, "Index of HotspotMinimum in XML-File " << testParameters.m_HotspotMinIndex << " (Index of HotspotMinimum calculated in mitkImageStatisticsCalculator: " << statistics.HotspotMinIndex << ")" );
  }
};


#include <itkTimeProbe.h>

/**
  \brief Verifies that TODO hotspot statistics part of ImageStatisticsCalculator.

  bla...
*/
int mitkImageStatisticsHotspotTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkImageStatisticsHotspotTest")
  try {
  // parse commandline parameters (see CMakeLists.txt)
  mitkImageStatisticsHotspotTestClass::Parameters parameters = mitkImageStatisticsHotspotTestClass::ParseParameters(argc,argv);

  // build a test image as described in parameters
  mitk::Image::Pointer image = mitkImageStatisticsHotspotTestClass::BuildTestImage(parameters);
  MITK_TEST_CONDITION_REQUIRED( image.IsNotNull(), "Generate test image" );

  itk::TimeProbe clock;
  clock.Start();

  // calculate statistics for this image (potentially use parameters for statistics ROI)
  mitk::ImageStatisticsCalculator::Statistics statistics = mitkImageStatisticsHotspotTestClass::CalculateStatistics(image, parameters);

  clock.Stop();
  std::cout << "Statistics time consumed: " << clock.GetTotal() << std::endl;
  // compare statistics against stored expected values
  mitkImageStatisticsHotspotTestClass::ValidateStatistics(statistics, parameters);

  }
  catch (std::exception& e)
  {
    std::cout << "Error: " <<  e.what() << std::endl;
  }


  MITK_TEST_END()
}
