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

#include "mitkTestingMacros.h"

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
    int m_ExpectedValueU;
    int m_ExpectedValueV;

    int m_ParameterX;
    int m_ParameterY;
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

      // read test image parameters, fill result structure
      std::vector<itk::DOMNode*> testimages;
      domRoot->GetChildren("testimage", testimages);
      MITK_TEST_CONDITION_REQUIRED( testimages.size() == 1, "One test image defined" )
      itk::DOMNode* testimage = testimages[0];

      int x = GetIntegerAttribute( testimage, "x" );
      int y = GetIntegerAttribute( testimage, "y" );

      MITK_TEST_OUTPUT( << "Read integer parameters (x,y)=" << x << "," << y);

      double xd = GetDoubleAttribute( testimage, "x" );
      double yd = GetDoubleAttribute( testimage, "y" );

      MITK_TEST_OUTPUT( << "Read double parameters (x,y)=" << xd << "," << yd);


      // read ROI parameters, fill result structure
      std::vector<itk::DOMNode*> rois;
      domRoot->GetChildren("roi", rois);
      MITK_TEST_CONDITION_REQUIRED( rois.size() == 1, "One ROI defined" )
      itk::DOMNode* roi = rois[0];

      double roiU = GetDoubleAttribute( roi, "u" );
      double roiV = GetDoubleAttribute( roi, "v" );

      double roiUndefined = GetDoubleAttribute( roi, "imWagenVorMir" );
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
    mitk::Image::Pointer result;

    // evaluate parameters, create corresponding image

    return result;
  }

  /**
    \brief Calculates hotspot statistics for given test image and ROI parameters.

    Uses ImageStatisticsCalculator to find a hotspot in a defined ROI within the given image.
  */
  static mitk::ImageStatisticsCalculator::Statistics CalculateStatistics(mitk::Image* image, const Parameters& testParameters)
  {
    mitk::ImageStatisticsCalculator::Statistics result;

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

    double actualValue = 0.0;
    double expectedValue = testParameters.m_ExpectedValueU;

    // float comparisons, allow tiny differences
    MITK_TEST_CONDITION( ::fabs(actualValue - expectedValue) < mitk::eps, "Calculated statistics value " << actualValue << " (expected " << expectedValue << ")" );
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
