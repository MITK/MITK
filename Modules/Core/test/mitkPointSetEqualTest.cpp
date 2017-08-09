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

#include "mitkPointSet.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

/**
 * @brief mitkPointSetEqualTestSuite A test class for Equal methods in mitk::PointSet.
 */

class mitkPointSetEqualTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPointSetEqualTestSuite);
  MITK_TEST(Equal_CloneAndOriginal_ReturnsTrue);
  MITK_TEST(Equal_DifferentGeometries_ReturnsFalse);
  MITK_TEST(Equal_DifferentNumberOfPoints_ReturnsFalse);
  MITK_TEST(Equal_DifferentPoints_ReturnsFalse);
  CPPUNIT_TEST_SUITE_END();

private:
  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::PointSet::Pointer m_PointSet;
  mitk::PointSet::Pointer m_AnotherPointSet;

public:
  /**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members
* for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
  void setUp() override
  {
    m_PointSet = mitk::PointSet::New();
    m_AnotherPointSet = m_PointSet->Clone();
  }

  void tearDown() override
  {
    m_PointSet = nullptr;
    m_AnotherPointSet = nullptr;
  }

  void Equal_CloneAndOriginal_ReturnsTrue()
  {
    mitk::PointSet::Pointer newPointSet = mitk::PointSet::New();
    MITK_ASSERT_EQUAL(newPointSet, newPointSet->Clone(), "A clone should be equal to its original.");
  }

  void Equal_DifferentGeometries_ReturnsFalse()
  {
    mitk::Point3D origin;
    origin[0] = 0.0;
    origin[1] = 0.0;
    origin[2] = 1.0 + 2 * mitk::eps;
    m_AnotherPointSet->GetGeometry()->SetOrigin(origin);

    MITK_ASSERT_NOT_EQUAL(m_PointSet, m_AnotherPointSet, "Origin was modified. Result should be false.");
  }

  void Equal_DifferentNumberOfPoints_ReturnsFalse()
  {
    mitk::Point3D tmpPoint;
    tmpPoint[0] = 1.0;
    tmpPoint[1] = 1.0;
    tmpPoint[2] = 1.0;

    m_PointSet->InsertPoint(1, tmpPoint);
    m_PointSet->InsertPoint(2, tmpPoint);

    m_AnotherPointSet->InsertPoint(1, tmpPoint);

    MITK_ASSERT_NOT_EQUAL(
      m_PointSet, m_AnotherPointSet, "One pointset has two points the other has one. Result should be false.");
  }

  void Equal_DifferentPoints_ReturnsFalse()
  {
    mitk::Point3D tmpPoint;
    tmpPoint[0] = 1.0;
    tmpPoint[1] = 1.0;
    tmpPoint[2] = 1.0;

    m_PointSet->InsertPoint(1, tmpPoint);

    tmpPoint[0] = 1.0 + 2 * mitk::eps;
    m_AnotherPointSet->InsertPoint(1, tmpPoint);

    MITK_ASSERT_NOT_EQUAL(
      m_PointSet, m_AnotherPointSet, "Two pointsets with different points. Result should be false.");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetEqual)
