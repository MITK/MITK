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

#include "mitkTestingMacros.h"
#include <mitkPointSet.h>

/** Members used inside the different (sub-)tests. All members are initialized via Setup().*/
mitk::PointSet::Pointer m_PointSet;
mitk::PointSet::Pointer m_AnotherPointSet;

/**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
static void Setup()
{
  m_PointSet = mitk::PointSet::New();
  m_AnotherPointSet = m_PointSet->Clone();
}

static void Equal_CloneAndOriginal_ReturnsTrue()
{
  Setup();
  MITK_TEST_EQUAL( m_PointSet, m_AnotherPointSet, "A clone should be equal to its original.");
}

static void Equal_DifferentGeometries_ReturnsFalse()
{
  Setup();

  mitk::Point3D origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 1.0 + 2*mitk::eps;
  m_AnotherPointSet->GetGeometry()->SetOrigin(origin);

  MITK_TEST_NOT_EQUAL( m_PointSet, m_AnotherPointSet, "Origin was modified. Result should be false.");
}

static void Equal_InputIsNull_ReturnsFalse()
{
  mitk::PointSet::Pointer pointSet = NULL;
  MITK_TEST_NOT_EQUAL( pointSet, pointSet, "Input is NULL. Result should be false.");
}

static void Equal_DifferentNumberOfPoints_ReturnsFalse()
{
  Setup();
  mitk::Point3D tmpPoint;
  tmpPoint[0] = 1.0;
  tmpPoint[1] = 1.0;
  tmpPoint[2] = 1.0;

  m_PointSet->InsertPoint( 1, tmpPoint );
  m_PointSet->InsertPoint( 2, tmpPoint );

  m_AnotherPointSet->InsertPoint( 1, tmpPoint );

  MITK_TEST_NOT_EQUAL( m_PointSet, m_AnotherPointSet, "One pointset has two points the other has one. Result should be false.");
}

static void Equal_DifferentPoints_ReturnsFalse()
{
  Setup();

  mitk::Point3D tmpPoint;
  tmpPoint[0] = 1.0;
  tmpPoint[1] = 1.0;
  tmpPoint[2] = 1.0;

  m_PointSet->InsertPoint( 1, tmpPoint );

  tmpPoint[0] = 1.0 + 2*mitk::eps;
  m_AnotherPointSet->InsertPoint( 1, tmpPoint );

  MITK_TEST_NOT_EQUAL( m_PointSet, m_AnotherPointSet, "Two pointsets with different points. Result should be false.");
}

/**
 * @brief mitkPointSetEqualTest A test class for Equal methods in mitk::PointSet.
 */
int mitkPointSetEqualTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkPointSetEqualTest);

  Equal_CloneAndOriginal_ReturnsTrue();
  Equal_InputIsNull_ReturnsFalse();
  Equal_DifferentGeometries_ReturnsFalse();
  Equal_DifferentNumberOfPoints_ReturnsFalse();
  Equal_DifferentPoints_ReturnsFalse();

  MITK_TEST_END();
}
