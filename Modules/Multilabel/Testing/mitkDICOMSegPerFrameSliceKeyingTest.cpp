/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkBaseGeometry.h>
#include <mitkImage.h>
#include <mitkSlicedGeometry3D.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkIndex.h>

#include <string>
#include <vector>

// Pins the geometry contract the DICOM SEG reader relies on when it keys
// per-frame source references by geometry slice index: the
// group geometry's WorldToIndex(IPP)[2] must return the slice a point lies on,
// for any query order and any sparse subset, and must round (not truncate) when
// a DS-parsed IPP comes back a hair off an integer index.
//
// This is a self-consistency check on the centre-based image-geometry
// convention: the test point for slice k is derived from the same geometry via
// IndexToWorld, so it CANNOT prove that the convention itself (centre vs corner)
// matches DICOM's ImagePositionPatient - a shared convention error would pass
// here. That claim is checked end-to-end against real dcmqi output by
// mitkDICOMSegmentationIONonZOrderedTest's IndexToWorld({0,0,0}) == min-Z IPP
// assertion. What this test does pin is the rounding-overload choice and the
// order/sparsity independence of the mapping.

class mitkDICOMSegPerFrameSliceKeyingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMSegPerFrameSliceKeyingTestSuite);
  MITK_TEST(ResolvesEachSliceIndex);
  MITK_TEST(ResolvesSlicesInShuffledAndGappedOrder);
  MITK_TEST(RoundsSubSpacingNoiseToNearestSlice);
  CPPUNIT_TEST_SUITE_END();

private:
  static constexpr unsigned int SLICE_COUNT = 6u;
  mitk::Image::Pointer m_Image;

  // Non-trivial origin / spacing (and the default axial direction) so the
  // assertions exercise real index<->world arithmetic, not an identity map.
  void BuildImage()
  {
    m_Image = mitk::Image::New();
    unsigned int dims[3] = {2u, 2u, SLICE_COUNT};
    m_Image->Initialize(mitk::MakeScalarPixelType<unsigned char>(), 3, dims);

    mitk::Vector3D spacing;
    spacing[0] = 0.8;
    spacing[1] = 0.8;
    spacing[2] = 2.5;
    m_Image->SetSpacing(spacing);

    mitk::Point3D origin;
    origin[0] = 10.0;
    origin[1] = 20.0;
    origin[2] = -30.0;
    m_Image->SetOrigin(origin);
  }

  // The point at continuous index (0, 0, k) is the centre of voxel (0,0,k);
  // mapping it back through the geometry must recover slice k.
  void AssertSliceResolves(const mitk::BaseGeometry *geometry, unsigned int k)
  {
    mitk::Point3D continuousIndex;
    continuousIndex[0] = 0.0;
    continuousIndex[1] = 0.0;
    continuousIndex[2] = static_cast<double>(k);

    mitk::Point3D world;
    geometry->IndexToWorld(continuousIndex, world);

    itk::Index<3> resolved;
    geometry->WorldToIndex(world, resolved);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("WorldToIndex resolves the point on slice " + std::to_string(k),
                                 static_cast<itk::IndexValueType>(k), resolved[2]);
  }

public:
  void setUp() override { this->BuildImage(); }
  void tearDown() override { m_Image = nullptr; }

  void ResolvesEachSliceIndex()
  {
    const auto *geometry = m_Image->GetSlicedGeometry();
    for (unsigned int k = 0; k < SLICE_COUNT; ++k)
      this->AssertSliceResolves(geometry, k);
  }

  void ResolvesSlicesInShuffledAndGappedOrder()
  {
    const auto *geometry = m_Image->GetSlicedGeometry();
    // Query order is independent of the result (each lookup is standalone), and
    // a sparse subset must resolve just as a contiguous run does - the two
    // properties a non-Z-ordered, gapped third-party SEG depends on.
    for (const unsigned int k : {4u, 0u, 5u, 2u})
      this->AssertSliceResolves(geometry, k);
  }

  void RoundsSubSpacingNoiseToNearestSlice()
  {
    const auto *geometry = m_Image->GetSlicedGeometry();

    // A DS-parsed IPP can land a hair off an integer index. The itk::Index
    // overload rounds (RoundHalfIntegerUp), so a continuous index of 2.9999997
    // must resolve to slice 3; a truncating cast would wrongly yield 2.
    mitk::Point3D continuousIndex;
    continuousIndex[0] = 0.0;
    continuousIndex[1] = 0.0;
    continuousIndex[2] = 2.9999997;

    mitk::Point3D world;
    geometry->IndexToWorld(continuousIndex, world);

    itk::Index<3> resolved;
    geometry->WorldToIndex(world, resolved);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Sub-spacing IPP noise rounds to the nearest slice",
                                 static_cast<itk::IndexValueType>(3), resolved[2]);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMSegPerFrameSliceKeying)
