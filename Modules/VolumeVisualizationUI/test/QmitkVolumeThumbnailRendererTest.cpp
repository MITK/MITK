/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkVolumeThumbnailRenderer.h>

#include <mitkExceptionMacro.h>
#include <mitkImage.h>
#include <mitkImageWriteAccessor.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkImageData.h>
#include <vtkMatrix4x4.h>

class QmitkVolumeThumbnailRendererTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(QmitkVolumeThumbnailRendererTestSuite);
  MITK_TEST(CreateVolume_KeepsSmallAxesWhole);
  MITK_TEST(CreateVolume_AveragesBlocksAlongLongAxis);
  MITK_TEST(CreateVolume_PlacesVoxelsAtTheirBlockCenters);
  MITK_TEST(CreateVolume_InvalidInput_Throws);
  CPPUNIT_TEST_SUITE_END();

public:
  /** 4 x 3 x 600 voxels whose value is their slice index, in a geometry that
   * is rotated, scaled and moved, so that every part of the mapping shows. */
  static mitk::Image::Pointer CreateImage()
  {
    auto image = mitk::Image::New();
    unsigned int dimensions[3] = {4, 3, 600};
    image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);

    {
      mitk::ImageWriteAccessor accessor(image);
      auto *pixels = static_cast<short *>(accessor.GetData());

      for (unsigned int z = 0; z < 600; ++z)
      {
        for (unsigned int i = 0; i < 4 * 3; ++i)
          pixels[z * 4 * 3 + i] = static_cast<short>(z);
      }
    }

    auto transform = mitk::AffineTransform3D::New();
    auto matrix = transform->GetMatrix();

    // A quarter turn about z, then spacings of 0.5, 2 and 3.
    matrix(0, 0) = 0.0;  matrix(0, 1) = -2.0; matrix(0, 2) = 0.0;
    matrix(1, 0) = 0.5;  matrix(1, 1) = 0.0;  matrix(1, 2) = 0.0;
    matrix(2, 0) = 0.0;  matrix(2, 1) = 0.0;  matrix(2, 2) = 3.0;

    mitk::Vector3D offset;
    mitk::FillVector3D(offset, 10.0, -20.0, 30.0);

    transform->SetMatrix(matrix);
    transform->SetOffset(offset);

    image->GetGeometry()->SetIndexToWorldTransform(transform);

    return image;
  }

  static void CreateVolume(const mitk::Image *image)
  {
    QmitkVolumeThumbnailRenderer::CreateVolume(image);
  }

  void CreateVolume_KeepsSmallAxesWhole()
  {
    const auto volume = QmitkVolumeThumbnailRenderer::CreateVolume(CreateImage());

    int dimensions[3];
    volume.ImageData->GetDimensions(dimensions);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Short axis reduced", 4, dimensions[0]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Short axis reduced", 3, dimensions[1]);
  }

  void CreateVolume_AveragesBlocksAlongLongAxis()
  {
    const auto volume = QmitkVolumeThumbnailRenderer::CreateVolume(CreateImage());

    int dimensions[3];
    volume.ImageData->GetDimensions(dimensions);

    // 600 slices need three to a voxel to fit into 256.
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong length of reduced axis", 200, dimensions[2]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong spacing of reduced axis", 9.0, volume.ImageData->GetSpacing()[2]);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong spacing of kept axis", 0.5, volume.ImageData->GetSpacing()[0]);

    // Voxel k averages slices 3k, 3k + 1 and 3k + 2.
    for (int k : {0, 1, 99, 199})
    {
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Wrong average of a block", 3.0 * k + 1.0,
        volume.ImageData->GetScalarComponentAsDouble(1, 1, k, 0));
    }
  }

  void CreateVolume_PlacesVoxelsAtTheirBlockCenters()
  {
    auto image = CreateImage();
    const auto volume = QmitkVolumeThumbnailRenderer::CreateVolume(image);

    for (int k : {0, 7, 199})
    {
      const double index[4] = {2.0, 1.0, static_cast<double>(k), 1.0};
      double world[4];
      volume.IndexToWorld->MultiplyPoint(index, world);

      // The block of voxel k is centered on slice 3k + 1.
      mitk::Point3D imageIndex;
      mitk::FillVector3D(imageIndex, 2.0, 1.0, 3.0 * k + 1.0);

      mitk::Point3D expected;
      image->GetGeometry()->IndexToWorld(imageIndex, expected);

      for (int i = 0; i < 3; ++i)
        CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("Voxel placed off its block", expected[i], world[i], 1e-9);
    }
  }

  void CreateVolume_InvalidInput_Throws()
  {
    CPPUNIT_ASSERT_THROW(CreateVolume(nullptr), mitk::Exception);
    CPPUNIT_ASSERT_THROW(CreateVolume(mitk::Image::New()), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(QmitkVolumeThumbnailRenderer)
