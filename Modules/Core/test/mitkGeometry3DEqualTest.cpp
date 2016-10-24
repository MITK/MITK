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

#include "mitkGeometry3D.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

class mitkGeometry3DEqualTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkGeometry3DEqualTestSuite);
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_DifferentOrigin_ReturnsFalse);
  MITK_TEST(Equal_DifferentIndexToWorldTransform_ReturnsFalse);
  MITK_TEST(Equal_DifferentSpacing_ReturnsFalse);
  MITK_TEST(Equal_DifferentImageGeometry_ReturnsFalse);
  MITK_TEST(Equal_DifferentBoundingBox_ReturnsFalse);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different test methods. All members are initialized via setUp().*/
  mitk::Geometry3D::Pointer m_Geometry3D;
  mitk::Geometry3D::Pointer m_AnotherGeometry3D;

public:
  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members
* for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp() override
  {
    m_Geometry3D = mitk::Geometry3D::New();
    m_Geometry3D->Initialize();
    m_AnotherGeometry3D = m_Geometry3D->Clone();
  }

  void tearDown() override
  {
    m_Geometry3D = nullptr;
    m_AnotherGeometry3D = nullptr;
  }

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    MITK_ASSERT_EQUAL(m_Geometry3D, m_AnotherGeometry3D, "A clone should be equal to its original.");
  }

  void Equal_DifferentOrigin_ReturnsFalse()
  {
    mitk::Point3D origin;
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 1.0 + 2 * mitk::eps;
    m_AnotherGeometry3D->SetOrigin(origin);

    MITK_ASSERT_NOT_EQUAL(m_Geometry3D, m_AnotherGeometry3D, "Origin was modified. Result should be false.");
  }

  void Equal_DifferentIndexToWorldTransform_ReturnsFalse()
  {
    // Create another index to world transform and make it different somehow
    mitk::AffineTransform3D::Pointer differentIndexToWorldTransform = mitk::AffineTransform3D::New();

    mitk::AffineTransform3D::MatrixType differentMatrix;
    differentMatrix.SetIdentity();
    differentMatrix(1, 1) = 2;

    differentIndexToWorldTransform->SetMatrix(differentMatrix);
    m_AnotherGeometry3D->SetIndexToWorldTransform(differentIndexToWorldTransform);

    MITK_ASSERT_NOT_EQUAL(
      m_Geometry3D, m_AnotherGeometry3D, "IndexToWorldTransform was modified. Result should be false.");
  }

  void Equal_DifferentSpacing_ReturnsFalse()
  {
    mitk::Vector3D differentSpacing;
    differentSpacing[0] = 1.0;
    differentSpacing[1] = 1.0 + 2 * mitk::eps;
    differentSpacing[2] = 1.0;

    m_AnotherGeometry3D->SetSpacing(differentSpacing);

    MITK_ASSERT_NOT_EQUAL(m_Geometry3D, m_AnotherGeometry3D, "Spacing was modified. Result should be false.");
  }

  void Equal_DifferentImageGeometry_ReturnsFalse()
  {
    m_AnotherGeometry3D->SetImageGeometry(true);

    MITK_ASSERT_NOT_EQUAL(
      m_Geometry3D, m_AnotherGeometry3D, "One Geometry is image, the other is not. Result should be false.");
  }

  void Equal_DifferentBoundingBox_ReturnsFalse()
  {
    // create different bounds to make the comparison false
    mitk::ScalarType bounds[] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    m_AnotherGeometry3D->SetBounds(bounds);

    MITK_ASSERT_NOT_EQUAL(m_Geometry3D, m_AnotherGeometry3D, "Bounds are different. Result should be false.");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkGeometry3DEqual)
