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
    int m_ImageRows;                          // XML-Tag "image-rows"
    int m_ImageColumns;                       // XML-Tag "image-columns"
    int m_ImageSlices;                        // XML-Tag "image-slices"
    int m_NumberOfGaussian;                   // XML-Tag "numberOfGaussians"

    // XML-Tag <gaussian>
    std::vector<int> m_CenterX;                // XML-Tag "centerIndexX"
    std::vector<int> m_CenterY;                // XML-Tag "centerIndexY"
    std::vector<int> m_CenterZ;                // XML-Tag "centerIndexZ"

    std::vector<int> m_SigmaX;                 // XML-Tag "deviationX"
    std::vector<int> m_SigmaY;                 // XML-Tag "deviationY"
    std::vector<int> m_SigmaZ;                 // XML-Tag "deviationZ"

    std::vector<int> m_Altitude;               // XML-Tag "altitude"

    // XML-Tag <roiY
    int m_RoiMaximumX;                        // XML-Tag "maximumX"
    int m_RoiMinimumX;                        // XML-Tag "minimumX"
    int m_RoiMaximumY;                        // XML-Tag "maximumY"
    int m_RoiMinimumY;                        // XML-Tag "minimumY"
    int m_RoiMaximumZ;                        // XML-Tag "maximumZ"
    int m_RoiMinimumZ;                        // XML-Tag "minimumZ"

    //XML-Tag <statistic>
    float m_HotspotMinimum;                   // XML-Tag "minimum"
    float m_HotspotMaximum;                   // XML-Tag "maximum"
    float m_HotspotPeak;                      // XML-Tag "peak"

    std::vector<int> m_HotspotMaximumIndex;   // XML-Tag "maximumIndexX", XML-Tag "maximumIndexY", XML-Tag "maximumIndexZ"
    std::vector<int> m_HotspotMinimumIndex;   // XML-Tag "minimumIndexX", XML-Tag "minimumIndexY", XML-Tag "minimumIndexZ"
    std::vector<int> m_HotspotPeakIndex;      // XML-Tag "peakIndexX", XML-Tag "peakIndexY", XML-Tag "peakIndexZ"
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

      MITK_TEST_OUTPUT( << "Read size parameters (x,y,z): " << result.m_ImageRows << "," << result.m_ImageColumns << "," << result.m_ImageSlices);

      NodeList gaussians;
      testimage->GetChildren("gaussian", gaussians);
      MITK_TEST_CONDITION_REQUIRED( gaussians.size() >= 1, "At least one gaussian is defined" )

      std::vector<int> tmpCenterX(result.m_NumberOfGaussian,0);
      std::vector<int> tmpCenterY(result.m_NumberOfGaussian,0);
      std::vector<int> tmpCenterZ(result.m_NumberOfGaussian,0);

      std::vector<int> tmpSigmaX(result.m_NumberOfGaussian,0);
      std::vector<int> tmpSigmaY(result.m_NumberOfGaussian,0);
      std::vector<int> tmpSigmaZ(result.m_NumberOfGaussian,0);

      std::vector<int> tmpAltitude(result.m_NumberOfGaussian,0);

      for(int i = 0; i < result.m_NumberOfGaussian ; ++i)
      {
        itk::DOMNode* gaussian = gaussians[i];

        tmpCenterX[i] = GetIntegerAttribute(gaussian, "centerIndexX");
        tmpCenterY[i] = GetIntegerAttribute(gaussian, "centerIndexY");
        tmpCenterZ[i] = GetIntegerAttribute(gaussian, "centerIndexZ");

        tmpSigmaX[i] = GetIntegerAttribute(gaussian, "deviationX");
        tmpSigmaY[i] = GetIntegerAttribute(gaussian, "deviationY");
        tmpSigmaZ[i] = GetIntegerAttribute(gaussian, "deviationZ");

        tmpAltitude[i] = GetIntegerAttribute(gaussian, "altitude");
      }

      result.m_CenterX = tmpCenterX;
      result.m_CenterY = tmpCenterY;
      result.m_CenterZ = tmpCenterZ;

      result.m_SigmaX = tmpSigmaX;
      result.m_SigmaY = tmpSigmaY;
      result.m_SigmaZ = tmpSigmaZ;

      result.m_Altitude = tmpAltitude;

      // read ROI parameters, fill result structure
      NodeList rois;
      domRoot->GetChildren("roi", rois);
      MITK_TEST_CONDITION_REQUIRED( rois.size() == 1, "One ROI defined" )
      itk::DOMNode* roi = rois[0];

      result.m_RoiMaximumX = GetIntegerAttribute(roi, "maximumX");
      result.m_RoiMinimumX = GetIntegerAttribute(roi, "minimumX");
      result.m_RoiMaximumY = GetIntegerAttribute(roi, "maximumY");
      result.m_RoiMinimumY = GetIntegerAttribute(roi, "minimumY");
      result.m_RoiMaximumZ = GetIntegerAttribute(roi, "maximumZ");
      result.m_RoiMinimumZ = GetIntegerAttribute(roi, "minimumZ");

      // read statistic parameters, fill result structure
      NodeList statistics;
      domRoot->GetChildren("statistic", statistics);
      MITK_TEST_CONDITION_REQUIRED( statistics.size() == 1, "One statistic defined" )
      itk::DOMNode* statistic = statistics[0];

      result.m_HotspotMinimum = GetDoubleAttribute(statistic, "minimum");
      result.m_HotspotMaximum = GetDoubleAttribute(statistic, "maximum");
      result.m_HotspotPeak = GetDoubleAttribute(statistic, "peak");

      std::vector<int> tmpMinimumIndex(3,0);

      tmpMinimumIndex[0] = GetIntegerAttribute(statistic, "minimumIndexX");
      tmpMinimumIndex[1] = GetIntegerAttribute(statistic, "minimumIndexY");
      tmpMinimumIndex[2] = GetIntegerAttribute(statistic, "minimumIndexZ");

      result.m_HotspotMinimumIndex = tmpMinimumIndex;


      std::vector<int> tmpMaximumIndex(3,0);

      tmpMaximumIndex[0] = GetIntegerAttribute(statistic, "maximumIndexX");
      tmpMaximumIndex[1] = GetIntegerAttribute(statistic, "maximumIndexY");
      tmpMaximumIndex[2] = GetIntegerAttribute(statistic, "maximumIndexZ");

      result.m_HotspotMaximumIndex = tmpMaximumIndex;

      std::vector<int> tmpPeakIndex(3,0);

      tmpPeakIndex[0] = GetIntegerAttribute(statistic, "peakIndexX");
      tmpPeakIndex[1] = GetIntegerAttribute(statistic, "peakIndexY");
      tmpPeakIndex[2] = GetIntegerAttribute(statistic, "peakIndexZ");

      result.m_HotspotPeakIndex = tmpPeakIndex;

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

    gaussianGenerator->SetSize( size );
    gaussianGenerator->SetSpacing( 1 );
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

    for(int i = 0; i < 3; ++i)
    {
      spacing[i] = 1.00;
      start[i] = 0.00;
    }

    size[0] = testParameters.m_ImageColumns;
    size[1] = testParameters.m_ImageRows;
    size[2] = testParameters.m_ImageSlices;

    MaskImageType::RegionType region;
    region.SetIndex(start);
    region.SetSize(size);

    mask->SetRegions(region);
    mask->Allocate();

    for(int x = testParameters.m_RoiMinimumX; x < testParameters.m_RoiMaximumX; ++x)
    {
      for(int y = testParameters.m_RoiMinimumY; y < testParameters.m_RoiMaximumY; ++y)
      {
        for(int z = testParameters.m_RoiMinimumZ; z < testParameters.m_RoiMaximumZ; ++z)
        {
          MaskImageType::IndexType pixelIndex;
          pixelIndex[0] = x;
          pixelIndex[1] = y;
          pixelIndex[2] = z;

          mask->SetPixel(pixelIndex, 1.00);
        }
      }
    }

    mitk::Image::Pointer mitkMaskImage;
    mitk::CastToMitkImage(mask, mitkMaskImage);

    statisticsCalculator->SetImage(image);
    statisticsCalculator->SetImageMask(mitkMaskImage);
    statisticsCalculator->SetMaskingModeToImage();
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

    double actualPeakValue = testParameters.m_HotspotPeak;
    double expectedPeakValue = statistics.HotspotPeak;

    double actualMaxValue = testParameters.m_HotspotMaximum;
    double expectedMaxValue = statistics.HotspotMax;

    double actualMinValue = testParameters.m_HotspotMinimum;
    double expectedMinValue = statistics.HotspotMin;


    //Peak Index
    std::vector<int> actualPeakIndex = testParameters.m_HotspotPeakIndex;
    vnl_vector<int> expectedVnlPeakIndex;
    expectedVnlPeakIndex = statistics.HotspotPeakIndex;

    vnl_vector<int> actualVnlPeakIndex;
    actualVnlPeakIndex.set_size(3);

    for(int i = 0; i < actualVnlPeakIndex.size(); ++i)
      actualVnlPeakIndex[i] = actualPeakIndex[i];

    // MaxIndex
    std::vector<int> actualMaxIndex = testParameters.m_HotspotMaximumIndex;
    vnl_vector<int> expectedVnlMaxIndex;
    expectedVnlMaxIndex = statistics.HotspotMaxIndex;

    vnl_vector<int> actualVnlMaxIndex;
    actualVnlMaxIndex.set_size(3);

    for(int i = 0; i < actualVnlMaxIndex.size(); ++i)
      actualVnlMaxIndex[i] = actualMaxIndex[i];

    //MinIndex
    std::vector<int> actualMinIndex = testParameters.m_HotspotMinimumIndex;
    vnl_vector<int> expectedVnlMinIndex;
    expectedVnlMinIndex = statistics.HotspotMinIndex;

    vnl_vector<int> actualVnlMinIndex;
    actualVnlMinIndex.set_size(3);

    for(int i = 0; i < actualVnlMinIndex.size(); ++i)
      actualVnlMinIndex[i] = actualMinIndex[i];

    double eps = 0.001;

    // float comparisons, allow tiny differences
    MITK_TEST_CONDITION( ::fabs(actualPeakValue - expectedPeakValue) < eps, "Actual hotspotPeak value " << actualPeakValue << " (expected " << expectedPeakValue << ")" );
    MITK_TEST_CONDITION( ::fabs(actualMaxValue - expectedMaxValue) < eps, "Actual hotspotMax value " << actualMaxValue << " (expected " << expectedMaxValue << ")" );
    MITK_TEST_CONDITION( ::fabs(actualMinValue - expectedMinValue) < eps, "Actual hotspotMin value " << actualMinValue << " (expected " << expectedMinValue << ")" );

    MITK_TEST_CONDITION( expectedVnlPeakIndex == actualVnlPeakIndex, "Actual hotspotPeakIndex " << actualVnlPeakIndex << " (expected " << expectedVnlPeakIndex << ")" );
    MITK_TEST_CONDITION( expectedVnlMaxIndex == actualVnlMaxIndex, "Actual hotspotMaxIndex " << actualVnlMaxIndex << " (expected " << expectedVnlMaxIndex << ")" );
    MITK_TEST_CONDITION( expectedVnlMinIndex == actualVnlMinIndex, "Actual hotspotMinIndex " << actualVnlMinIndex << " (expected " << expectedVnlMinIndex << ")" );
  }
};

/**
  \brief Verifies that TODO hotspot statistics part of ImageStatisticsCalculator.

  bla...
*/
int mitkImageStatisticsHotspotTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkImageStatisticsHotspotTest")

  // parse commandline parameters (see CMakeLists.txt)
  mitkImageStatisticsHotspotTestClass::Parameters parameters = mitkImageStatisticsHotspotTestClass::ParseParameters(argc,argv);

  // build a test image as described in parameters
  mitk::Image::Pointer image = mitkImageStatisticsHotspotTestClass::BuildTestImage(parameters);
  MITK_TEST_CONDITION_REQUIRED( image.IsNotNull(), "Generate test image" );

  // calculate statistics for this image (potentially use parameters for statistics ROI)
  mitk::ImageStatisticsCalculator::Statistics statistics = mitkImageStatisticsHotspotTestClass::CalculateStatistics(image, parameters);

  // compare statistics against stored expected values
  mitkImageStatisticsHotspotTestClass::ValidateStatistics(statistics, parameters);

  MITK_TEST_END()
}
