/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include "mitkImageMappingHelper.h"

#include <mitkGeometry3D.h>
#include <mitkVector.h>

class mitkImageMappingHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageMappingHelperTestSuite);
  MITK_TEST(GenerateSuperSampledGeometry_NoScaling_KeepsGeometry);
  MITK_TEST(GenerateSuperSampledGeometry_Scaled_CoversSameSpace);
  MITK_TEST(GenerateSuperSampledGeometry_Rotated_CoversSameSpace);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Geometry3D::Pointer m_Geometry;

  /** Outer corner of the corner voxel. Super sampling changes the spacing and
      therefore the origin, but this corner has to stay where it is. */
  static mitk::Point3D GetMatrixCorner(const mitk::BaseGeometry* geometry)
  {
    mitk::Point3D cornerIndex;
    cornerIndex.Fill(-0.5);

    mitk::Point3D corner;
    geometry->IndexToWorld(cornerIndex, corner);

    return corner;
  }

  static mitk::Geometry3D::Pointer CreateGeometry(const mitk::AffineTransform3D::MatrixType& rotation)
  {
    auto geometry = mitk::Geometry3D::New();

    mitk::BaseGeometry::BoundsArrayType bounds;
    bounds[0] = 0.0; bounds[1] = 10.0;
    bounds[2] = 0.0; bounds[3] = 20.0;
    bounds[4] = 0.0; bounds[5] = 30.0;
    geometry->SetBounds(bounds);

    auto transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(rotation);
    geometry->SetIndexToWorldTransform(transform);

    // SetSpacing scales the columns of the matrix, so it must follow the rotation.
    mitk::Vector3D spacing;
    spacing[0] = 1.0; spacing[1] = 2.0; spacing[2] = 4.0;
    geometry->SetSpacing(spacing);

    mitk::Point3D origin;
    origin[0] = 7.0; origin[1] = -3.0; origin[2] = 11.0;
    geometry->SetOrigin(origin);

    return geometry;
  }

public:
  void setUp() override
  {
    mitk::AffineTransform3D::MatrixType identity;
    identity.SetIdentity();

    m_Geometry = CreateGeometry(identity);
  }

  void tearDown() override
  {
    m_Geometry = nullptr;
  }

  void GenerateSuperSampledGeometry_NoScaling_KeepsGeometry()
  {
    auto result = mitk::ImageMappingHelper::GenerateSuperSampledGeometry(m_Geometry, 1.0, 1.0, 1.0);

    CPPUNIT_ASSERT_MESSAGE("Spacing must be unchanged.",
      mitk::Equal(result->GetSpacing(), m_Geometry->GetSpacing(), mitk::eps));
    CPPUNIT_ASSERT_MESSAGE("Origin must be unchanged.",
      mitk::Equal(result->GetOrigin(), m_Geometry->GetOrigin(), mitk::eps));
  }

  void GenerateSuperSampledGeometry_Scaled_CoversSameSpace()
  {
    auto result = mitk::ImageMappingHelper::GenerateSuperSampledGeometry(m_Geometry, 2.0, 4.0, 1.0);

    mitk::Vector3D expectedSpacing;
    expectedSpacing[0] = 0.5; expectedSpacing[1] = 0.5; expectedSpacing[2] = 4.0;
    CPPUNIT_ASSERT_MESSAGE("Spacing must be divided by the scaling.",
      mitk::Equal(result->GetSpacing(), expectedSpacing, mitk::eps));

    CPPUNIT_ASSERT_MESSAGE("Voxel count must be multiplied by the scaling.",
      mitk::Equal(result->GetBounds()[1], 20.0, mitk::eps) &&
      mitk::Equal(result->GetBounds()[3], 80.0, mitk::eps) &&
      mitk::Equal(result->GetBounds()[5], 30.0, mitk::eps));

    CPPUNIT_ASSERT_MESSAGE("The corner of the voxel matrix must not move.",
      mitk::Equal(GetMatrixCorner(result), GetMatrixCorner(m_Geometry), mitk::eps));
  }

  void GenerateSuperSampledGeometry_Rotated_CoversSameSpace()
  {
    // 90 degrees around z, so the image x axis points along world y. Correcting
    // the origin along the world axes instead of the image axes would put the
    // voxel matrix somewhere else.
    mitk::AffineTransform3D::MatrixType rotation;
    rotation.Fill(0.0);
    rotation[1][0] = 1.0;
    rotation[0][1] = -1.0;
    rotation[2][2] = 1.0;

    auto rotated = CreateGeometry(rotation);
    auto result = mitk::ImageMappingHelper::GenerateSuperSampledGeometry(rotated, 2.0, 2.0, 2.0);

    CPPUNIT_ASSERT_MESSAGE("The corner of the voxel matrix must not move for a rotated geometry either.",
      mitk::Equal(GetMatrixCorner(result), GetMatrixCorner(rotated), mitk::eps));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageMappingHelper)
