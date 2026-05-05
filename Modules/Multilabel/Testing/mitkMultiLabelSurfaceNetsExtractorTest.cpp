/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkGeometry3D.h>
#include <mitkMultiLabelSurfaceNetsExtractor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkImageData.h>
#include <vtkMatrix4x4.h>
#include <vtkNew.h>
#include <vtkPolyData.h>

#include <cmath>

class mitkMultiLabelSurfaceNetsExtractorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkMultiLabelSurfaceNetsExtractorTestSuite);
  MITK_TEST(Extract_NullInput_ReturnsEmpty);
  MITK_TEST(Extract_EmptyLabels_ReturnsEmpty);
  MITK_TEST(Extract_SingleLabel_ProducesBoundaryCells);
  MITK_TEST(Extract_TwoLabels_BoundaryLabelsCarryForegroundValue);
  MITK_TEST(ExtractPerLabel_ReturnsOnePolyDataPerPresentLabel);
  MITK_TEST(ExtractPerLabel_OmitsLabelsWithNoBoundary);
  MITK_TEST(SmoothingToggle_ChangesPointPositions);
  MITK_TEST(GetImageToWorldMatrix_NullGeometry_ReturnsIdentity);
  MITK_TEST(GetImageToWorldMatrix_StripsSpacingAndKeepsOrigin);
  MITK_TEST(GetImageToWorldMatrix_PreservesNonIdentityDirection);
  CPPUNIT_TEST_SUITE_END();

private:
  using LabelValueType = mitk::MultiLabelSegmentation::LabelValueType;

  /** Build a 16x16x16 uint16 image with one or two cube-shaped labels. */
  static vtkSmartPointer<vtkImageData> MakeLabeledImage(bool addSecondLabel)
  {
    auto image = vtkSmartPointer<vtkImageData>::New();
    image->SetDimensions(16, 16, 16);
    image->SetSpacing(1.0, 1.0, 1.0);
    image->SetOrigin(0.0, 0.0, 0.0);
    image->AllocateScalars(VTK_UNSIGNED_SHORT, 1);

    auto* scalars = static_cast<unsigned short*>(image->GetScalarPointer());
    const vtkIdType totalVoxels = 16 * 16 * 16;
    std::fill(scalars, scalars + totalVoxels, static_cast<unsigned short>(0));

    auto setVoxel = [&](int x, int y, int z, unsigned short v)
    {
      scalars[z * 16 * 16 + y * 16 + x] = v;
    };

    // Label 1: 4x4x4 cube at [4..7].
    for (int z = 4; z < 8; ++z)
      for (int y = 4; y < 8; ++y)
        for (int x = 4; x < 8; ++x)
          setVoxel(x, y, z, 1);

    if (addSecondLabel)
    {
      // Label 2: 2x2x2 cube at [10..11], well separated from label 1.
      for (int z = 10; z < 12; ++z)
        for (int y = 10; y < 12; ++y)
          for (int x = 10; x < 12; ++x)
            setVoxel(x, y, z, 2);
    }

    return image;
  }

public:
  void Extract_NullInput_ReturnsEmpty()
  {
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(nullptr, {1});
    CPPUNIT_ASSERT(result != nullptr);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), result->GetNumberOfCells());
  }

  void Extract_EmptyLabels_ReturnsEmpty()
  {
    auto image = MakeLabeledImage(false);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(image, {});
    CPPUNIT_ASSERT(result != nullptr);
    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), result->GetNumberOfCells());
  }

  void Extract_SingleLabel_ProducesBoundaryCells()
  {
    auto image = MakeLabeledImage(false);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(image, {1});

    CPPUNIT_ASSERT(result != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Single-label cube should yield boundary cells", result->GetNumberOfCells() > 0);

    auto* boundaryLabels = result->GetCellData()->GetArray("BoundaryLabels");
    CPPUNIT_ASSERT_MESSAGE("Output must carry the BoundaryLabels cell array", boundaryLabels != nullptr);
    CPPUNIT_ASSERT_EQUAL(2, boundaryLabels->GetNumberOfComponents());
  }

  void Extract_TwoLabels_BoundaryLabelsCarryForegroundValue()
  {
    auto image = MakeLabeledImage(true);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto result = extractor.Extract(image, {1, 2});

    auto* boundaryLabels = result->GetCellData()->GetArray("BoundaryLabels");
    CPPUNIT_ASSERT(boundaryLabels != nullptr);

    bool sawLabel1 = false;
    bool sawLabel2 = false;
    for (vtkIdType i = 0; i < boundaryLabels->GetNumberOfTuples(); ++i)
    {
      const auto fg = boundaryLabels->GetComponent(i, 0);
      if (fg == 1.0) sawLabel1 = true;
      else if (fg == 2.0) sawLabel2 = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Component 0 should hold each label's foreground value", sawLabel1 && sawLabel2);
  }

  void ExtractPerLabel_ReturnsOnePolyDataPerPresentLabel()
  {
    auto image = MakeLabeledImage(true);
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto results = extractor.ExtractPerLabel(image, {1, 2});

    CPPUNIT_ASSERT_EQUAL(static_cast<size_t>(2), results.size());
    CPPUNIT_ASSERT(results.find(1) != results.end());
    CPPUNIT_ASSERT(results.find(2) != results.end());
    CPPUNIT_ASSERT(results[1]->GetNumberOfCells() > 0);
    CPPUNIT_ASSERT(results[2]->GetNumberOfCells() > 0);
  }

  void ExtractPerLabel_OmitsLabelsWithNoBoundary()
  {
    auto image = MakeLabeledImage(false); // only label 1
    mitk::MultiLabelSurfaceNetsExtractor extractor;
    auto results = extractor.ExtractPerLabel(image, {1, 99});

    CPPUNIT_ASSERT(results.find(1) != results.end());
    CPPUNIT_ASSERT_MESSAGE("Labels with no boundary must be absent from the result map",
                           results.find(99) == results.end());
  }

  void SmoothingToggle_ChangesPointPositions()
  {
    auto image = MakeLabeledImage(false);

    mitk::MultiLabelSurfaceNetsExtractor smoothExtractor;
    smoothExtractor.SetSmoothing(true);
    auto smoothPoly = smoothExtractor.Extract(image, {1});

    mitk::MultiLabelSurfaceNetsExtractor exactExtractor;
    exactExtractor.SetSmoothing(false);
    auto exactPoly = exactExtractor.Extract(image, {1});

    CPPUNIT_ASSERT(exactPoly->GetNumberOfPoints() > 0);
    CPPUNIT_ASSERT(smoothPoly->GetNumberOfPoints() > 0);

    // vtkSurfaceNets3D places exact-mode vertices on the dual grid (between voxel
    // centers), so for unit-spacing input every coordinate is a multiple of 0.5.
    // Smoothing relaxes them off that grid. Checking grid alignment is order- and
    // count-independent; comparing point-by-point would be unreliable because
    // vtkSurfaceNets3D + vtkPolyDataNormals can change point order and count
    // between independent extractions, so a per-index check could pass under
    // arbitrary reorderings even if smoothing were a no-op.
    auto onHalfGrid = [](double v)
    {
      const double doubled = 2.0 * v;
      return std::abs(doubled - std::round(doubled)) < 1e-4;
    };
    auto allOnHalfGrid = [&](const double p[3])
    {
      return onHalfGrid(p[0]) && onHalfGrid(p[1]) && onHalfGrid(p[2]);
    };

    for (vtkIdType i = 0; i < exactPoly->GetNumberOfPoints(); ++i)
    {
      double p[3];
      exactPoly->GetPoint(i, p);
      CPPUNIT_ASSERT_MESSAGE("Exact-mode points must lie on the voxel half-grid",
                             allOnHalfGrid(p));
    }

    bool foundOffGrid = false;
    for (vtkIdType i = 0; i < smoothPoly->GetNumberOfPoints() && !foundOffGrid; ++i)
    {
      double p[3];
      smoothPoly->GetPoint(i, p);
      if (!allOnHalfGrid(p))
        foundOffGrid = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Smoothed output must contain at least one point off the voxel half-grid",
                           foundOffGrid);
  }

  void GetImageToWorldMatrix_NullGeometry_ReturnsIdentity()
  {
    auto matrix = mitk::MultiLabelSurfaceNetsExtractor::GetImageToWorldMatrix(nullptr);
    CPPUNIT_ASSERT(matrix != nullptr);
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        CPPUNIT_ASSERT_DOUBLES_EQUAL(i == j ? 1.0 : 0.0, matrix->GetElement(i, j), 1e-9);
  }

  void GetImageToWorldMatrix_StripsSpacingAndKeepsOrigin()
  {
    // Geometry with non-zero origin and non-unit spacing, identity direction.
    // Confirms the spacing is removed and the origin column survives, so polydata in
    // mm-scaled image-local coords lands at world = local + origin.
    auto geometry = mitk::Geometry3D::New();
    mitk::Vector3D spacing;
    spacing[0] = 2.0; spacing[1] = 3.0; spacing[2] = 4.0;
    geometry->SetSpacing(spacing);
    mitk::Point3D origin;
    origin[0] = 10.0; origin[1] = 20.0; origin[2] = 30.0;
    geometry->SetOrigin(origin);

    auto matrix = mitk::MultiLabelSurfaceNetsExtractor::GetImageToWorldMatrix(geometry);

    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, matrix->GetElement(0, 0), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, matrix->GetElement(1, 1), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, matrix->GetElement(2, 2), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(10.0, matrix->GetElement(0, 3), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, matrix->GetElement(1, 3), 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(30.0, matrix->GetElement(2, 3), 1e-9);

    // (1, 2, 3) in image-local coords → (11, 22, 33) in world.
    double world[4];
    const double local[4] = {1.0, 2.0, 3.0, 1.0};
    matrix->MultiplyPoint(local, world);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(11.0, world[0], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(22.0, world[1], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(33.0, world[2], 1e-9);
  }

  void GetImageToWorldMatrix_PreservesNonIdentityDirection()
  {
    // 90 degrees around z: x→y, y→-x, z→z.
    auto geometry = mitk::Geometry3D::New();
    mitk::AffineTransform3D::Pointer transform = mitk::AffineTransform3D::New();
    mitk::AffineTransform3D::MatrixType matrixComponent;
    matrixComponent[0][0] = 0.0; matrixComponent[0][1] = -1.0; matrixComponent[0][2] = 0.0;
    matrixComponent[1][0] = 1.0; matrixComponent[1][1] = 0.0;  matrixComponent[1][2] = 0.0;
    matrixComponent[2][0] = 0.0; matrixComponent[2][1] = 0.0;  matrixComponent[2][2] = 1.0;
    transform->SetMatrix(matrixComponent);
    geometry->SetIndexToWorldTransform(transform);

    auto matrix = mitk::MultiLabelSurfaceNetsExtractor::GetImageToWorldMatrix(geometry);

    // (1, 0, 0) in image-local → (0, 1, 0) in world after the rotation.
    double world[4];
    const double local[4] = {1.0, 0.0, 0.0, 1.0};
    matrix->MultiplyPoint(local, world);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, world[0], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, world[1], 1e-9);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, world[2], 1e-9);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkMultiLabelSurfaceNetsExtractor)
