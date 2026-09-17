/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkAndMaskGenerator.h>
#include <mitkIgnorePixelMaskGenerator.h>
#include <mitkImageMaskGenerator.h>
#include <mitkPlanarFigureMaskGenerator.h>

#include <mitkImageReadAccessor.h>
#include <mitkITKImageImport.h>
#include <mitkPlanarPolygon.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkImage.h>
#include <itkImageRegionIterator.h>
#include <itkMath.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
  using LabelValueType = mitk::AndMaskGenerator::LabelValueType;
  using LabelVectorType = std::vector<LabelValueType>;
  using DirectionType = itk::Matrix<double, 3, 3>;

  struct Grid
  {
    mitk::Point3D origin;
    mitk::Vector3D spacing;
    DirectionType direction;

    Grid()
    {
      origin.Fill(0.0);
      spacing.Fill(1.0);
      direction.SetIdentity();
    }
  };

  Grid MakeGrid(double originX, double originY, double originZ,
                double spacingX = 1.0, double spacingY = 1.0, double spacingZ = 1.0)
  {
    Grid grid;
    grid.origin[0] = originX;
    grid.origin[1] = originY;
    grid.origin[2] = originZ;
    grid.spacing[0] = spacingX;
    grid.spacing[1] = spacingY;
    grid.spacing[2] = spacingZ;
    return grid;
  }

  itk::Size<3> MakeSize(unsigned int x, unsigned int y, unsigned int z)
  {
    itk::Size<3> size;
    size[0] = x;
    size[1] = y;
    size[2] = z;
    return size;
  }

  itk::Size<4> MakeSize(unsigned int x, unsigned int y, unsigned int z, unsigned int t)
  {
    itk::Size<4> size;
    size[0] = x;
    size[1] = y;
    size[2] = z;
    size[3] = t;
    return size;
  }

  // flat buffer offset of the voxel at the in-plane position (u, v) of the slice at
  // `slice` along `axis`, in a cubic image of the given size
  std::size_t VoxelOffset(unsigned int axis, unsigned int slice, unsigned int u, unsigned int v, unsigned int size)
  {
    std::array<unsigned int, 3> index{};
    index[axis] = slice;
    index[0 == axis ? 1 : 0] = u;
    index[2 == axis ? 1 : 2] = v;

    return index[0] + size * (index[1] + size * index[2]);
  }

  // builds a 3D mitk::Image from raw values in the iteration order of itk::ImageRegionIterator
  template <typename TPixel>
  mitk::Image::Pointer BuildImage(const itk::Size<3>& size, const std::vector<TPixel>& values, const Grid& grid = Grid())
  {
    using ImageType = itk::Image<TPixel, 3>;

    typename ImageType::IndexType start;
    start.Fill(0);
    typename ImageType::RegionType region(start, size);

    typename ImageType::PointType origin;
    typename ImageType::SpacingType spacing;

    for (unsigned int i = 0; i < 3; ++i)
    {
      origin[i] = grid.origin[i];
      spacing[i] = grid.spacing[i];
    }

    auto itkImage = ImageType::New();
    itkImage->SetRegions(region);
    itkImage->SetOrigin(origin);
    itkImage->SetSpacing(spacing);
    itkImage->SetDirection(grid.direction);
    itkImage->Allocate();

    itk::ImageRegionIterator<ImageType> it(itkImage, region);
    auto valueIt = values.cbegin();

    for (it.GoToBegin(); !it.IsAtEnd(); ++it, ++valueIt)
      it.Set(*valueIt);

    return mitk::GrabItkImageMemory(itkImage, nullptr, nullptr, false);
  }

  // builds a 3D+t mitk::Image on the unit grid, time step by time step
  template <typename TPixel>
  mitk::Image::Pointer BuildImage(const itk::Size<4>& size, const std::vector<TPixel>& values)
  {
    using ImageType = itk::Image<TPixel, 4>;

    typename ImageType::IndexType start;
    start.Fill(0);

    auto itkImage = ImageType::New();
    itkImage->SetRegions(typename ImageType::RegionType(start, size));
    itkImage->Allocate();

    itk::ImageRegionIterator<ImageType> it(itkImage, itkImage->GetLargestPossibleRegion());
    auto valueIt = values.cbegin();

    for (it.GoToBegin(); !it.IsAtEnd(); ++it, ++valueIt)
      it.Set(*valueIt);

    // GrabItkImageMemory would only import the volume of the first time step
    return mitk::GrabItkImageMemoryChannel(itkImage.GetPointer(), nullptr, nullptr, false);
  }

  // plane through the voxel centers of the slice at `slice` along `axis`, with in-plane
  // coordinates in mm along the two remaining image axes
  mitk::PlaneGeometry::Pointer MakeSlicePlane(const mitk::Image* image, unsigned int axis, unsigned int slice)
  {
    const auto* geometry = image->GetGeometry();
    const unsigned int right = 0 == axis ? 1 : 0;
    const unsigned int down = 2 == axis ? 1 : 2;

    mitk::Vector3D spacing;
    spacing[0] = geometry->GetSpacing()[right];
    spacing[1] = geometry->GetSpacing()[down];
    spacing[2] = geometry->GetSpacing()[axis];

    auto plane = mitk::PlaneGeometry::New();
    plane->InitializeStandardPlane(image->GetDimension(right), image->GetDimension(down),
      geometry->GetAxisVector(right), geometry->GetAxisVector(down), &spacing);

    mitk::Point3D index;
    index.Fill(0.0);
    index[axis] = slice;

    mitk::Point3D origin;
    geometry->IndexToWorld(index, origin);
    plane->SetOrigin(origin);

    return plane;
  }

  mitk::ImageMaskGenerator::Pointer MakeImageMaskGenerator(const mitk::Image* inputImage, const mitk::Image* mask)
  {
    auto generator = mitk::ImageMaskGenerator::New();
    generator->SetInputImage(inputImage);
    generator->SetImageMask(mask);
    return generator;
  }

  mitk::IgnorePixelMaskGenerator::Pointer MakeIgnoreGenerator(const mitk::Image* inputImage,
    mitk::IgnorePixelMaskGenerator::RealType ignoredValue = 0)
  {
    auto generator = mitk::IgnorePixelMaskGenerator::New();
    generator->SetInputImage(inputImage);
    generator->SetIgnoredPixelValue(ignoredValue);
    return generator;
  }

  mitk::AndMaskGenerator::Pointer MakeAndGenerator(mitk::MaskGenerator* primary,
                                                   mitk::MaskGenerator* secondary,
                                                   LabelValueType label)
  {
    auto generator = mitk::AndMaskGenerator::New();
    generator->SetPrimaryMaskGenerator(primary);
    generator->SetSecondaryMaskGenerator(secondary);
    generator->SetSecondaryLabelValue(label);
    return generator;
  }

  mitk::Point2D MakePoint2D(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  mitk::PlanarPolygon::Pointer MakePolygon(mitk::PlaneGeometry* plane, const std::vector<mitk::Point2D>& points)
  {
    auto figure = mitk::PlanarPolygon::New();
    figure->SetPlaneGeometry(plane);
    figure->PlaceFigure(points[0]);

    for (unsigned int i = 1; i < points.size(); ++i)
      figure->SetControlPoint(i, points[i], true);

    return figure;
  }

  // corners on the pixel boundaries around the pixel centers 1 and 2, in plane coordinates (mm)
  std::vector<mitk::Point2D> SquareCoveringPixelsOneAndTwo(double spacingX, double spacingY)
  {
    return { MakePoint2D(0.5 * spacingX, 0.5 * spacingY),
             MakePoint2D(2.5 * spacingX, 0.5 * spacingY),
             MakePoint2D(2.5 * spacingX, 2.5 * spacingY),
             MakePoint2D(0.5 * spacingX, 2.5 * spacingY) };
  }

  template <typename TPixel>
  std::vector<TPixel> ReadPixels(const mitk::Image* image)
  {
    const std::size_t count = image->GetDimension(0) * image->GetDimension(1) * image->GetDimension(2);
    mitk::ImageReadAccessor access(image);
    const auto* data = static_cast<const TPixel*>(access.GetData());

    return std::vector<TPixel>(data, data + count);
  }

  LabelVectorType ReadMask(const mitk::Image* mask)
  {
    CPPUNIT_ASSERT_MESSAGE("Mask does not have the label pixel type.",
      mask->GetPixelType() == mitk::MakeScalarPixelType<LabelValueType>());

    return ReadPixels<LabelValueType>(mask);
  }
}

class mitkAndMaskGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkAndMaskGeneratorTestSuite);
  MITK_TEST(TestBinaryPrimaryWithIgnoreZeroSecondary);
  MITK_TEST(TestMultiLabelPrimaryAndSecondaryLabelSelection);
  MITK_TEST(TestNonLabelPixelTypes);
  MITK_TEST(TestSecondaryOffsetOnSameGrid);
  MITK_TEST(TestSecondaryWithFinerSpacing);
  MITK_TEST(TestPlanarFigurePrimaryWithVolumeSecondary);
  MITK_TEST(TestPlanarFigurePrimaryWithAnisotropicSpacing);
  MITK_TEST(TestPlanarFigurePrimaryWithFlippedAxes);
  MITK_TEST(TestPlanarFigureOnSagittalSlice);
  MITK_TEST(TestPlanarFigureOnCoronalSlice);
  MITK_TEST(TestTimePointIsForwardedToChainedGenerators);
  MITK_TEST(TestReferenceImageFollowsTimePoint);
  MITK_TEST(TestMisalignedGridsThrow);
  MITK_TEST(TestInvalidStateThrows);
  MITK_TEST(TestMTimeFollowsChainedGenerators);
  CPPUNIT_TEST_SUITE_END();

public:
  void TestBinaryPrimaryWithIgnoreZeroSecondary()
  {
    const auto size = MakeSize(2, 2, 2);
    auto image = BuildImage<short>(size, { 1, 0, 3, 0, 5, 6, 0, 8 });
    auto primaryMask = BuildImage<LabelValueType>(size, { 1, 1, 1, 1, 0, 0, 1, 1 });

    auto primary = MakeImageMaskGenerator(image, primaryMask);
    auto secondary = MakeIgnoreGenerator(image);
    auto generator = MakeAndGenerator(primary, secondary, 1);

    CPPUNIT_ASSERT_EQUAL(1u, generator->GetNumberOfMasks());
    CPPUNIT_ASSERT(generator->GetReferenceImage().GetPointer() == image.GetPointer());

    auto result = generator->GetMask(0);
    CPPUNIT_ASSERT(result.IsNotNull());
    CPPUNIT_ASSERT_EQUAL(3u, result->GetDimension());
    CPPUNIT_ASSERT(mitk::Equal(*result->GetGeometry(), *primaryMask->GetGeometry(), mitk::eps, true));
    CPPUNIT_ASSERT(LabelVectorType({ 1, 0, 1, 0, 0, 0, 0, 1 }) == ReadMask(result));

    // results are not cached but reproducible
    auto secondResult = generator->GetMask(0);
    CPPUNIT_ASSERT(secondResult.GetPointer() != result.GetPointer());
    CPPUNIT_ASSERT(ReadMask(result) == ReadMask(secondResult));
  }

  void TestMultiLabelPrimaryAndSecondaryLabelSelection()
  {
    const auto size = MakeSize(2, 2, 2);
    auto primaryMask = BuildImage<LabelValueType>(size, { 1, 2, 3, 4, 1, 2, 3, 4 });
    auto secondaryMask = BuildImage<LabelValueType>(size, { 1, 2, 2, 1, 0, 0, 7, 7 });
    auto primary = MakeImageMaskGenerator(primaryMask, primaryMask);
    auto secondary = MakeImageMaskGenerator(primaryMask, secondaryMask);

    auto generator = MakeAndGenerator(primary, secondary, 2);
    CPPUNIT_ASSERT(LabelVectorType({ 0, 2, 3, 0, 0, 0, 0, 0 }) == ReadMask(generator->GetMask(0)));

    generator->SetSecondaryLabelValue(1);
    CPPUNIT_ASSERT(LabelVectorType({ 1, 0, 0, 4, 0, 0, 0, 0 }) == ReadMask(generator->GetMask(0)));

    // a label the secondary mask does not contain selects nothing
    generator->SetSecondaryLabelValue(9);
    CPPUNIT_ASSERT(LabelVectorType(8, 0) == ReadMask(generator->GetMask(0)));
  }

  void TestNonLabelPixelTypes()
  {
    const auto size = MakeSize(2, 2, 2);
    auto primaryMask = BuildImage<unsigned char>(size, { 1, 2, 3, 4, 1, 2, 3, 4 });
    auto secondaryMask = BuildImage<float>(size, { 1, 2, 2, 1, 0, 0, 7, 7 });
    auto generator = MakeAndGenerator(MakeImageMaskGenerator(primaryMask, primaryMask),
                                      MakeImageMaskGenerator(primaryMask, secondaryMask), 2);

    auto result = generator->GetMask(0);
    CPPUNIT_ASSERT(result->GetPixelType() == mitk::MakeScalarPixelType<LabelValueType>());
    CPPUNIT_ASSERT(mitk::Equal(*result->GetGeometry(), *primaryMask->GetGeometry(), mitk::eps, true));
    CPPUNIT_ASSERT(LabelVectorType({ 0, 2, 3, 0, 0, 0, 0, 0 }) == ReadMask(result));
  }

  void TestSecondaryOffsetOnSameGrid()
  {
    auto primaryMask = BuildImage<LabelValueType>(MakeSize(3, 3, 1), LabelVectorType(9, 1));
    auto primary = MakeImageMaskGenerator(primaryMask, primaryMask);

    // secondary covers the primary voxels with x and y in {1, 2}
    auto shifted = BuildImage<LabelValueType>(MakeSize(2, 2, 1), LabelVectorType(4, 1), MakeGrid(1, 1, 0));
    auto result = MakeAndGenerator(primary, MakeImageMaskGenerator(shifted, shifted), 1)->GetMask(0);
    CPPUNIT_ASSERT(LabelVectorType({ 0, 0, 0, 0, 1, 1, 0, 1, 1 }) == ReadMask(result));

    // secondary starts in front of the primary and covers only its voxel (0, 0)
    auto leading = BuildImage<LabelValueType>(MakeSize(2, 2, 1), LabelVectorType(4, 1), MakeGrid(-1, -1, 0));
    result = MakeAndGenerator(primary, MakeImageMaskGenerator(leading, leading), 1)->GetMask(0);
    CPPUNIT_ASSERT(LabelVectorType({ 1, 0, 0, 0, 0, 0, 0, 0, 0 }) == ReadMask(result));
  }

  void TestSecondaryWithFinerSpacing()
  {
    auto primaryMask = BuildImage<LabelValueType>(MakeSize(3, 3, 1), LabelVectorType(9, 1));
    auto primary = MakeImageMaskGenerator(primaryMask, primaryMask);

    // half the spacing: primary voxel (i, j) maps to secondary voxel (2i, 2j); (2, 2) is not selected
    LabelVectorType fineValues(36, 1);
    fineValues[2 + 6 * 2] = 0;
    auto fine = BuildImage<LabelValueType>(MakeSize(6, 6, 1), fineValues, MakeGrid(0, 0, 0, 0.5, 0.5, 1));

    auto result = MakeAndGenerator(primary, MakeImageMaskGenerator(fine, fine), 1)->GetMask(0);
    CPPUNIT_ASSERT(LabelVectorType({ 1, 1, 1, 1, 0, 1, 1, 1, 1 }) == ReadMask(result));
  }

  void TestPlanarFigurePrimaryWithVolumeSecondary()
  {
    this->VerifyPlanarFigureCase(Grid());
  }

  void TestPlanarFigurePrimaryWithAnisotropicSpacing()
  {
    this->VerifyPlanarFigureCase(MakeGrid(0, 0, 0, 2, 2, 3));
  }

  void TestPlanarFigurePrimaryWithFlippedAxes()
  {
    Grid grid;
    grid.direction(0, 0) = -1.0;
    grid.direction(1, 1) = -1.0;
    this->VerifyPlanarFigureCase(grid);
  }

  void TestPlanarFigureOnSagittalSlice()
  {
    this->VerifyPlanarFigureOnAxis(0);
  }

  void TestPlanarFigureOnCoronalSlice()
  {
    this->VerifyPlanarFigureOnAxis(1);
  }

  void TestTimePointIsForwardedToChainedGenerators()
  {
    // two time steps with different patterns of the ignored values 5 (primary) and 0 (secondary)
    auto image = BuildImage<short>(MakeSize(2, 2, 2, 2), { 1, 0, 5, 2, 3, 4, 5, 0,
                                                           5, 5, 1, 1, 0, 0, 2, 2 });

    auto generator = MakeAndGenerator(MakeIgnoreGenerator(image, 5), MakeIgnoreGenerator(image, 0), 1);
    const auto* timeGeometry = image->GetTimeGeometry();

    generator->SetTimePoint(timeGeometry->TimeStepToTimePoint(0));
    CPPUNIT_ASSERT(LabelVectorType({ 1, 0, 0, 1, 1, 1, 0, 0 }) == ReadMask(generator->GetMask(0)));

    generator->SetTimePoint(timeGeometry->TimeStepToTimePoint(1));
    CPPUNIT_ASSERT(LabelVectorType({ 0, 0, 1, 1, 0, 0, 1, 1 }) == ReadMask(generator->GetMask(0)));
  }

  void TestReferenceImageFollowsTimePoint()
  {
    // 4x4x2 with two time steps that differ in value, so the extracted slice identifies the time step
    std::vector<short> values(2 * 4 * 4 * 2, 10);
    std::fill(values.begin() + 4 * 4 * 2, values.end(), 20);
    auto image = BuildImage<short>(MakeSize(4, 4, 2, 2), values);

    auto polygon = MakePolygon(MakeSlicePlane(image, 2, 1), SquareCoveringPixelsOneAndTwo(1.0, 1.0));

    auto primary = mitk::PlanarFigureMaskGenerator::New();
    primary->SetInputImage(image);
    primary->SetPlanarFigure(polygon.GetPointer());

    auto generator = MakeAndGenerator(primary, MakeIgnoreGenerator(image), 1);
    const auto* timeGeometry = image->GetTimeGeometry();

    // the reference image is asked for before any mask, so the time point has to reach the
    // primary generator here as well
    generator->SetTimePoint(timeGeometry->TimeStepToTimePoint(1));
    CPPUNIT_ASSERT(std::vector<short>(16, 20) == ReadPixels<short>(generator->GetReferenceImage()));

    generator->SetTimePoint(timeGeometry->TimeStepToTimePoint(0));
    CPPUNIT_ASSERT(std::vector<short>(16, 10) == ReadPixels<short>(generator->GetReferenceImage()));
  }

  void TestMisalignedGridsThrow()
  {
    const auto size = MakeSize(2, 2, 1);
    auto primaryMask = BuildImage<LabelValueType>(size, LabelVectorType(4, 1));
    auto primary = MakeImageMaskGenerator(primaryMask, primaryMask);

    // sub-voxel offset
    auto shifted = BuildImage<LabelValueType>(size, LabelVectorType(4, 1), MakeGrid(0.5, 0, 0));
    auto generator = MakeAndGenerator(primary, MakeImageMaskGenerator(shifted, shifted), 1);
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception);

    // coarser spacing: every second primary voxel falls between secondary voxels
    auto coarse = BuildImage<LabelValueType>(size, LabelVectorType(4, 1), MakeGrid(0, 0, 0, 2, 1, 1));
    generator = MakeAndGenerator(primary, MakeImageMaskGenerator(coarse, coarse), 1);
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception);

    // rotated grid
    Grid rotated;
    const double angle = 30.0 * itk::Math::pi / 180.0;
    rotated.direction(0, 0) = std::cos(angle);
    rotated.direction(0, 1) = -std::sin(angle);
    rotated.direction(1, 0) = std::sin(angle);
    rotated.direction(1, 1) = std::cos(angle);
    auto rotatedMask = BuildImage<LabelValueType>(size, LabelVectorType(4, 1), rotated);
    generator = MakeAndGenerator(primary, MakeImageMaskGenerator(rotatedMask, rotatedMask), 1);
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception);
  }

  void TestInvalidStateThrows()
  {
    const auto size = MakeSize(2, 2, 1);
    auto mask = BuildImage<LabelValueType>(size, LabelVectorType(4, 1));
    auto primary = MakeImageMaskGenerator(mask, mask);
    auto secondary = MakeImageMaskGenerator(mask, mask);

    auto generator = mitk::AndMaskGenerator::New();
    CPPUNIT_ASSERT_THROW(generator->GetNumberOfMasks(), mitk::Exception);
    CPPUNIT_ASSERT_THROW(generator->GetReferenceImage(), mitk::Exception);
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception);

    // the chained generators carry the input image, this one has none of its own
    CPPUNIT_ASSERT_THROW(generator->SetInputImage(mask), mitk::Exception);

    generator->SetPrimaryMaskGenerator(primary);
    CPPUNIT_ASSERT(generator->GetPrimaryMaskGenerator() == primary.GetPointer());
    CPPUNIT_ASSERT_EQUAL(1u, generator->GetNumberOfMasks());
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception); // no secondary generator

    generator->SetSecondaryMaskGenerator(secondary);
    CPPUNIT_ASSERT(generator->GetSecondaryMaskGenerator() == secondary.GetPointer());
    CPPUNIT_ASSERT(!generator->GetSecondaryLabelValue().has_value());
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception); // no label value

    generator->SetSecondaryLabelValue(1);
    CPPUNIT_ASSERT(LabelValueType(1) == generator->GetSecondaryLabelValue());
    CPPUNIT_ASSERT_NO_THROW(generator->GetMask(0));
    CPPUNIT_ASSERT_THROW(generator->GetMask(1), mitk::Exception); // primary has a single mask

    CPPUNIT_ASSERT_EQUAL(0u, generator->GetSecondaryMaskID());
    generator->SetSecondaryMaskID(5);
    CPPUNIT_ASSERT_EQUAL(5u, generator->GetSecondaryMaskID());
    CPPUNIT_ASSERT_THROW(generator->GetMask(0), mitk::Exception); // secondary has a single mask
  }

  void TestMTimeFollowsChainedGenerators()
  {
    const auto size = MakeSize(2, 2, 1);
    auto mask = BuildImage<LabelValueType>(size, LabelVectorType(4, 1));
    auto primary = MakeImageMaskGenerator(mask, mask);
    auto secondary = MakeIgnoreGenerator(mask);
    auto generator = MakeAndGenerator(primary, secondary, 1);

    auto previous = generator->GetMTime();
    primary->Modified();
    CPPUNIT_ASSERT(generator->GetMTime() > previous);
    CPPUNIT_ASSERT(generator->GetMTime() >= primary->GetMTime());

    previous = generator->GetMTime();
    secondary->SetIgnoredPixelValue(7);
    CPPUNIT_ASSERT(generator->GetMTime() > previous);

    previous = generator->GetMTime();
    generator->SetSecondaryLabelValue(1);
    generator->SetPrimaryMaskGenerator(primary);
    CPPUNIT_ASSERT_EQUAL(previous, generator->GetMTime());

    generator->SetSecondaryLabelValue(3);
    CPPUNIT_ASSERT(generator->GetMTime() > previous);
  }

private:
  // 4x4x4 image of value 10, with a figure on slice 1 along the given axis covering the
  // in-plane pixels [1,2]x[1,2], and zero voxels inside the figure, inside its footprint but
  // on the next slice, and on the slice but outside the figure
  void VerifyPlanarFigureOnAxis(unsigned int axis)
  {
    const unsigned int size = 4;
    const unsigned int slice = 1;

    std::vector<short> values(size * size * size, 10);
    values[VoxelOffset(axis, slice, 1, 1, size)] = 0;
    values[VoxelOffset(axis, slice + 1, 2, 2, size)] = 0;
    values[VoxelOffset(axis, slice, 0, 0, size)] = 0;

    auto image = BuildImage<short>(MakeSize(size, size, size), values);
    auto polygon = MakePolygon(MakeSlicePlane(image, axis, slice), SquareCoveringPixelsOneAndTwo(1.0, 1.0));

    auto primary = mitk::PlanarFigureMaskGenerator::New();
    primary->SetInputImage(image);
    primary->SetPlanarFigure(polygon.GetPointer());

    auto generator = MakeAndGenerator(primary, MakeIgnoreGenerator(image), 1);

    // the extracted slice carries the voxels of the requested image slice, indexed by the two
    // remaining image axes in increasing order
    auto referenceImage = primary->GetReferenceImage();
    CPPUNIT_ASSERT_EQUAL(2u, referenceImage->GetDimension());
    CPPUNIT_ASSERT_EQUAL(size, referenceImage->GetDimension(0));
    CPPUNIT_ASSERT_EQUAL(size, referenceImage->GetDimension(1));
    CPPUNIT_ASSERT(std::vector<short>({  0, 10, 10, 10,
                                        10,  0, 10, 10,
                                        10, 10, 10, 10,
                                        10, 10, 10, 10 }) == ReadPixels<short>(referenceImage));

    CPPUNIT_ASSERT(LabelVectorType({ 0, 0, 0, 0,
                                     0, 1, 1, 0,
                                     0, 1, 1, 0,
                                     0, 0, 0, 0 }) == ReadMask(primary->GetMask(0)));

    CPPUNIT_ASSERT(LabelVectorType({ 0, 0, 0, 0,
                                     0, 0, 1, 0,
                                     0, 1, 1, 0,
                                     0, 0, 0, 0 }) == ReadMask(generator->GetMask(0)));
  }

  // 4x4x3 image with zero voxels at (1,1,1) inside the figure on slice 1, at (2,2,0) inside the
  // footprint of the figure but on another slice, and at (0,0,2) outside the footprint
  void VerifyPlanarFigureCase(const Grid& grid)
  {
    std::vector<short> values(4 * 4 * 3, 10);
    values[1 + 4 * (1 + 4 * 1)] = 0;
    values[2 + 4 * (2 + 4 * 0)] = 0;
    values[0 + 4 * (0 + 4 * 2)] = 0;
    auto image = BuildImage<short>(MakeSize(4, 4, 3), values, grid);

    auto polygon = MakePolygon(image->GetSlicedGeometry()->GetPlaneGeometry(1),
                               SquareCoveringPixelsOneAndTwo(grid.spacing[0], grid.spacing[1]));

    auto primary = mitk::PlanarFigureMaskGenerator::New();
    primary->SetInputImage(image);
    primary->SetPlanarFigure(polygon.GetPointer());

    auto secondary = MakeIgnoreGenerator(image);
    auto generator = MakeAndGenerator(primary, secondary, 1);

    // the 2D mask has to lie on the reference slice, otherwise it cannot be matched by world coordinates
    auto primaryMask = primary->GetMask(0);
    CPPUNIT_ASSERT_EQUAL(2u, primaryMask->GetDimension());
    CPPUNIT_ASSERT(mitk::Equal(*primaryMask->GetGeometry(), *primary->GetReferenceImage()->GetGeometry(), mitk::eps, true));
    CPPUNIT_ASSERT(LabelVectorType({ 0, 0, 0, 0,
                                     0, 1, 1, 0,
                                     0, 1, 1, 0,
                                     0, 0, 0, 0 }) == ReadMask(primaryMask));

    auto result = generator->GetMask(0);
    CPPUNIT_ASSERT_EQUAL(2u, result->GetDimension());
    CPPUNIT_ASSERT(mitk::Equal(*result->GetGeometry(), *primaryMask->GetGeometry(), mitk::eps, true));
    CPPUNIT_ASSERT(LabelVectorType({ 0, 0, 0, 0,
                                     0, 0, 1, 0,
                                     0, 1, 1, 0,
                                     0, 0, 0, 0 }) == ReadMask(result));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkAndMaskGenerator)
