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
#include <mitkSurface.h>
#include <mitkIOUtil.h>
#include <vtkCleanPolyData.h>
#include <mitkTestFixture.h>


#include "mitkAnisotropicIterativeClosestPointRegistration.h"
#include "mitkCovarianceMatrixCalculator.h"
#include "mitkAnisotropicRegistrationCommon.h"

/**
 * Test to verify the results of the A-ICP registration.
 * The test runs the standard A-ICP and the trimmed variant.
 */
class mitkAnisotropicIterativeClosestPointRegistrationTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkAnisotropicIterativeClosestPointRegistrationTestSuite);
  MITK_TEST(testAicpRegistration);
  MITK_TEST(testTrimmedAicpregistration);
  CPPUNIT_TEST_SUITE_END();

private:

  typedef itk::Matrix < double, 3, 3 > Matrix3x3;
  typedef itk::Vector < double, 3 > Vector3;
  typedef std::vector < Matrix3x3 > CovarianceMatrixList;

  mitk::Surface::Pointer m_X;
  mitk::Surface::Pointer m_Y;

  mitk::PointSet::Pointer m_TargetsX;
  mitk::PointSet::Pointer m_TargetsY;

  CovarianceMatrixList m_SigmasX;
  CovarianceMatrixList m_SigmasY;

  double m_FRENormalizationFactor;

public:

  /**
   * @brief Setup Always call this method before each Test-case to ensure
   * correct and new intialization of the used members for a new test case.
   * (If the members are not used in a test, the method does not need to be called).
   */
  void setUp()
  {
    mitk::CovarianceMatrixCalculator::Pointer matrixCalculator
                                      = mitk::CovarianceMatrixCalculator::New();

    m_X = mitk::IOUtil::LoadSurface(GetTestDataFilePath("RenderingTestData/Stanford_bunny.stl"));
    m_Y = mitk::IOUtil::LoadSurface(GetTestDataFilePath("SurfaceRegistration/bunny_scaled_3000.stl"));

    m_TargetsX = mitk::IOUtil::LoadPointSet(GetTestDataFilePath("SurfaceRegistration/bunny_target_point_1000_ICP.mps"));
    m_TargetsY = mitk::IOUtil::LoadPointSet(GetTestDataFilePath("SurfaceRegistration/bunny_target_point_3000.mps"));


    matrixCalculator->SetInputSurface(m_X);
    matrixCalculator->ComputeCovarianceMatrices();
    m_SigmasX = matrixCalculator->GetCovarianceMatrices();
    const double meanVarX = matrixCalculator->GetMeanVariance();

    matrixCalculator->SetInputSurface(m_Y);
    matrixCalculator->ComputeCovarianceMatrices();
    m_SigmasY = matrixCalculator->GetCovarianceMatrices();
    const double meanVarY = matrixCalculator->GetMeanVariance();

    m_FRENormalizationFactor = sqrt( meanVarX + meanVarY );
  }

  void tearDown()
  {
    m_X = NULL;
    m_Y = NULL;

    m_TargetsX = NULL;
    m_TargetsY = NULL;

    m_SigmasX.clear();
    m_SigmasY.clear();
  }

  void testAicpRegistration()
  {
    const double expTRE = 0.0811035;
    const double expFRE = 3.15788;
    mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
                      mitk::AnisotropicIterativeClosestPointRegistration::New();

    // set up parameters
    aICP->SetMovingSurface(m_X);
    aICP->SetFixedSurface(m_Y);
    aICP->SetCovarianceMatricesMovingSurface(m_SigmasX);
    aICP->SetCovarianceMatricesFixedSurface(m_SigmasY);
    aICP->SetFRENormalizationFactor(m_FRENormalizationFactor);
    aICP->SetThreshold(0.000001);

    // run the algorithm
    aICP->Update();

    MITK_INFO << "FRE: Expected: " << expFRE << ", computed: " << aICP->GetFRE();
    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test FRE",
                           mitk::Equal(aICP->GetFRE(),expFRE,0.0001));

    // compute the target registration Error
    const double tre = mitk::AnisotropicRegistrationCommon::ComputeTargetRegistrationError(
                                                                                     m_TargetsX.GetPointer(),
                                                                                     m_TargetsY.GetPointer(),
                                                                                     aICP->GetRotation(),
                                                                                     aICP->GetTranslation()
                                                                                   );

    MITK_INFO << "TRE: Expected: " << expTRE << ", computed: " << tre;
    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test TRE",
                           mitk::Equal(tre,expTRE,0.0001));
  }

  void testTrimmedAicpregistration()
  {
    const double expTRE = 0.0553922;
    const double expFRE = 1.95305;
    mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
                      mitk::AnisotropicIterativeClosestPointRegistration::New();

    // set up parameters
    aICP->SetMovingSurface(m_X);
    aICP->SetFixedSurface(m_Y);
    aICP->SetCovarianceMatricesMovingSurface(m_SigmasX);
    aICP->SetCovarianceMatricesFixedSurface(m_SigmasY);
    aICP->SetFRENormalizationFactor(m_FRENormalizationFactor);
    aICP->SetThreshold(0.000001);
    aICP->SetTrimmFactor(0.5);

    // run the algorithm
    aICP->Update();

    MITK_INFO << "FRE: Expected: " << expFRE << ", computed: " << aICP->GetFRE();

    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test FRE",
                           mitk::Equal(aICP->GetFRE(),expFRE,0.0001));

    // compute the target registration Error
    const double tre = mitk::AnisotropicRegistrationCommon::ComputeTargetRegistrationError( m_TargetsX.GetPointer(),
                                                                                     m_TargetsY.GetPointer(),
                                                                                     aICP->GetRotation(),
                                                                                     aICP->GetTranslation()
                                                                                   );

    MITK_INFO << "TRE: Expected: " << expTRE << ", computed: " << tre;
    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test TRE",
                           mitk::Equal(tre,expTRE,0.0001));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkAnisotropicIterativeClosestPointRegistration)
