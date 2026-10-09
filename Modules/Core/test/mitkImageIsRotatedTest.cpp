/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <mitkImage.h>

#include <array>
#include <cmath>

class mitkImageIsRotatedTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkImageIsRotatedTestSuite);
  MITK_TEST(Identity_IsNotRotated);
  MITK_TEST(AnisotropicSpacing_IsNotRotated);
  MITK_TEST(AxisFlip_IsNotRotated);
  MITK_TEST(AxisPermutation_IsNotRotated);
  MITK_TEST(FlippedAxisPermutation_IsNotRotated);
  MITK_TEST(RoundingNoise_IsNotRotated);
  MITK_TEST(SmallRotation_IsRotated);
  MITK_TEST(SmallRotationWithAnisotropicSpacing_IsRotated);
  MITK_TEST(TiltBelowTolerance_IsNotRotated);
  MITK_TEST(TiltAboveTolerance_IsRotated);
  CPPUNIT_TEST_SUITE_END();

private:
  static bool IsRotated(const mitk::AffineTransform3D::MatrixType::InternalMatrixType& matrix)
  {
    auto image = mitk::Image::New();
    const unsigned int dimensions[] = { 2, 2, 2 };
    image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dimensions);

    auto transform = mitk::AffineTransform3D::New();
    transform->SetMatrix(mitk::AffineTransform3D::MatrixType(matrix));
    image->GetGeometry()->SetIndexToWorldTransform(transform);

    return image->IsRotated();
  }

  static mitk::AffineTransform3D::MatrixType::InternalMatrixType Matrix(const std::array<double, 9>& values)
  {
    return mitk::AffineTransform3D::MatrixType::InternalMatrixType(values.data());
  }

  static mitk::AffineTransform3D::MatrixType::InternalMatrixType RotationAboutZ(double degrees, double xSpacing, double ySpacing)
  {
    const auto radians = degrees * std::acos(-1.0) / 180.0;
    const auto c = std::cos(radians);
    const auto s = std::sin(radians);

    return Matrix({ c * xSpacing, -s * ySpacing, 0.0,
                    s * xSpacing,  c * ySpacing, 0.0,
                    0.0,           0.0,          1.0 });
  }

public:
  void Identity_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(Matrix({ 1, 0, 0, 0, 1, 0, 0, 0, 1 })));
  }

  void AnisotropicSpacing_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(Matrix({ 0.5, 0, 0, 0, 0.5, 0, 0, 0, 5 })));
  }

  void AxisFlip_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(Matrix({ -1, 0, 0, 0, -1, 0, 0, 0, 1 })));
  }

  void AxisPermutation_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(Matrix({ 0, 0, 1, 1, 0, 0, 0, 1, 0 })));
  }

  void FlippedAxisPermutation_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(Matrix({ 0, 0, -3, 0.7, 0, 0, 0, -0.7, 0 })));
  }

  void RoundingNoise_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(Matrix({ 1, 1e-6, 0, -1e-6, 1, 0, 0, 0, 1 })));
  }

  void SmallRotation_IsRotated()
  {
    CPPUNIT_ASSERT(IsRotated(RotationAboutZ(1.0, 1.0, 1.0)));
  }

  void SmallRotationWithAnisotropicSpacing_IsRotated()
  {
    CPPUNIT_ASSERT(IsRotated(RotationAboutZ(1.0, 0.1, 5.0)));
  }

  void TiltBelowTolerance_IsNotRotated()
  {
    CPPUNIT_ASSERT(!IsRotated(RotationAboutZ(0.1, 1.0, 1.0)));
    CPPUNIT_ASSERT(!IsRotated(RotationAboutZ(0.1, 0.5, 5.0)));
  }

  void TiltAboveTolerance_IsRotated()
  {
    CPPUNIT_ASSERT(IsRotated(RotationAboutZ(0.3, 1.0, 1.0)));
    CPPUNIT_ASSERT(IsRotated(RotationAboutZ(0.3, 0.5, 5.0)));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkImageIsRotated)
