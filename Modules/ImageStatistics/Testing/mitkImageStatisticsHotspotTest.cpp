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
#include "mitkImageCast.h"

#include <itkImageRegionIterator.h>

#include <stdexcept>

#include <itkDOMNode.h>
#include <itkDOMReader.h>

#include <mitkHotspotMaskGenerator.h>
#include <mitkImageMaskGenerator.h>

#include <mitkIOUtil.h>

/**
 \section hotspotCalculationTestCases Testcases

 To see the different Hotspot-Testcases have a look at the \ref hotspottestdoc.

 Note from an intensive session of checking the test results:
  - itk::MultiGaussianImageSource needs a review
  - the test idea is ok, but the combination of XML files for parameters and MultiGaussianImageSource has serious flaws
    - the XML file should contain exactly the parameters that MultiGaussianImageSource requires
    - in contrast, now the XML file mentions index coordinates for gaussian centers while the MultiGaussianImageSource expects world coordinates
      - this requires a transformation (index * spacing assuming no rotation) that was actually broken until recently
*/

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
  public:

    // XML-Tag <testimage>

    /** \brief XML-Tag "image-rows": size of x-dimension  */
    int m_ImageRows;
    /** \brief  XML-Tag "image-columns": size of y-dimension  */
    int m_ImageColumns;
    /** \brief  XML-Tag "image-slices": size of z-dimension  */
    int m_ImageSlices;
    /** \brief  XML-Tag "numberOfGaussians": number of used gauss-functions */
    int m_NumberOfGaussian;

    /** \brief  XML-Tags "spacingX", "spacingY", "spacingZ": spacing of image in every direction */
    double m_Spacing[3];

    /** \brief XML-Tag "entireHotSpotInImage" */
    unsigned int m_EntireHotspotInImage;

    // XML-Tag <gaussian>

    /**
      \brief  XML-Tag "centerIndexX: gaussian parameter
      \warning This parameter READS the centerIndexX parameter from file and is THEN MISUSED to calculate some position in world coordinates, so we require double.
     */
    std::vector<double> m_CenterX;
    /** \brief  XML-Tag "centerIndexY: gaussian parameter
      \warning This parameter READS the centerIndexX parameter from file and is THEN MISUSED to calculate some position in world coordinates, so we require double.
     */
    std::vector<double> m_CenterY;
    /** \brief  XML-Tag "centerIndexZ: gaussian parameter
      \warning This parameter READS the centerIndexX parameter from file and is THEN MISUSED to calculate some position in world coordinates, so we require double.
     */
    std::vector<double> m_CenterZ;

    /** \brief  XML-Tag "deviationX: gaussian parameter  */
    std::vector<double> m_SigmaX;
    /** \brief  XML-Tag "deviationY: gaussian parameter  */
    std::vector<double> m_SigmaY;
    /** \brief  XML-Tag "deviationZ: gaussian parameter  */
    std::vector<double> m_SigmaZ;

    /** \brief  XML-Tag "altitude: gaussian parameter  */
    std::vector<double> m_Altitude;

    // XML-Tag <segmentation>

    /** \brief  XML-Tag "numberOfLabels": number of different labels which appear in the mask */
    unsigned int m_NumberOfLabels;
    /** \brief  XML-Tag "hotspotRadiusInMM": radius of hotspot */
    double m_HotspotRadiusInMM;

    // XML-Tag <roi>

    /** \brief  XML-Tag "maximumSizeX": maximum position of ROI in x-dimension */
    vnl_vector<int> m_MaxIndexX;
    /** \brief  XML-Tag "minimumSizeX": minimum position of ROI in x-dimension */
    vnl_vector<int> m_MinIndexX;
    /** \brief  XML-Tag "maximumSizeX": maximum position of ROI in y-dimension */
    vnl_vector<int> m_MaxIndexY;
    /** \brief  XML-Tag "minimumSizeX": minimum position of ROI in y-dimension */
    vnl_vector<int> m_MinIndexY;
    /** \brief  XML-Tag "maximumSizeX": maximum position of ROI in z-dimension */
    vnl_vector<int> m_MaxIndexZ;
    /** \brief  XML-Tag "minimumSizeX": minimum position of ROI in z-dimension */
    vnl_vector<int> m_MinIndexZ;

    /** \brief  XML-Tag "label": value of label */
    vnl_vector<unsigned int> m_Label;

    //XML-Tag <statistic>

    /** \brief  XML-Tag "minimum": minimum inside hotspot */
    vnl_vector<double> m_HotspotMin;
    /** \brief  XML-Tag "maximum": maximum inside hotspot */
    vnl_vector<double> m_HotspotMax;
    /** \brief  XML-Tag "mean": mean value of hotspot */
    vnl_vector<double> m_HotspotMean;

    /** \brief  XML-Tag "maximumIndexX": x-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMaxIndexX;
    /** \brief  XML-Tag "maximumIndexX": y-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMaxIndexY;
    /** \brief  XML-Tag "maximumIndexX": z-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMaxIndexZ;

    /** \brief  XML-Tag "maximumIndexX": x-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMinIndexX;
    /** \brief  XML-Tag "maximumIndexX": y-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMinIndexY;
    /** \brief  XML-Tag "maximumIndexX": z-coordinate of maximum-location inside hotspot */
    vnl_vector<int> m_HotspotMinIndexZ;

    /** \brief  XML-Tag "maximumIndexX": x-coordinate of hotspot-location */
    vnl_vector<int> m_HotspotIndexX;
    /** \brief  XML-Tag "maximumIndexX": y-coordinate of hotspot-location */
    vnl_vector<int> m_HotspotIndexY;
    /** \brief  XML-Tag "maximumIndexX": z-coordinate of hotspot-location */
    vnl_vector<int> m_HotspotIndexZ;
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
    catch(std::exception& /*e*/)
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
    catch(std::exception& /*e*/)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Convert tag value '" << attributeValue << "' for tag '" << tag << "' to double");
      return 0.0; // just to satisfy compiler
    }
  }

  /**
  \brief Read XML file describing the test parameters.

  Reads XML file given in first commandline parameter in order
  to construct a Parameters structure. The XML file should be
  structurs as the following example, i.e. we describe the
  three test aspects of Parameters in four different tags,
  with all the details described as tag attributes. */

  /**
  \verbatim
  <testcase>
  <!--
  Test case: multi-label mask
  -->

  <testimage image-rows="50" image-columns="50" image-slices="20" numberOfGaussians="2" spacingX="1" spacingY="1" spacingZ="1" entireHotSpotInImage="1">
    <gaussian centerIndexX="10" centerIndexY="10" centerIndexZ="10" deviationX="5" deviationY="5" deviationZ="5" altitude="200"/>
    <gaussian centerIndexX="40" centerIndexY="40" centerIndexZ="10" deviationX="2" deviationY="4" deviationZ="6" altitude="180"/>
  </testimage>
  <segmentation numberOfLabels="2" hotspotRadiusInMM="6.2035">
    <roi label="1" maximumSizeX="20" minimumSizeX="0" maximumSizeY="20" minimumSizeY="0" maximumSizeZ="20" minimumSizeZ="0"/>
    <roi label="2" maximumSizeX="50" minimumSizeX="30" maximumSizeY="50" minimumSizeY="30" maximumSizeZ="20" minimumSizeZ="0"/>
  </segmentation>
  <statistic hotspotIndexX="10" hotspotIndexY="10" hotspotIndexZ="10" mean="122.053" maximumIndexX="10" maximumIndexY="10" maximumIndexZ="10" maximum="200" minimumIndexX="9" minimumIndexY="9" minimumIndexZ="4" minimum="93.5333"/>
  <statistic hotspotIndexX="40" hotspotIndexY="40" hotspotIndexZ="10" mean="61.1749" maximumIndexX="40" maximumIndexY="40" maximumIndexZ="10" maximum="180" minimumIndexX="46" minimumIndexY="39" minimumIndexZ="9" minimum="1.91137"/>
 </testcase>

  \endverbatim
  */
  static Parameters ParseParameters(int argc, char* argv[])
  {
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

      result.m_EntireHotspotInImage = GetIntegerAttribute( testimage, "entireHotSpotInImage" );

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

        result.m_SigmaX[i] = GetDoubleAttribute(gaussian, "deviationX");
        result.m_SigmaY[i] = GetDoubleAttribute(gaussian, "deviationY");
        result.m_SigmaZ[i] = GetDoubleAttribute(gaussian, "deviationZ");

        result.m_Altitude[i] = GetDoubleAttribute(gaussian, "altitude");

        result.m_CenterX[i] = result.m_CenterX[i] * result.m_Spacing[0];
        result.m_CenterY[i] = result.m_CenterY[i] * result.m_Spacing[1];
        result.m_CenterZ[i] = result.m_CenterZ[i] * result.m_Spacing[2];

        result.m_SigmaX[i] = result.m_SigmaX[i] * result.m_Spacing[0];
        result.m_SigmaY[i] = result.m_SigmaY[i] * result.m_Spacing[1];
        result.m_SigmaZ[i] = result.m_SigmaZ[i] * result.m_Spacing[2];
      }

      NodeList segmentations;
      domRoot->GetChildren("segmentation", segmentations);
      MITK_TEST_CONDITION_REQUIRED( segmentations.size() == 1, "One segmentation defined");
      itk::DOMNode* segmentation = segmentations[0];

      result.m_NumberOfLabels = GetIntegerAttribute(segmentation, "numberOfLabels");
      result.m_HotspotRadiusInMM = GetDoubleAttribute(segmentation, "hotspotRadiusInMM");


      // read ROI parameters, fill result structure
      NodeList rois;
      segmentation->GetChildren("roi", rois);
      MITK_TEST_CONDITION_REQUIRED( rois.size() >= 1, "At least one ROI defined" )

      result.m_MaxIndexX.set_size(result.m_NumberOfLabels);
      result.m_MinIndexX.set_size(result.m_NumberOfLabels);
      result.m_MaxIndexY.set_size(result.m_NumberOfLabels);
      result.m_MinIndexY.set_size(result.m_NumberOfLabels);
      result.m_MaxIndexZ.set_size(result.m_NumberOfLabels);
      result.m_MinIndexZ.set_size(result.m_NumberOfLabels);
      result.m_Label.set_size(result.m_NumberOfLabels);

      for(unsigned int i = 0; i < rois.size(); ++i)
      {
        result.m_MaxIndexX[i] = GetIntegerAttribute(rois[i], "maximumIndexX");
        result.m_MinIndexX[i] = GetIntegerAttribute(rois[i], "minimumIndexX");

        result.m_MaxIndexY[i] = GetIntegerAttribute(rois[i], "maximumIndexY");
        result.m_MinIndexY[i] = GetIntegerAttribute(rois[i], "minimumIndexY");

        result.m_MaxIndexZ[i] = GetIntegerAttribute(rois[i], "maximumIndexZ");
        result.m_MinIndexZ[i] = GetIntegerAttribute(rois[i], "minimumIndexZ");

        result.m_Label[i] = GetIntegerAttribute(rois[i], "label");
      }

      // read statistic parameters, fill result structure
      NodeList statistics;
      domRoot->GetChildren("statistic", statistics);
      MITK_TEST_CONDITION_REQUIRED( statistics.size() >= 1 , "At least one statistic defined" )
      MITK_TEST_CONDITION_REQUIRED( statistics.size() == rois.size(), "Same number of rois and corresponding statistics defined");

      result.m_HotspotMin.set_size(statistics.size());
      result.m_HotspotMax.set_size(statistics.size());
      result.m_HotspotMean.set_size(statistics.size());

      result.m_HotspotMinIndexX.set_size(statistics.size());
      result.m_HotspotMinIndexY.set_size(statistics.size());
      result.m_HotspotMinIndexZ.set_size(statistics.size());

      result.m_HotspotMaxIndexX.set_size(statistics.size());
      result.m_HotspotMaxIndexY.set_size(statistics.size());
      result.m_HotspotMaxIndexZ.set_size(statistics.size());

      result.m_HotspotIndexX.set_size(statistics.size());
      result.m_HotspotIndexY.set_size(statistics.size());
      result.m_HotspotIndexZ.set_size(statistics.size());

      for(unsigned int i = 0; i < statistics.size(); ++i)
      {
        result.m_HotspotMin[i] = GetDoubleAttribute(statistics[i], "minimum");
        result.m_HotspotMax[i] = GetDoubleAttribute(statistics[i], "maximum");
        result.m_HotspotMean[i] = GetDoubleAttribute(statistics[i], "mean");

        result.m_HotspotMinIndexX[i] = GetIntegerAttribute(statistics[i], "minimumIndexX");
        result.m_HotspotMinIndexY[i] = GetIntegerAttribute(statistics[i], "minimumIndexY");
        result.m_HotspotMinIndexZ[i] = GetIntegerAttribute(statistics[i], "minimumIndexZ");

        result.m_HotspotMaxIndexX[i] = GetIntegerAttribute(statistics[i], "maximumIndexX");
        result.m_HotspotMaxIndexY[i] = GetIntegerAttribute(statistics[i], "maximumIndexY");
        result.m_HotspotMaxIndexZ[i] = GetIntegerAttribute(statistics[i], "maximumIndexZ");

        result.m_HotspotIndexX[i] = GetIntegerAttribute(statistics[i], "hotspotIndexX");
        result.m_HotspotIndexY[i] = GetIntegerAttribute(statistics[i], "hotspotIndexY");
        result.m_HotspotIndexZ[i] = GetIntegerAttribute(statistics[i], "hotspotIndexZ");
      }
    }
    catch (std::exception& e)
    {
      MITK_TEST_CONDITION_REQUIRED(false, "Reading test parameters from XML file. Error message: " << e.what());
    }

    return result;
  }

  /**
    \brief Generate an image that contains a couple of 3D gaussian distributions.

    Uses the given parameters to produce a test image using class MultiGaussianImageSource.
  */

  static mitk::Image::Pointer BuildTestImage(const Parameters& testParameters)
  {
    mitk::Image::Pointer result;

    typedef double PixelType;
    const int Dimension = 3;
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

    for( int i = 0; i < Dimension; ++i )
      spacing[i] = testParameters.m_Spacing[i];

    gaussianGenerator->SetSize( size );
    gaussianGenerator->SetSpacing( spacing );
    gaussianGenerator->SetRadius(testParameters.m_HotspotRadiusInMM);
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
  static mitk::ImageStatisticsContainer::ImageStatisticsObject CalculateStatistics(mitk::Image* image, const Parameters& testParameters,  unsigned int label)
  {
    const unsigned int Dimension = 3;
    typedef itk::Image<unsigned short, Dimension> MaskImageType;
    MaskImageType::Pointer mask = MaskImageType::New();

    MaskImageType::SizeType size;
    MaskImageType::SpacingType spacing;
    MaskImageType::IndexType start;

    mitk::ImageStatisticsCalculator::Pointer statisticsCalculator = mitk::ImageStatisticsCalculator::New();
    statisticsCalculator->SetInputImage(image);
    mitk::Image::Pointer mitkMaskImage;

    if((testParameters.m_MaxIndexX[label] > testParameters.m_MinIndexX[label] && testParameters.m_MinIndexX[label] >= 0) &&
      (testParameters.m_MaxIndexY[label] > testParameters.m_MinIndexY[label] && testParameters.m_MinIndexY[label] >= 0) &&
      (testParameters.m_MaxIndexZ[label] > testParameters.m_MinIndexZ[label] && testParameters.m_MinIndexZ[label] >= 0))
    {
      for(unsigned int i = 0; i < Dimension; ++i)
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
        maskIt.Set(0);
      }

      for(unsigned int i = 0; i < testParameters.m_NumberOfLabels; ++i)
      {

        for(maskIt.GoToBegin(); !maskIt.IsAtEnd(); ++maskIt)
        {
          MaskImageType::IndexType index = maskIt.GetIndex();

          if((index[0] >= testParameters.m_MinIndexX[i]  && index[0] <= testParameters.m_MaxIndexX[i] ) &&
             (index[1] >= testParameters.m_MinIndexY[i]  && index[1] <= testParameters.m_MaxIndexY[i] ) &&
             (index[2] >= testParameters.m_MinIndexZ[i]  && index[2] <= testParameters.m_MaxIndexZ[i] ))
          {
            maskIt.Set(testParameters.m_Label[i]);
          }
        }
      }

      mitk::CastToMitkImage(mask, mitkMaskImage);
      mitk::ImageMaskGenerator::Pointer imgMaskGen = mitk::ImageMaskGenerator::New();
      imgMaskGen->SetImageMask(mitkMaskImage);

      mitk::HotspotMaskGenerator::Pointer hotspotMaskGen = mitk::HotspotMaskGenerator::New();
      hotspotMaskGen->SetInputImage(image);
      hotspotMaskGen->SetLabel(testParameters.m_Label[label]);
      hotspotMaskGen->SetMask(imgMaskGen.GetPointer());
      hotspotMaskGen->SetHotspotRadiusInMM(testParameters.m_HotspotRadiusInMM);
      if(testParameters.m_EntireHotspotInImage == 1)
      {
        MITK_INFO << "Hotspot must be completly inside image";
        hotspotMaskGen->SetHotspotMustBeCompletelyInsideImage(true);
      }
      else
      {
        MITK_INFO << "Hotspot must not be completly inside image";
        hotspotMaskGen->SetHotspotMustBeCompletelyInsideImage(false);
      }

      statisticsCalculator->SetMask(hotspotMaskGen.GetPointer());
      MITK_DEBUG << "Masking is set to hotspot+image mask";
    }
    else
    {
      mitk::HotspotMaskGenerator::Pointer hotspotMaskGen = mitk::HotspotMaskGenerator::New();
      hotspotMaskGen->SetInputImage(image);
      hotspotMaskGen->SetHotspotRadiusInMM(testParameters.m_HotspotRadiusInMM);
      if(testParameters.m_EntireHotspotInImage == 1)
      {
        MITK_INFO << "Hotspot must be completly inside image";
        hotspotMaskGen->SetHotspotMustBeCompletelyInsideImage(true);
      }
      else
      {
        MITK_INFO << "Hotspot must not be completly inside image";
        hotspotMaskGen->SetHotspotMustBeCompletelyInsideImage(false);
      }
      MITK_DEBUG << "Masking is set to hotspot only";
    }

    return statisticsCalculator->GetStatistics()->GetStatisticsForTimeStep(0);
  }

  static void ValidateStatisticsItem(const std::string& label, double testvalue, double reference, double tolerance)
  {
    double diff = ::fabs(reference - testvalue);
    MITK_TEST_CONDITION( diff < tolerance, "'" << label << "' value close enough to reference value "
                                           "(value=" << testvalue <<
                                           ", reference=" << reference <<
                                           ", diff=" << diff << ")" );
  }

  static void ValidateStatisticsItem(const std::string& label, const vnl_vector<int>& testvalue, const vnl_vector<int>& reference)
  {
    double diffX = ::fabs(double(testvalue[0] - reference[0]));
    double diffY = ::fabs(double(testvalue[1] - reference[1]));
    double diffZ = ::fabs(double(testvalue[2] - reference[2]));

    std::stringstream testPosition;
    testPosition << testvalue[0] << "," << testvalue[1] << "," << testvalue[2];
    std::stringstream referencePosition;
    referencePosition << reference[0] << "," << reference[1] << "," << reference[2];
    MITK_TEST_CONDITION( diffX < mitk::eps && diffY < mitk::eps && diffZ < mitk::eps,
                         "'" << label << "' close enough to reference value " <<
                         "(value=[" << testPosition.str() << "]," <<
                         " reference=[" << referencePosition.str() << "]");
  }


  /**
    \brief Compares calculated against actual statistics values.

    Checks validness of all statistics aspects. Lets test fail if any aspect is not sufficiently equal.
  */
  static void ValidateStatistics(const mitk::ImageStatisticsContainer::ImageStatisticsObject hotspotStatistics, const Parameters& testParameters, unsigned int label)
  {
    // check all expected test result against actual results
    double eps = 0.25; // value above the largest tested difference

    auto mean = hotspotStatistics.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MEAN());
    auto max = hotspotStatistics.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MAXIMUM());
    auto min = hotspotStatistics.GetValueConverted<mitk::ImageStatisticsContainer::RealType>(mitk::ImageStatisticsConstants::MINIMUM());

    ValidateStatisticsItem("Hotspot mean", mean, testParameters.m_HotspotMean[label], eps);
    ValidateStatisticsItem("Hotspot maximum", max, testParameters.m_HotspotMax[label], eps);
    ValidateStatisticsItem("Hotspot minimum", min, testParameters.m_HotspotMin[label], eps);

    vnl_vector<int> referenceHotspotCenterIndex; referenceHotspotCenterIndex.set_size(3);
    referenceHotspotCenterIndex[0] = testParameters.m_HotspotIndexX[label];
    referenceHotspotCenterIndex[1] = testParameters.m_HotspotIndexY[label];
    referenceHotspotCenterIndex[2] = testParameters.m_HotspotIndexZ[label];
    // ValidateStatisticsItem("Hotspot center position", statistics.GetHotspotStatistics().GetHotspotIndex(), referenceHotspotCenterIndex); TODO: new image statistics calculator does not give hotspot position

    // TODO we do not test minimum/maximum positions within the peak/hotspot region, because
    //      these positions are not unique, i.e. there are multiple valid minima/maxima positions.
    //      One solution would be to modify the test cases in order to achive clear positions.
    //      The BETTER/CORRECT solution would be to change the singular position into a set of positions / a region
  }
};
/**
  \brief Verifies that hotspot statistics part of ImageStatisticsCalculator.

  The test reads parameters from an XML-file to generate a test-image, calculates the hotspot statistics of the image
  and checks if the calculated statistics are the same as the specified values of the XML-file.
*/
int mitkImageStatisticsHotspotTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkImageStatisticsHotspotTest")
    try {
      mitkImageStatisticsHotspotTestClass::Parameters parameters = mitkImageStatisticsHotspotTestClass::ParseParameters(argc,argv);

      mitk::Image::Pointer image = mitkImageStatisticsHotspotTestClass::BuildTestImage(parameters);
      MITK_TEST_CONDITION_REQUIRED( image.IsNotNull(), "Generate test image" );

      for(unsigned int label = 0; label < parameters.m_NumberOfLabels; ++label)
      {
        mitk::ImageStatisticsContainer::ImageStatisticsObject statistics = mitkImageStatisticsHotspotTestClass::CalculateStatistics(image, parameters, label);

        mitkImageStatisticsHotspotTestClass::ValidateStatistics(statistics, parameters, label);
        std::cout << std::endl;
      }


  }
  catch (std::exception& e)
  {
    std::cout << "Error: " <<  e.what() << std::endl;
    MITK_TEST_CONDITION_REQUIRED( false, "Exception occurred during test execution: " << e.what() );
  }
  catch(...)
  {
    MITK_TEST_CONDITION_REQUIRED( false, "Exception occurred during test execution." );
  }


  MITK_TEST_END()
}
