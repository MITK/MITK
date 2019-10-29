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
// Testing
#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

//MITK includes
#include <mitkIOUtil.h>
#include <mitkCropTimestepsImageFilter.h>

class mitkCropTimestepsImageFilterTestSuite : public mitk::TestFixture
{
	CPPUNIT_TEST_SUITE(mitkCropTimestepsImageFilterTestSuite);
  MITK_TEST(Constructor_Null);
  MITK_TEST(Constructor_NoTime);
  MITK_TEST(Setter_UpperBoundaryTimestepSmallerThanLowerBoundaryTimestep);
  MITK_TEST(Setter_UpperTimestepsGreaterThanMaxImageTimestep);
  MITK_TEST(Filter_Default);
  MITK_TEST(Filter_OnlyLowerBoundary);
  MITK_TEST(Filter_OnlyUpperBoundary);
  MITK_TEST(Filter_BothBoundaries);
  MITK_TEST(Filter_BothBoundaries2Dt);
	CPPUNIT_TEST_SUITE_END();
private:
    std::string m_ImageFilename2D, m_ImageFilename3D, m_ImageFilename2Dt, m_ImageFilename3Dt;
    mitk::CropTimestepsImageFilter::Pointer m_cropTimestepsFilter;
public:
	void setUp() override
	{
    m_ImageFilename2D = GetTestDataFilePath("Png2D-bw.png");
    m_ImageFilename3D = GetTestDataFilePath("Pic3D.nrrd");
    m_ImageFilename2Dt = GetTestDataFilePath("Pic2DplusT.nrrd");
    m_ImageFilename3Dt = GetTestDataFilePath("3D+t-ITKIO-TestData/LinearModel_4D_arbitrary_time_geometry.nrrd");
    m_cropTimestepsFilter = mitk::CropTimestepsImageFilter::New();
	}

	void tearDown() override
	{
	}

  void Constructor_Null()
	{
    m_cropTimestepsFilter->SetInput(nullptr);
    CPPUNIT_ASSERT_THROW(m_cropTimestepsFilter->Update(), itk::ExceptionObject);
	}

  void Constructor_NoTime()
  {
    auto refImage2D = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename2D);
    auto refImage3D = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename3D);
    m_cropTimestepsFilter->SetInput(refImage2D);
    CPPUNIT_ASSERT_THROW(m_cropTimestepsFilter->Update(), mitk::Exception);
    m_cropTimestepsFilter->SetInput(refImage3D);
    CPPUNIT_ASSERT_THROW(m_cropTimestepsFilter->Update(), mitk::Exception);
  }

  void Setter_UpperBoundaryTimestepSmallerThanLowerBoundaryTimestep()
  {
    //check for Exception if UpperBoundaryTimestep < LowerBoundaryTimestep
    auto refImage2Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename2Dt);
    m_cropTimestepsFilter->SetInput(refImage2Dt);
    unsigned int lowerTimeStep = 5;
    unsigned int upperTimeStep = 4;
    m_cropTimestepsFilter->SetLowerBoundaryTimestep(lowerTimeStep);
    m_cropTimestepsFilter->SetUpperBoundaryTimestep(upperTimeStep);
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetLowerBoundaryTimestep(), lowerTimeStep);
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetUpperBoundaryTimestep(), upperTimeStep);
    CPPUNIT_ASSERT_THROW(m_cropTimestepsFilter->Update(), mitk::Exception);
  }

  void Setter_UpperTimestepsGreaterThanMaxImageTimestep()
  {
    //check for correction if UpperBoundaryTimestep > image->GetTimestep()
    auto refImage2Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename2Dt);
    m_cropTimestepsFilter->SetInput(refImage2Dt);
    unsigned int upperTimeStep = 11;
    m_cropTimestepsFilter->SetUpperBoundaryTimestep(upperTimeStep);
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetUpperBoundaryTimestep(), upperTimeStep);
    CPPUNIT_ASSERT_NO_THROW(m_cropTimestepsFilter->Update());
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetUpperBoundaryTimestep(), refImage2Dt->GetTimeSteps());
  }

  void Filter_Default()
  {
    //Everything default: timesteps should stay the same
    auto refImage3Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename3Dt);
    auto timeGeometryBefore = refImage3Dt->GetTimeGeometry()->Clone();
    m_cropTimestepsFilter->SetInput(refImage3Dt);
    unsigned int expectedLowerTimestep = 0;
    unsigned int expectedUpperTimestep = std::numeric_limits<unsigned int>::max();
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetLowerBoundaryTimestep(), expectedLowerTimestep);
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetUpperBoundaryTimestep(), expectedUpperTimestep);
    CPPUNIT_ASSERT_NO_THROW(m_cropTimestepsFilter->Update());
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetLowerBoundaryTimestep(), expectedLowerTimestep);
    CPPUNIT_ASSERT_EQUAL(m_cropTimestepsFilter->GetUpperBoundaryTimestep(), refImage3Dt->GetTimeSteps());
    auto result = m_cropTimestepsFilter->GetOutput();
    auto timeGeometry = result->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->GetMinimumTimePoint(), timeGeometry->GetMinimumTimePoint());
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->GetMaximumTimePoint(), timeGeometry->GetMaximumTimePoint());

    CPPUNIT_ASSERT_EQUAL(result->GetTimeSteps(), refImage3Dt->GetTimeSteps());
  }

	void Filter_OnlyLowerBoundary()
	{
    //Crop lower 2 timesteps
    auto refImage3Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename3Dt);
    MITK_WARN << "before: " << *refImage3Dt;
    auto timeGeometryBefore = refImage3Dt->GetTimeGeometry()->Clone();
    m_cropTimestepsFilter->SetInput(refImage3Dt);
    unsigned int lowerTimestep = 2;
    m_cropTimestepsFilter->SetLowerBoundaryTimestep(lowerTimestep);
    CPPUNIT_ASSERT_NO_THROW(m_cropTimestepsFilter->Update());
    auto result = m_cropTimestepsFilter->GetOutput();
    auto timeGeometry = result->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->TimeStepToTimePoint(lowerTimestep), timeGeometry->GetMinimumTimePoint());
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->GetMaximumTimePoint(), timeGeometry->GetMaximumTimePoint());
    CPPUNIT_ASSERT_EQUAL(refImage3Dt->GetTimeSteps()-lowerTimestep, result->GetTimeSteps());
	}

  void Filter_OnlyUpperBoundary()
  {
    // Crop upper 3 timesteps
    auto refImage3Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename3Dt);
    auto timeGeometryBefore = refImage3Dt->GetTimeGeometry()->Clone();
    m_cropTimestepsFilter->SetInput(refImage3Dt);
    unsigned int upperTimestep = 7;
    m_cropTimestepsFilter->SetUpperBoundaryTimestep(upperTimestep);
    CPPUNIT_ASSERT_NO_THROW(m_cropTimestepsFilter->Update());
    auto result = m_cropTimestepsFilter->GetOutput();
    auto timeGeometry = result->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->TimeStepToTimePoint(upperTimestep), timeGeometry->GetMaximumTimePoint());
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->GetMinimumTimePoint(), timeGeometry->GetMinimumTimePoint());
    CPPUNIT_ASSERT_EQUAL(upperTimestep, result->GetTimeSteps());
  }

  void Filter_BothBoundaries()
  {
    //Crop lower 2 and upper 3 timesteps
    auto refImage3Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename3Dt);
    m_cropTimestepsFilter->SetInput(refImage3Dt);
    auto timeGeometryBefore = refImage3Dt->GetTimeGeometry()->Clone();
    unsigned int lowerTimestep = 1;
    unsigned int upperTimestep = 7;
    m_cropTimestepsFilter->SetLowerBoundaryTimestep(lowerTimestep);
    m_cropTimestepsFilter->SetUpperBoundaryTimestep(upperTimestep);
    CPPUNIT_ASSERT_NO_THROW(m_cropTimestepsFilter->Update());
    auto result = m_cropTimestepsFilter->GetOutput();
    auto timeGeometry = result->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->TimeStepToTimePoint(upperTimestep), timeGeometry->GetMaximumTimePoint());
    CPPUNIT_ASSERT_EQUAL(timeGeometryBefore->TimeStepToTimePoint(lowerTimestep), timeGeometry->GetMinimumTimePoint());
    CPPUNIT_ASSERT_EQUAL(upperTimestep-lowerTimestep, result->GetTimeSteps());
  }

  void Filter_BothBoundaries2Dt()
  {
    //Crop lower 1 and upper 1 timestep, resulting in 1 remaining timestep (2D+t input)
    auto refImage2Dt = mitk::IOUtil::Load<mitk::Image>(m_ImageFilename2Dt);
    m_cropTimestepsFilter->SetInput(refImage2Dt);
    unsigned int lowerTimestep = 1;
    unsigned int upperTimestep = 2;
    m_cropTimestepsFilter->SetLowerBoundaryTimestep(lowerTimestep);
    m_cropTimestepsFilter->SetUpperBoundaryTimestep(upperTimestep);
    CPPUNIT_ASSERT_NO_THROW(m_cropTimestepsFilter->Update());
    auto result = m_cropTimestepsFilter->GetOutput();
    auto timeGeometry = result->GetTimeGeometry();
    CPPUNIT_ASSERT_EQUAL(mitk::TimePointType(upperTimestep), timeGeometry->GetMaximumTimePoint());
    CPPUNIT_ASSERT_EQUAL(mitk::TimePointType(lowerTimestep), timeGeometry->GetMinimumTimePoint());
    CPPUNIT_ASSERT_EQUAL(upperTimestep - lowerTimestep, result->GetTimeSteps());
  }

};
MITK_TEST_SUITE_REGISTRATION(mitkCropTimestepsImageFilter)
