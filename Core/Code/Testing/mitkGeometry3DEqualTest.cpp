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
#include "mitkTestingMacros.h"

/** Members used inside the different (sub-)tests. All members are initialized via Setup().*/
mitk::Geometry3D::Pointer m_Geometry3D;
mitk::Geometry3D::Pointer m_AnotherGeometry3D;

/**
* @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the used members for a new test case. (If the members are not used in a test, the method does not need to be called).
*/
static void Setup()
{
  m_Geometry3D = mitk::Geometry3D::New();
  m_Geometry3D->Initialize();
  m_AnotherGeometry3D = m_Geometry3D->Clone();
}

static void Equal_CloneAndOriginal_ReturnsTrue()
{
  Setup();
  MITK_TEST_EQUAL( m_Geometry3D, m_AnotherGeometry3D, "A clone should be equal to its original.");
}

static void Equal_DifferentOrigin_ReturnsFalse()
{
  Setup();

  mitk::Point3D origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = 1.0 + 2*mitk::eps;
  m_AnotherGeometry3D->SetOrigin(origin);

  MITK_TEST_NOT_EQUAL( m_Geometry3D, m_AnotherGeometry3D, "Origin was modified. Result should be false.");
}

static void Equal_DifferentIndexToWorldTransform_ReturnsFalse()
{
  Setup();

  //Create another index to world transform and make it different somehow
  mitk::AffineTransform3D::Pointer differentIndexToWorldTransform = mitk::AffineTransform3D::New();

  mitk::AffineTransform3D::MatrixType differentMatrix;
  differentMatrix.SetIdentity();
  differentMatrix(1,1) = 2;

  differentIndexToWorldTransform->SetMatrix( differentMatrix );
  m_AnotherGeometry3D->SetIndexToWorldTransform(differentIndexToWorldTransform);

  MITK_TEST_NOT_EQUAL( m_Geometry3D, m_AnotherGeometry3D, "IndexToWorldTransform was modified. Result should be false.");
}

static void Equal_DifferentSpacing_ReturnsFalse()
{
  Setup();

  mitk::Vector3D differentSpacing;
  differentSpacing[0] = 1.0;
  differentSpacing[1] = 1.0 + 2*mitk::eps;
  differentSpacing[2] = 1.0;

  m_AnotherGeometry3D->SetSpacing(differentSpacing);

  MITK_TEST_NOT_EQUAL( m_Geometry3D, m_AnotherGeometry3D, "Spacing was modified. Result should be false.");
}

static void Equal_InputIsNull_ReturnsFalse()
{
  mitk::Geometry3D::Pointer geometry3D = NULL;
  MITK_TEST_NOT_EQUAL( geometry3D, geometry3D, "Input is NULL. Result should be false.");
}

static void Equal_DifferentImageGeometry_ReturnsFalse()
{
  Setup();

  m_AnotherGeometry3D->SetImageGeometry(true);

  MITK_TEST_NOT_EQUAL( m_Geometry3D, m_AnotherGeometry3D, "One Geometry is image, the other is not. Result should be false.");
}

static void Equal_DifferentBoundingBox_ReturnsFalse()
{
    Setup();

    //create different bounds to make the comparison false
    float bounds[ ] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    m_AnotherGeometry3D->SetBounds(bounds);

    MITK_TEST_NOT_EQUAL( m_Geometry3D, m_AnotherGeometry3D, "Bounds are different. Result should be false.");
}

/**
 * @brief mitkGeometry3DEqualTest A test class for Equal methods in mitk::Geometry3D.
 */
int mitkGeometry3DEqualTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkGeometry3DEqualTest);

  Equal_CloneAndOriginal_ReturnsTrue();
  Equal_InputIsNull_ReturnsFalse();
  Equal_DifferentSpacing_ReturnsFalse();
  Equal_DifferentImageGeometry_ReturnsFalse();
  Equal_DifferentOrigin_ReturnsFalse();
  Equal_DifferentIndexToWorldTransform_ReturnsFalse();
  Equal_DifferentBoundingBox_ReturnsFalse();

  MITK_TEST_END();
}
