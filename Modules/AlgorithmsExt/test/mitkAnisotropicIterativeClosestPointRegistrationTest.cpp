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

#include <mitkIOUtil.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <vtkCleanPolyData.h>

#include "mitkAnisotropicIterativeClosestPointRegistration.h"
#include "mitkAnisotropicRegistrationCommon.h"
#include "mitkCovarianceMatrixCalculator.h"

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
  typedef itk::Matrix<double, 3, 3> Matrix3x3;
  typedef itk::Vector<double, 3> Vector3;
  typedef std::vector<Matrix3x3> CovarianceMatrixList;

  mitk::Surface::Pointer m_MovingSurface;
  mitk::Surface::Pointer m_FixedSurface;

  mitk::PointSet::Pointer m_TargetsMovingSurface;
  mitk::PointSet::Pointer m_TargetsFixedSurface;

  CovarianceMatrixList m_SigmasMovingSurface;
  CovarianceMatrixList m_SigmasFixedSurface;

  double m_FRENormalizationFactor;

public:
  /**
   * @brief Setup Always call this method before each Test-case to ensure
   * correct and new intialization of the used members for a new test case.
   * (If the members are not used in a test, the method does not need to be called).
   */
  void setUp() override
  {
    mitk::CovarianceMatrixCalculator::Pointer matrixCalculator = mitk::CovarianceMatrixCalculator::New();

    m_MovingSurface = dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("AICPRegistration/head_green.stl"))[0].GetPointer());
    m_FixedSurface = dynamic_cast<mitk::Surface*>(mitk::IOUtil::Load(GetTestDataFilePath("AICPRegistration/head_red.stl"))[0].GetPointer());

    m_TargetsMovingSurface = dynamic_cast<mitk::PointSet*>(mitk::IOUtil::Load(GetTestDataFilePath("AICPRegistration/targets_head_green.mps"))[0].GetPointer());
    m_TargetsFixedSurface = dynamic_cast<mitk::PointSet*>(mitk::IOUtil::Load(GetTestDataFilePath("AICPRegistration/targets_head_red.mps"))[0].GetPointer());

    // compute covariance matrices
    matrixCalculator->SetInputSurface(m_MovingSurface);
    matrixCalculator->ComputeCovarianceMatrices();
    m_SigmasMovingSurface = matrixCalculator->GetCovarianceMatrices();
    const double meanVarX = matrixCalculator->GetMeanVariance();

    matrixCalculator->SetInputSurface(m_FixedSurface);
    matrixCalculator->ComputeCovarianceMatrices();
    m_SigmasFixedSurface = matrixCalculator->GetCovarianceMatrices();
    const double meanVarY = matrixCalculator->GetMeanVariance();

    m_FRENormalizationFactor = sqrt(meanVarX + meanVarY);
  }

  void tearDown() override
  {
    m_MovingSurface = nullptr;
    m_FixedSurface = nullptr;

    m_TargetsMovingSurface = nullptr;
    m_TargetsFixedSurface = nullptr;

    m_SigmasMovingSurface.clear();
    m_SigmasFixedSurface.clear();
  }

  void testAicpRegistration()
  {
    const double expFRE = 27.5799;
    const double expTRE = 1.68835;
    mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
      mitk::AnisotropicIterativeClosestPointRegistration::New();

    // set up parameters
    aICP->SetMovingSurface(m_MovingSurface);
    aICP->SetFixedSurface(m_FixedSurface);
    aICP->SetCovarianceMatricesMovingSurface(m_SigmasMovingSurface);
    aICP->SetCovarianceMatricesFixedSurface(m_SigmasFixedSurface);
    aICP->SetFRENormalizationFactor(m_FRENormalizationFactor);
    aICP->SetThreshold(0.000001);

    // run the algorithm
    aICP->Update();

    MITK_INFO << "FRE: Expected: " << expFRE << ", computed: " << aICP->GetFRE();
    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test FRE",
                           mitk::Equal(aICP->GetFRE(), expFRE, 0.0001));

    // compute the target registration Error
    const double tre =
      mitk::AnisotropicRegistrationCommon::ComputeTargetRegistrationError(m_TargetsMovingSurface.GetPointer(),
                                                                          m_TargetsFixedSurface.GetPointer(),
                                                                          aICP->GetRotation(),
                                                                          aICP->GetTranslation());

    // MITK_INFO << "R:\n" << aICP->GetRotation() << "T: "<< aICP->GetTranslation();

    MITK_INFO << "TRE: Expected: " << expTRE << ", computed: " << tre;
    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test TRE",
                           mitk::Equal(tre, expTRE, 0.00001));
  }

  void testTrimmedAicpregistration()
  {
    const double expFRE = 4.8912;
    const double expTRE = 0.0484215;

    mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
      mitk::AnisotropicIterativeClosestPointRegistration::New();

    // Swap X and Y for partial overlapping registration
    aICP->SetMovingSurface(m_MovingSurface);
    aICP->SetFixedSurface(m_FixedSurface);
    aICP->SetCovarianceMatricesMovingSurface(m_SigmasMovingSurface);
    aICP->SetCovarianceMatricesFixedSurface(m_SigmasFixedSurface);
    aICP->SetFRENormalizationFactor(m_FRENormalizationFactor);
    aICP->SetThreshold(0.000001);
    aICP->SetTrimmFactor(0.50);

    // run the algorithm
    aICP->Update();

    MITK_INFO << "FRE: Expected: " << expFRE << ", computed: " << aICP->GetFRE();

    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test FRE",
                           mitk::Equal(aICP->GetFRE(), expFRE, 0.01));

    // compute the target registration Error
    const double tre =
      mitk::AnisotropicRegistrationCommon::ComputeTargetRegistrationError(m_TargetsMovingSurface.GetPointer(),
                                                                          m_TargetsFixedSurface.GetPointer(),
                                                                          aICP->GetRotation(),
                                                                          aICP->GetTranslation());

    MITK_INFO << "TRE: Expected: " << expTRE << ", computed: " << tre;
    CPPUNIT_ASSERT_MESSAGE("mitkAnisotropicIterativeClosestPointRegistrationTest:AicpRegistration Test TRE",
                           mitk::Equal(tre, expTRE, 0.01));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkAnisotropicIterativeClosestPointRegistration)
