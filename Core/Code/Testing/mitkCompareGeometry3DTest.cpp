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

mitk::Geometry3D::Pointer m_Geometry3D;
mitk::Geometry3D::Pointer m_AnotherGeometry3D;

/**
 * @brief Setup Always call this method before each Test-case to ensure correct and new intialization of the two used members for a new test case.
 */
void Setup()
{
  m_Geometry3D = mitk::Geometry3D::New();
  m_Geometry3D->Initialize();
  m_AnotherGeometry3D = m_Geometry3D->Clone();
}

void AreEqual_CloneAndOriginal_ReturnsTrue()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED( mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "A clone should be equal to its original.");
}

void AreEqual_DifferentOrigin_ReturnsFalse()
{
  Setup();

  mitk::Point3D origin;
  origin[0] = 0.0;
  origin[1] = 0.0;
  origin[2] = mitk::eps * 1.01;
  m_AnotherGeometry3D->SetOrigin(origin);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "Origin was modified. Result should be false.");
}

void AreEqual_DifferentIndexToWorldTransform_ReturnsFalse()
{
  Setup();

  //Create another index to world transform and make it different somehow
  mitk::AffineTransform3D::Pointer differentIndexToWorldTransform = mitk::AffineTransform3D::New();

  mitk::AffineTransform3D::MatrixType differentMatrix;
  differentMatrix.SetIdentity();
  differentMatrix(1,1) = 2;

  differentIndexToWorldTransform->SetMatrix( differentMatrix );
  m_AnotherGeometry3D->SetIndexToWorldTransform(differentIndexToWorldTransform);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "IndexToWorldTransform was modified. Result should be false.");
}

void AreEqual_DifferentSpacing_ReturnsFalse()
{
  Setup();

  mitk::Vector3D differentSpacing;
  differentSpacing[0] = 1.0;
  differentSpacing[1] = 1.0*mitk::eps;
  differentSpacing[2] = 1.0;

  m_AnotherGeometry3D->SetSpacing(differentSpacing);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "Spacing was modified. Result should be false.");
}

void AreEqual_InputIsNull_ReturnsFalse()
{
  Setup();

  m_Geometry3D = NULL;
  m_AnotherGeometry3D = NULL;

  MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "Input is NULL. Result should be false.");
}

void AreEqual_DifferentImageGeometry_ReturnsFalse()
{
  Setup();

  m_AnotherGeometry3D->SetImageGeometry(true);

  MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "One Geometry is image, the other is not. Result should be false.");
}

void AreEqual_DifferentBoundingBox_ReturnsFalse()
{
    Setup();

    //create different bounds to make the comparison false
    float bounds[ ] = {0.0, 0.0, 0.0*mitk::eps, 0.0, 0.0, 0.0};
    m_AnotherGeometry3D->SetBounds(bounds);

    MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreEqual( m_Geometry3D, m_AnotherGeometry3D), "Bounds are different. Result should be false.");
    //The second test-case invokes explicitly the concrete comparison method,
    //because this is currently skipped in mitk::Geometry3D::AreEqual as the
    //comparison of extent and axis fails if the bounds are changed.
    MITK_TEST_CONDITION_REQUIRED( !mitk::Geometry3D::AreBoundingBoxesEqual( m_Geometry3D, m_AnotherGeometry3D), "Bounds are different. Result should be false.");
}

/**
 * @brief mitkCompareGeometry3DTest A test class for AreEqual methods in mitk::Geometry3D.
 */
int mitkCompareGeometry3DTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN(mitkCompareGeometry3DTest);

  AreEqual_CloneAndOriginal_ReturnsTrue();
  AreEqual_InputIsNull_ReturnsFalse();
  AreEqual_DifferentSpacing_ReturnsFalse();
  AreEqual_DifferentImageGeometry_ReturnsFalse();
  AreEqual_DifferentOrigin_ReturnsFalse();
  AreEqual_DifferentIndexToWorldTransform_ReturnsFalse();
  AreEqual_DifferentBoundingBox_ReturnsFalse();

  MITK_TEST_END();

  return EXIT_SUCCESS;
}
