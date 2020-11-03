/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCovarianceMatrixCalculator.h>
#include <mitkIOUtil.h>
#include <mitkSurface.h>
#include <mitkTestingMacros.h>
#include <mitkVector.h>
#include <vtkCleanPolyData.h>

#include <mitkTestFixture.h>

/** Test class to test the computation of covariance matrices
  * for the A-ICP algorithm. The test runs the CM_PCA method.
  */
class mitkCovarianceMatrixCalculatorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkCovarianceMatrixCalculatorTestSuite);
  MITK_TEST(testCovarianceMatrixCalculation_CM_PCA);
  CPPUNIT_TEST_SUITE_END();

private:
  typedef itk::Matrix<double, 3, 3> CovarianceMatrix;
  typedef std::vector<CovarianceMatrix> CovarianceMatrixList;

  mitk::Surface::Pointer m_Surface;
  CovarianceMatrixList m_Reference;

public:
  void setUp() override
  {
    m_Surface = mitk::IOUtil::Load<mitk::Surface>(GetTestDataFilePath("RenderingTestData/Stanford_bunny.stl"));

    // saved results
    CovarianceMatrix m1, m2, m3;

    // set 3 reference results into the vector. The first in the array, one from
    // the middle and the last one
    m1[0][0] = 366.169;
    m1[1][0] = 39.5242;
    m1[2][0] = 102.368;
    m1[0][1] = 39.5242;
    m1[1][1] = 6.97979;
    m1[2][1] = 6.91726;
    m1[0][2] = 102.368;
    m1[1][2] = 6.91726;
    m1[2][2] = 389.481;
    m_Reference.push_back(m1);

    m2[0][0] = 107.999;
    m2[1][0] = 71.6708;
    m2[2][0] = -0.908269;
    m2[0][1] = 71.6708;
    m2[1][1] = 133.407;
    m2[2][1] = 40.8706;
    m2[0][2] = -0.908269;
    m2[1][2] = 40.8706;
    m2[2][2] = 25.1825;
    m_Reference.push_back(m2);

    m3[0][0] = 177.916;
    m3[1][0] = 4.92498;
    m3[2][0] = 5.86319;
    m3[0][1] = 4.92498;
    m3[1][1] = 0.214147;
    m3[2][1] = -1.98345;
    m3[0][2] = 5.86319;
    m3[1][2] = -1.98345;
    m3[2][2] = 232.092;
    m_Reference.push_back(m3);
  }

  void tearDown() override
  {
    m_Surface = nullptr;
    m_Reference.clear();
  }

  void testCovarianceMatrixCalculation_CM_PCA()
  {
    mitk::CovarianceMatrixCalculator::Pointer matrixCalculator = mitk::CovarianceMatrixCalculator::New();
    matrixCalculator->SetInputSurface(m_Surface);
    matrixCalculator->ComputeCovarianceMatrices();
    CovarianceMatrixList result = matrixCalculator->GetCovarianceMatrices();

    CPPUNIT_ASSERT_MESSAGE("mitkCovarianceMatrixCalculatorTestSuite test first matrix",
                           mitk::MatrixEqualElementWise(result.at(0), m_Reference.at(0), 0.001));

    CPPUNIT_ASSERT_MESSAGE("mitkCovarianceMatrixCalculatorTestSuite test middle matrix",
                           mitk::MatrixEqualElementWise(result.at(result.size() / 2), m_Reference.at(1), 0.001));

    CPPUNIT_ASSERT_MESSAGE("mitkCovarianceMatrixCalculatorTestSuite test last matrix",
                           mitk::MatrixEqualElementWise(result.at((result.size() - 1)), m_Reference.at(2), 0.001));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCovarianceMatrixCalculator)
