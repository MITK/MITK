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

#include <mitkTestingMacros.h>
#include <mitkTestingConfig.h>
#include <mitkTestFixture.h>
#include <mitkManualSegmentationToSurfaceFilter.h>
#include <mitkIOUtil.h>

class mitkManualSegmentationToSurfaceFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkManualSegmentationToSurfaceFilterTestSuite);
  //Add tests with reference data.
  //For now I only add reference for BallBinary, since
  //other images do not really make sense.
  //Note: .stl consumes much more memory and even loses
  //some vertices and triangles during saving. Hence,
  //.vtp is preferred as reference format.
  MITK_PARAMETERIZED_TEST_2(Update_BallBinary_OutputEqualsReference,
                            "BallBinary30x30x30.nrrd",
                            "BallBinary30x30x30Reference.vtp");
  MITK_PARAMETERIZED_TEST_2(Update_BallBinaryAndSmooth_OutputEqualsReference,
                            "BallBinary30x30x30.nrrd",
                            "BallBinary30x30x30SmoothReference.vtp");
  CPPUNIT_TEST_SUITE_END();

private:

  mitk::ManualSegmentationToSurfaceFilter::Pointer m_Filter;
  mitk::Surface::Pointer m_ReferenceSurface;

public:

  void setUp() override
  {
    std::vector<std::string> parameter = GetTestParameter();
    m_Filter = mitk::ManualSegmentationToSurfaceFilter::New();
    if(parameter.size() == 2)
    {
      m_Filter->SetInput(mitk::IOUtil::LoadImage(GetTestDataFilePath(parameter.at(0))));
      //For the tests which have reference data
      m_ReferenceSurface = mitk::IOUtil::LoadSurface(GetTestDataFilePath(parameter.at(1)));
    }
    else
    {
      MITK_WARN << "No test run for parameter for unknown parameter.";
    }
  }

  void tearDown() override
  {
    m_Filter = NULL;
    m_ReferenceSurface = NULL;
  }

  void Update_BallBinary_OutputEqualsReference()
  {
    m_Filter->Update();
    mitk::Surface::Pointer computedOutput = m_Filter->GetOutput();
    MITK_ASSERT_EQUAL(computedOutput, m_ReferenceSurface, "Computed equals the reference?");
  }

  void Update_BallBinaryAndSmooth_OutputEqualsReference()
  {
    m_Filter->MedianFilter3DOn();
    m_Filter->SetGaussianStandardDeviation(1.5);
    m_Filter->InterpolationOn();
    m_Filter->UseGaussianImageSmoothOn();
    m_Filter->SetThreshold( 1 );
    m_Filter->SetDecimate( mitk::ImageToSurfaceFilter::DecimatePro );
    m_Filter->SetTargetReduction(0.05f);
    m_Filter->SmoothOn();
    m_Filter->Update();
    mitk::Surface::Pointer computedOutput = m_Filter->GetOutput();

    MITK_ASSERT_EQUAL(computedOutput, m_ReferenceSurface, "Computed equals the reference?");
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkManualSegmentationToSurfaceFilter)

