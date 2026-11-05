/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureMaskGenerator.h>

#include <mitkImageReadAccessor.h>
#include <mitkITKImageImport.h>
#include <mitkPlanarCircle.h>
#include <mitkPlanarDoubleEllipse.h>
#include <mitkPlanarFourPointAngle.h>
#include <mitkPlanarLine.h>
#include <mitkPlanarPath.h>
#include <mitkPlaneGeometry.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itkImage.h>

#include <algorithm>
#include <numbers>
#include <vector>

namespace
{
  constexpr unsigned int ImageSize = 20;

  mitk::Image::Pointer CreateImage()
  {
    using ImageType = itk::Image<short, 3>;

    ImageType::SizeType size;
    size.Fill(ImageSize);
    size[2] = 3;

    auto itkImage = ImageType::New();
    itkImage->SetRegions(ImageType::RegionType(size));
    itkImage->Allocate();
    itkImage->FillBuffer(1);

    return mitk::GrabItkImageMemory(itkImage);
  }

  mitk::Point2D P(double x, double y)
  {
    mitk::Point2D point;
    point[0] = x;
    point[1] = y;
    return point;
  }

  std::vector<unsigned short> GetMaskPixels(const mitk::Image *image, const mitk::PlanarFigure *figure)
  {
    auto generator = mitk::PlanarFigureMaskGenerator::New();
    generator->SetInputImage(image);
    generator->SetPlanarFigure(figure);

    const auto mask = generator->GetMask(0);
    const auto numberOfPixels = mask->GetDimension(0) * mask->GetDimension(1);

    mitk::ImageReadAccessor accessor(mask);
    const auto *pixels = static_cast<const unsigned short *>(accessor.GetData());

    return std::vector<unsigned short>(pixels, pixels + numberOfPixels);
  }

  unsigned int CountMaskedPixels(const mitk::Image *image, const mitk::PlanarFigure *figure)
  {
    const auto pixels = GetMaskPixels(image, figure);
    return static_cast<unsigned int>(std::count_if(pixels.begin(), pixels.end(), [](unsigned short pixel) { return pixel != 0; }));
  }
}

class mitkPlanarFigureMaskGeneratorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPlanarFigureMaskGeneratorTestSuite);

  MITK_TEST(Ring_ExcludesHole);
  MITK_TEST(OpenFigure_CoversEveryContour);
  MITK_TEST(Path_CoversEverySegment);
  MITK_TEST(OpenFigure_IsClippedToImage);
  MITK_TEST(OpenFigure_RoundsToNearestVoxel);

  CPPUNIT_TEST_SUITE_END();

  mitk::Image::Pointer m_Image;
  mitk::PlaneGeometry::Pointer m_Plane;

public:
  void setUp() override
  {
    m_Image = CreateImage();
    m_Plane = mitk::PlaneGeometry::New();
    m_Plane->InitializeStandardPlane(m_Image->GetGeometry(), mitk::AnatomicalPlane::Axial, 1);
  }

  void tearDown() override
  {
    m_Plane = nullptr;
    m_Image = nullptr;
  }

  void Ring_ExcludesHole()
  {
    auto disc = mitk::PlanarCircle::New();
    disc->SetPlaneGeometry(m_Plane);
    disc->SetFromSpan(P(10, 10), P(18, 10));

    // A ring of half the radius.
    auto ring = mitk::PlanarDoubleEllipse::New();
    ring->SetPlaneGeometry(m_Plane);
    ring->SetFromSpan(P(10, 10), P(18, 10));

    const auto discPixels = CountMaskedPixels(m_Image, disc);
    const auto ringPixels = CountMaskedPixels(m_Image, ring);
    const auto holeArea = std::numbers::pi * 4.0 * 4.0;

    CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("The ring leaves its hole out", discPixels - holeArea, ringPixels, 10.0);
  }

  void OpenFigure_CoversEveryContour()
  {
    auto angle = mitk::PlanarFourPointAngle::New();
    angle->SetPlaneGeometry(m_Plane);
    angle->SetControlPoints({ P(2, 5), P(17, 5), P(5, 2), P(5, 17) });

    // Two lines of 16 pixels each that cross in one pixel.
    CPPUNIT_ASSERT_EQUAL(31u, CountMaskedPixels(m_Image, angle));
  }

  void Path_CoversEverySegment()
  {
    auto path = mitk::PlanarPath::New();
    path->SetPlaneGeometry(m_Plane);
    path->SetControlPoints({ P(2, 2), P(17, 2), P(17, 10), P(2, 10) });

    // Three segments of 16, 9 and 16 pixels that share their end pixels.
    CPPUNIT_ASSERT_EQUAL(39u, CountMaskedPixels(m_Image, path));
  }

  void OpenFigure_IsClippedToImage()
  {
    auto line = mitk::PlanarLine::New();
    line->SetPlaneGeometry(m_Plane);
    line->SetControlPoints({ P(10, 10), P(40, 10) });

    // Only the pixels 10 to 19 of the row lie in the image.
    CPPUNIT_ASSERT_EQUAL(10u, CountMaskedPixels(m_Image, line));

    // From outside to outside, one pixel per column.
    line->SetControlPoints({ P(-10, 0), P(30, 20) });
    CPPUNIT_ASSERT_EQUAL(ImageSize, CountMaskedPixels(m_Image, line));
  }

  void OpenFigure_RoundsToNearestVoxel()
  {
    mitk::Point3D index;
    index[0] = 0.0;
    index[1] = 10.0;
    index[2] = 1.0;

    mitk::Point3D voxelCenter;
    m_Image->GetGeometry()->IndexToWorld(index, voxelCenter);

    mitk::Point2D voxelCenterOnPlane;
    m_Plane->Map(voxelCenter, voxelCenterOnPlane);

    const auto getLineMask = [this, &voxelCenterOnPlane](double offset) {
      const auto y = voxelCenterOnPlane[1] + offset;

      auto line = mitk::PlanarLine::New();
      line->SetPlaneGeometry(m_Plane);
      line->SetControlPoints({ P(2, y), P(17, y) });
      return GetMaskPixels(m_Image, line);
    };

    const auto onVoxelCenters = getLineMask(0.0);

    CPPUNIT_ASSERT_EQUAL(16, static_cast<int>(std::count(onVoxelCenters.begin(), onVoxelCenters.end(), 1)));
    CPPUNIT_ASSERT_MESSAGE("A line below the voxel centers belongs to the nearest row", onVoxelCenters == getLineMask(-0.3));
    CPPUNIT_ASSERT_MESSAGE("A line above the voxel centers belongs to the nearest row", onVoxelCenters == getLineMask(0.3));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPlanarFigureMaskGenerator)
