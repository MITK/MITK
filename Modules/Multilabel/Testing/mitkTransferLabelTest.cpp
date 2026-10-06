/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkExtractSliceFilter.h>
#include <mitkGeometry3D.h>
#include <mitkIOUtil.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkLabelSetImage.h>
#include <mitkSliceNavigationController.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

#include <algorithm>
#include <array>
#include <utility>

class mitkTransferLabelTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkTransferLabelTestSuite);
  MITK_TEST(TestTransfer_defaults);
  MITK_TEST(TestTransfer_Merge_RegardLocks);
  MITK_TEST(TestTransfer_Merge_IgnoreLocks);
  MITK_TEST(TestTransfer_Replace_RegardLocks);
  MITK_TEST(TestTransfer_Replace_IgnoreLocks);
  MITK_TEST(TestTransfer_multipleLabels);
  MITK_TEST(TestTransfer_Merge_RegardLocks_AtTimeStep);
  MITK_TEST(TestTransfer_Merge_IgnoreLocks_AtTimeStep);
  MITK_TEST(TestTransfer_Replace_RegardLocks_AtTimeStep);
  MITK_TEST(TestTransfer_Replace_IgnoreLocks_AtTimeStep);
  MITK_TEST(TestTransfer_multipleLabels_AtTimeStep);
  MITK_TEST(TestTransfer_SubGeometry_Merge);
  MITK_TEST(TestTransfer_SubGeometry_Replace);
  MITK_TEST(TestTransfer_SubGeometry_EveryVoxel);
  MITK_TEST(TestTransfer_SubGeometry_Replace_LockedBackground);
  MITK_TEST(TestTransfer_SameImage_NoChaining);
  MITK_TEST(TestTransfer_RepeatedMapping);
  MITK_TEST(TestTransfer_ConflictInLaterGroupPair);
  MITK_TEST(TestTransfer_StaticSource_AtTimeStep);
  MITK_TEST(TestTransfer_InvalidInput);
  MITK_TEST(TestTransferSurface_RegardLocks);
  MITK_TEST(TestTransferSurface_IgnoreLocks);
  MITK_TEST(TestTransferSurface_LockedBackground);
  MITK_TEST(TestTransferSurface_AtTimeStep);
  MITK_TEST(TestTransferSlice_ViewDirections);
  MITK_TEST(TestTransferSlice_TiltedGeometry);
  MITK_TEST(TestTransferSlice_Locks);
  MITK_TEST(TestTransferSlice_Misaligned);
  CPPUNIT_TEST_SUITE_END();

private:
  mitk::MultiLabelSegmentation::Pointer m_SourceImage;

public:
  void setUp() override
  {
    m_SourceImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_source.nrrd"));
  }

  void tearDown() override
  {
    m_SourceImage = nullptr;
  }

  void TestTransfer_defaults()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedUnlabeledImage);

    CPPUNIT_ASSERT_MESSAGE("Transfer with default settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with default settings + exterior lock failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Merge_RegardLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_regardLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedUnlabeledImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + regardLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + regardLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Merge_IgnoreLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_ignoreLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_ignoreLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedUnlabeledImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Replace_RegardLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedUnlabeledImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + regardLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + regardLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Replace_IgnoreLocks()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_ignoreLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_ignoreLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedUnlabeledImage, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }


  void TestTransfer_multipleLabels()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_multipleLabels.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_multipleLabels_lockedExterior.nrrd"));

    mitk::TransferLabelContent(m_SourceImage, destinationImage, { {1,1}, {3,1}, {2,4}, {4,2} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContent(m_SourceImage, destinationLockedUnlabeledImage, { {1,1}, {3,1}, {2,4}, {4,2} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer multiple labels (1->1, 3->1, 2->4, 4->2) with replace + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer multiple labels (1->1, 3->1, 2->4, 4->2) with replace + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Merge_RegardLocks_AtTimeStep()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_regardLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationLockedUnlabeledImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + regardLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + regardLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Merge_IgnoreLocks_AtTimeStep()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_ignoreLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_merge_ignoreLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationLockedUnlabeledImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with merge + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Replace_RegardLocks_AtTimeStep()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_regardLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationLockedUnlabeledImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + regardLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + regardLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  void TestTransfer_Replace_IgnoreLocks_AtTimeStep()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_ignoreLocks.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_replace_ignoreLocks_lockedExterior.nrrd"));

    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationLockedUnlabeledImage, 0, { {1,1} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer with replace + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }


  void TestTransfer_multipleLabels_AtTimeStep()
  {
    auto destinationImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination.nrrd"));
    auto destinationLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_destination_lockedExterior.nrrd"));
    auto refmage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_multipleLabels.nrrd"));
    auto refLockedUnlabeledImage = mitk::IOUtil::Load<mitk::MultiLabelSegmentation>(GetTestDataFilePath("Multilabel/LabelTransferTest_result_multipleLabels_lockedExterior.nrrd"));

    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationImage, 0, { {1,1}, {3,1}, {2,4}, {4,2} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);
    mitk::TransferLabelContentAtTimeStep(m_SourceImage, destinationLockedUnlabeledImage, 0, { {1,1}, {3,1}, {2,4}, {4,2} }, mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_MESSAGE("Transfer multiple labels (1->1, 3->1, 2->4, 4->2) with replace + ignoreLocks settings failed",
      mitk::Equal(*(destinationImage.GetPointer()), *(refmage.GetPointer()), mitk::eps, false));
    CPPUNIT_ASSERT_MESSAGE("Transfer multiple labels (1->1, 3->1, 2->4, 4->2) with replace + ignoreLocks + exterior lock settings failed",
      mitk::Equal(*(destinationLockedUnlabeledImage.GetPointer()), *(refLockedUnlabeledImage.GetPointer()), mitk::eps, false));
  }

  using Index = std::array<std::size_t, 3>;

  static mitk::MultiLabelSegmentation::Pointer CreateSurfaceTestSegmentation(unsigned int timeSteps, bool tilted = false)
  {
    auto referenceImage = mitk::Image::New();
    unsigned int dimensions[4] = { 40, 40, 40, timeSteps };
    referenceImage->Initialize(mitk::MakeScalarPixelType<char>(), 1 < timeSteps ? 4 : 3, dimensions);

    if (tilted)
    {
      // Slightly rotated, like scans that are not aligned with the world axes.
      mitk::Vector3D axis;
      axis[0] = 1.0;
      axis[1] = 2.0;
      axis[2] = 3.0;
      axis.Normalize();

      auto indexToWorld = mitk::AffineTransform3D::New();
      indexToWorld->Rotate3D(axis, 0.1);
      referenceImage->GetGeometry()->SetIndexToWorldTransform(indexToWorld);
    }

    auto segmentation = mitk::MultiLabelSegmentation::New();
    segmentation->Initialize(referenceImage);

    auto lockedLabel = mitk::Label::New(1, "Locked");
    lockedLabel->SetLocked(true);
    segmentation->AddLabel(lockedLabel, 0);

    auto unlockedLabel = mitk::Label::New(2, "Unlocked");
    unlockedLabel->SetLocked(false);
    segmentation->AddLabel(unlockedLabel, 0);

    segmentation->AddLabel(mitk::Label::New(3, "Target"), 0);

    return segmentation;
  }

  /** Creates a cube spanning [first, last] on every axis. With the default image geometry, voxel centers lie on
   * integer world coordinates, so a cube from 9.5 to 20.5 covers the indices 10 to 20. */
  static mitk::Surface::Pointer CreateCubeSurface(double first, double last, const mitk::TimeGeometry* timeGeometry, mitk::TimeStepType timeStep)
  {
    auto points = vtkSmartPointer<vtkPoints>::New();
    for (int corner = 0; corner < 8; ++corner)
      points->InsertNextPoint(corner & 1 ? last : first, corner & 2 ? last : first, corner & 4 ? last : first);

    const vtkIdType faces[6][4] = { { 0, 2, 3, 1 }, { 4, 5, 7, 6 }, { 0, 1, 5, 4 }, { 2, 6, 7, 3 }, { 0, 4, 6, 2 }, { 1, 3, 7, 5 } };
    auto polys = vtkSmartPointer<vtkCellArray>::New();
    for (const auto& face : faces)
      polys->InsertNextCell(4, face);

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetPolys(polys);

    // Shares the time steps of the segmentation, like the result of the surface interpolation does.
    auto surfaceTimeGeometry = timeGeometry->Clone();
    surfaceTimeGeometry->ReplaceTimeStepGeometries(mitk::Geometry3D::New());

    auto surface = mitk::Surface::New();
    surface->SetTimeGeometry(surfaceTimeGeometry);
    surface->Expand(static_cast<unsigned int>(timeGeometry->CountTimeSteps()));
    surface->SetVtkPolyData(polyData, static_cast<unsigned int>(timeStep));

    return surface;
  }

  static std::size_t ToLinearIndex(const mitk::Image* image, mitk::TimeStepType timeStep, const Index& index)
  {
    const std::size_t sizeX = image->GetDimension(0);
    const std::size_t sizeY = image->GetDimension(1);
    const std::size_t sizeZ = image->GetDimension(2);

    return ((timeStep * sizeZ + index[2]) * sizeY + index[1]) * sizeX + index[0];
  }

  static void SetPixel(mitk::Image* image, mitk::TimeStepType timeStep, const Index& index, mitk::Label::PixelType value)
  {
    {
      mitk::ImageWriteAccessor accessor(image);
      static_cast<mitk::Label::PixelType*>(accessor.GetData())[ToLinearIndex(image, timeStep, index)] = value;
    }

    image->Modified();
  }

  static mitk::Label::PixelType GetPixel(const mitk::Image* image, mitk::TimeStepType timeStep, const Index& index)
  {
    mitk::ImageReadAccessor accessor(image);
    return static_cast<const mitk::Label::PixelType*>(accessor.GetData())[ToLinearIndex(image, timeStep, index)];
  }

  static void TransferCube(mitk::MultiLabelSegmentation* segmentation, double first, double last, mitk::TimeStepType timeStep,
    bool backgroundLocked, mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle)
  {
    mitk::TransferSurfaceContentAtTimeStep(CreateCubeSurface(first, last, segmentation->GetTimeGeometry(), timeStep),
      segmentation->GetGroupImage(0), segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0)),
      timeStep, 3, mitk::MultiLabelSegmentation::UNLABELED_VALUE, backgroundLocked, overwriteStyle);
  }

  void TestTransferSurface_RegardLocks()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    auto* groupImage = segmentation->GetGroupImage(0);

    SetPixel(groupImage, 0, { 12, 15, 15 }, 1);
    SetPixel(groupImage, 0, { 13, 15, 15 }, 2);
    SetPixel(groupImage, 0, { 5, 5, 5 }, 3);

    TransferCube(segmentation, 9.5, 20.5, 0, false, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Background inside the surface was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 15, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("First voxel inside the surface was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 10, 10, 10 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last voxel inside the surface was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 20, 20, 20 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel in front of the surface was assigned",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, { 9, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel behind the surface was assigned",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, { 21, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked label inside the surface was overwritten",
      mitk::Label::PixelType(1), GetPixel(groupImage, 0, { 12, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlocked label inside the surface was not overwritten",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 13, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Target label outside the surface was removed",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 5, 5, 5 }));
  }

  void TestTransferSurface_IgnoreLocks()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    auto* groupImage = segmentation->GetGroupImage(0);

    SetPixel(groupImage, 0, { 12, 15, 15 }, 1);

    TransferCube(segmentation, 9.5, 20.5, 0, true, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked label inside the surface was not overwritten",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 12, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked background inside the surface was not overwritten",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 15, 15, 15 }));
  }

  void TestTransferSurface_LockedBackground()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    auto* groupImage = segmentation->GetGroupImage(0);

    SetPixel(groupImage, 0, { 13, 15, 15 }, 2);

    TransferCube(segmentation, 9.5, 20.5, 0, true, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked background inside the surface was overwritten",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, { 15, 15, 15 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlocked label inside the surface was not overwritten",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, { 13, 15, 15 }));
  }

  void TestTransferSurface_AtTimeStep()
  {
    auto segmentation = CreateSurfaceTestSegmentation(2);
    auto* groupImage = segmentation->GetGroupImage(0);

    // Reaches beyond the image, so the part outside of it has to be skipped.
    TransferCube(segmentation, 29.5, 60.5, 1, false, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel inside the surface was not assigned at the time step",
      mitk::Label::PixelType(3), GetPixel(groupImage, 1, { 30, 30, 30 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Last voxel of the image inside the surface was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 1, { 39, 39, 39 }));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel of another time step was assigned",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, { 30, 30, 30 }));
  }

  /** The slice through the voxel along the view direction, as the render windows extract it. */
  static mitk::Image::Pointer ExtractSliceThrough(const mitk::Image* image, mitk::AnatomicalPlane viewDirection, const Index& index)
  {
    auto navigationController = mitk::SliceNavigationController::New();
    navigationController->SetInputWorldTimeGeometry(image->GetTimeGeometry());
    navigationController->Update(viewDirection);

    mitk::Point3D indexPoint;
    for (unsigned int i = 0; i < 3; ++i)
      indexPoint[i] = static_cast<mitk::ScalarType>(index[i]);

    mitk::Point3D worldPoint;
    image->GetGeometry()->IndexToWorld(indexPoint, worldPoint);
    navigationController->SelectSliceByPoint(worldPoint);

    auto extractor = mitk::ExtractSliceFilter::New();
    extractor->SetInput(image);
    extractor->SetWorldGeometry(navigationController->GetCurrentPlaneGeometry());
    extractor->SetResliceTransformByGeometry(image->GetGeometry());
    extractor->SetVtkOutputRequest(false);
    extractor->Update();

    return extractor->GetOutput();
  }

  /** The index moved by i and j along the two axes within slices perpendicular to dim, and by k along dim. */
  static Index Offset(const Index& index, unsigned int dim, int i, int j, int k)
  {
    auto result = index;

    const auto move = [&result](unsigned int axis, int distance) {
      result[axis] = static_cast<std::size_t>(static_cast<int>(result[axis]) + distance);
    };

    move((dim + 1) % 3, i);
    move((dim + 2) % 3, j);
    move(dim, k);
    return result;
  }

  static void TransferSlice(mitk::MultiLabelSegmentation* segmentation, const mitk::Image* slice,
    mitk::Label::PixelType sourceLabel, mitk::MultiLabelSegmentation::OverwriteStyle overwriteStyle)
  {
    mitk::TransferSliceContentAtTimeStep(slice, segmentation->GetGroupImage(0),
      segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0)), 0, sourceLabel, 3,
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, overwriteStyle);
  }

  /**
   * Marks voxels of the unlocked label in the slice through the center and next to it, and transfers the slice
   * extracted along each view direction. Only the marked voxels in the slice may change, so any flip or shift in
   * mapping pixels to voxels makes the checks fail.
   */
  static void CheckTransferSliceAlongViewDirections(bool tilted)
  {
    const Index center = { 15, 15, 15 };
    const std::array<std::pair<mitk::AnatomicalPlane, unsigned int>, 3> viewDirections = { {
      { mitk::AnatomicalPlane::Axial, 2 }, { mitk::AnatomicalPlane::Coronal, 1 }, { mitk::AnatomicalPlane::Sagittal, 0 } } };

    for (const auto& [viewDirection, dim] : viewDirections)
    {
      auto segmentation = CreateSurfaceTestSegmentation(1, tilted);
      auto* groupImage = segmentation->GetGroupImage(0);

      const auto first = Offset(center, dim, 3, -5, 0);
      const auto second = Offset(center, dim, -7, 2, 0);
      const auto nextSlice = Offset(center, dim, 3, -5, 1);

      SetPixel(groupImage, 0, first, 2);
      SetPixel(groupImage, 0, second, 2);
      SetPixel(groupImage, 0, nextSlice, 2);

      TransferSlice(segmentation, ExtractSliceThrough(groupImage, viewDirection, center), 2,
        mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("First marked voxel in the slice was not assigned",
        mitk::Label::PixelType(3), GetPixel(groupImage, 0, first));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Second marked voxel in the slice was not assigned",
        mitk::Label::PixelType(3), GetPixel(groupImage, 0, second));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Marked voxel in the next slice was assigned",
        mitk::Label::PixelType(2), GetPixel(groupImage, 0, nextSlice));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel mirrored to the first marked one was assigned",
        mitk::Label::PixelType(0), GetPixel(groupImage, 0, Offset(center, dim, -3, 5, 0)));
    }
  }

  void TestTransferSlice_ViewDirections()
  {
    CheckTransferSliceAlongViewDirections(false);
  }

  void TestTransferSlice_TiltedGeometry()
  {
    CheckTransferSliceAlongViewDirections(true);
  }

  void TestTransferSlice_Locks()
  {
    const Index center = { 15, 15, 15 };
    const Index locked = { 12, 15, 15 };
    const Index unlocked = { 13, 15, 15 };

    for (const auto overwriteStyle : { mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks,
                                       mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks })
    {
      auto segmentation = CreateSurfaceTestSegmentation(1);
      auto* groupImage = segmentation->GetGroupImage(0);

      SetPixel(groupImage, 0, locked, 2);
      SetPixel(groupImage, 0, unlocked, 2);
      const auto slice = ExtractSliceThrough(groupImage, mitk::AnatomicalPlane::Axial, center);

      // Locked only after extracting, so that the slice still marks the voxel.
      SetPixel(groupImage, 0, locked, 1);

      TransferSlice(segmentation, slice, 2, overwriteStyle);

      const auto expectedLocked = mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks == overwriteStyle
        ? mitk::Label::PixelType(1)
        : mitk::Label::PixelType(3);

      CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked label was not treated according to the overwrite style",
        expectedLocked, GetPixel(groupImage, 0, locked));
      CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlocked label was not overwritten",
        mitk::Label::PixelType(3), GetPixel(groupImage, 0, unlocked));
    }
  }

  void TestTransferSlice_Misaligned()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    const auto slice = ExtractSliceThrough(segmentation->GetGroupImage(0), mitk::AnatomicalPlane::Axial, { 15, 15, 15 });

    auto* sliceGeometry = slice->GetGeometry();
    auto origin = sliceGeometry->GetOrigin();
    origin[0] += 0.5 * sliceGeometry->GetSpacing()[0];
    sliceGeometry->SetOrigin(origin);

    CPPUNIT_ASSERT_THROW(TransferSlice(segmentation, slice, 2, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks),
      mitk::Exception);
  }

  /** Sets the voxels of the first time step from first to last (inclusive) on every axis. */
  static void FillBox(mitk::Image* image, const Index& first, const Index& last, mitk::Label::PixelType value)
  {
    mitk::ImageWriteAccessor accessor(image);
    auto* pixels = static_cast<mitk::Label::PixelType*>(accessor.GetData());
    const std::size_t sizeX = image->GetDimension(0);
    const std::size_t sizeY = image->GetDimension(1);

    for (std::size_t z = first[2]; z <= last[2]; ++z)
      for (std::size_t y = first[1]; y <= last[1]; ++y)
        std::fill_n(pixels + (z * sizeY + y) * sizeX + first[0], last[0] - first[0] + 1, value);
  }

  /** A 3D label image of the given size with the default geometry, unlabeled except for the box from first to last. */
  static mitk::Image::Pointer CreateLabelImage(const Index& size, const Index& first, const Index& last, mitk::Label::PixelType value)
  {
    auto image = mitk::Image::New();
    unsigned int dimensions[3];
    for (unsigned int d = 0; d < 3; ++d)
      dimensions[d] = static_cast<unsigned int>(size[d]);

    image->Initialize(mitk::MakeScalarPixelType<mitk::Label::PixelType>(), 3, dimensions);

    FillBox(image, { 0, 0, 0 }, { size[0] - 1, size[1] - 1, size[2] - 1 }, mitk::MultiLabelSegmentation::UNLABELED_VALUE);
    FillBox(image, first, last, value);

    return image;
  }

  static constexpr Index SubSourceOffset = { 5, 7, 9 };

  /** A source of 10x12x14 voxels that lies at SubSourceOffset in the default 40^3 segmentation. The box from
   * (2, 3, 4) to (6, 9, 11) and the last voxel are marked with 1. Unequal sizes and an off-center box let
   * mixed-up axes or strides show. */
  static mitk::Image::Pointer CreateSubSource()
  {
    auto source = CreateLabelImage({ 10, 12, 14 }, { 2, 3, 4 }, { 6, 9, 11 }, 1);
    FillBox(source, { 9, 11, 13 }, { 9, 11, 13 }, 1);

    mitk::Point3D origin;
    for (unsigned int d = 0; d < 3; ++d)
      origin[d] = static_cast<mitk::ScalarType>(SubSourceOffset[d]);

    source->GetGeometry()->SetOrigin(origin);
    return source;
  }

  /** Destination for the sub source tests: the voxel values at the returned indices probe the transfer rules. */
  struct SubGeometryProbes
  {
    Index MarkedLocked = { 8, 11, 14 };         // source (3, 4, 5), locked label 1
    Index MarkedUnlocked = { 9, 11, 14 };       // source (4, 4, 5), unlocked label 2
    Index MarkedUnlabeled = { 7, 10, 13 };      // source (2, 3, 4), unlabeled
    Index UnmarkedTarget = { 6, 8, 10 };        // source (1, 1, 1), target label 3
    Index OutsideTarget = { 30, 30, 30 };       // beyond the source, target label 3
    Index OutsideOther = { 30, 31, 30 };        // beyond the source, label 2
    Index LastSourceVoxel = { 14, 18, 22 };     // source (9, 11, 13), unlabeled
    Index PastLastSourceVoxel = { 15, 18, 22 }; // beyond the source, unlabeled
  };

  static mitk::MultiLabelSegmentation::Pointer CreateSubGeometryDestination(const SubGeometryProbes& probes)
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    auto* groupImage = segmentation->GetGroupImage(0);

    SetPixel(groupImage, 0, probes.MarkedLocked, 1);
    SetPixel(groupImage, 0, probes.MarkedUnlocked, 2);
    SetPixel(groupImage, 0, probes.UnmarkedTarget, 3);
    SetPixel(groupImage, 0, probes.OutsideTarget, 3);
    SetPixel(groupImage, 0, probes.OutsideOther, 2);

    return segmentation;
  }

  static void TransferSubSource(mitk::MultiLabelSegmentation* segmentation, mitk::MultiLabelSegmentation::MergeStyle mergeStyle,
    bool backgroundLocked)
  {
    mitk::TransferLabelContentAtTimeStep(CreateSubSource(), segmentation->GetGroupImage(0),
      segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0)), 0,
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, backgroundLocked,
      { {1, 3} }, mergeStyle, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
  }

  void TestTransfer_SubGeometry_Merge()
  {
    const SubGeometryProbes probes;
    auto segmentation = CreateSubGeometryDestination(probes);
    const auto* groupImage = segmentation->GetGroupImage(0);

    TransferSubSource(segmentation, mitk::MultiLabelSegmentation::MergeStyle::Merge, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked label under a marked voxel was overwritten",
      mitk::Label::PixelType(1), GetPixel(groupImage, 0, probes.MarkedLocked));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlocked label under a marked voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.MarkedUnlocked));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlabeled voxel under a marked voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.MarkedUnlabeled));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Merging cleared the target label under an unmarked voxel",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.UnmarkedTarget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Merging cleared the target label beyond the source",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.OutsideTarget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Another label beyond the source was changed",
      mitk::Label::PixelType(2), GetPixel(groupImage, 0, probes.OutsideOther));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel under the last source voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.LastSourceVoxel));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel past the last source voxel was changed",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, probes.PastLastSourceVoxel));
  }

  void TestTransfer_SubGeometry_Replace()
  {
    const SubGeometryProbes probes;
    auto segmentation = CreateSubGeometryDestination(probes);
    const auto* groupImage = segmentation->GetGroupImage(0);

    TransferSubSource(segmentation, mitk::MultiLabelSegmentation::MergeStyle::Replace, false);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked label under a marked voxel was overwritten",
      mitk::Label::PixelType(1), GetPixel(groupImage, 0, probes.MarkedLocked));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlocked label under a marked voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.MarkedUnlocked));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlabeled voxel under a marked voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.MarkedUnlabeled));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Replacing kept the target label under an unmarked voxel",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, probes.UnmarkedTarget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Replacing kept the target label beyond the source",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, probes.OutsideTarget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Another label beyond the source was changed",
      mitk::Label::PixelType(2), GetPixel(groupImage, 0, probes.OutsideOther));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel under the last source voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.LastSourceVoxel));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel past the last source voxel was changed",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, probes.PastLastSourceVoxel));
  }

  void TestTransfer_SubGeometry_EveryVoxel()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    const auto* groupImage = segmentation->GetGroupImage(0);
    const auto source = CreateSubSource();

    TransferSubSource(segmentation, mitk::MultiLabelSegmentation::MergeStyle::Merge, false);

    mitk::ImageReadAccessor sourceAccessor(source);
    mitk::ImageReadAccessor destinationAccessor(groupImage);
    const auto* sourcePixels = static_cast<const mitk::Label::PixelType*>(sourceAccessor.GetData());
    const auto* destinationPixels = static_cast<const mitk::Label::PixelType*>(destinationAccessor.GetData());

    const Index sourceSize = { source->GetDimension(0), source->GetDimension(1), source->GetDimension(2) };
    const Index destinationSize = { groupImage->GetDimension(0), groupImage->GetDimension(1), groupImage->GetDimension(2) };

    for (std::size_t z = 0; z < destinationSize[2]; ++z)
    {
      for (std::size_t y = 0; y < destinationSize[1]; ++y)
      {
        for (std::size_t x = 0; x < destinationSize[0]; ++x)
        {
          const Index index = { x, y, z };
          bool marked = true;
          std::size_t sourceLinearIndex = 0;

          for (int d = 2; d >= 0; --d)
          {
            if (index[d] < SubSourceOffset[d] || index[d] >= SubSourceOffset[d] + sourceSize[d])
              marked = false;
            else
              sourceLinearIndex = sourceLinearIndex * sourceSize[d] + (index[d] - SubSourceOffset[d]);
          }

          marked = marked && 1 == sourcePixels[sourceLinearIndex];

          const auto expected = mitk::Label::PixelType(marked ? 3 : 0);
          const auto actual = destinationPixels[(z * destinationSize[1] + y) * destinationSize[0] + x];

          if (expected != actual)
          {
            CPPUNIT_FAIL("Wrong value at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) +
              "): " + std::to_string(actual) + " instead of " + std::to_string(expected));
          }
        }
      }
    }
  }

  void TestTransfer_SubGeometry_Replace_LockedBackground()
  {
    const SubGeometryProbes probes;
    auto segmentation = CreateSubGeometryDestination(probes);
    const auto* groupImage = segmentation->GetGroupImage(0);

    TransferSubSource(segmentation, mitk::MultiLabelSegmentation::MergeStyle::Replace, true);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Unlocked label under a marked voxel was not assigned",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.MarkedUnlocked));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Locked background under a marked voxel was assigned",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, probes.MarkedUnlabeled));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Target label under an unmarked voxel was cleared although the background is locked",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.UnmarkedTarget));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Target label beyond the source was cleared although the background is locked",
      mitk::Label::PixelType(3), GetPixel(groupImage, 0, probes.OutsideTarget));
  }

  void TestTransfer_SameImage_NoChaining()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    auto* groupImage = segmentation->GetGroupImage(0);
    const Index first = { 10, 10, 10 };
    const Index second = { 11, 10, 10 };

    SetPixel(groupImage, 0, first, 1);
    SetPixel(groupImage, 0, second, 2);

    mitk::TransferLabelContentAtTimeStep(groupImage, groupImage,
      segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0)), 0,
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false,
      { {1, 2}, {2, 3} }, mitk::MultiLabelSegmentation::MergeStyle::Merge, mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel was mapped twice", mitk::Label::PixelType(2), GetPixel(groupImage, 0, first));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel was not mapped", mitk::Label::PixelType(3), GetPixel(groupImage, 0, second));
  }

  void TestTransfer_RepeatedMapping()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    const auto* groupImage = segmentation->GetGroupImage(0);
    const Index marked = { 20, 20, 20 };

    mitk::TransferLabelContentAtTimeStep(CreateLabelImage({ 40, 40, 40 }, marked, marked, 2), segmentation->GetGroupImage(0),
      segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0)), 0,
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, { {2, 3}, {2, 3} });

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Marked voxel was not assigned", mitk::Label::PixelType(3), GetPixel(groupImage, 0, marked));
  }

  void TestTransfer_ConflictInLaterGroupPair()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    const auto group = segmentation->AddGroup();
    segmentation->AddLabel(mitk::Label::New(4, "Second group 1"), group);
    segmentation->AddLabel(mitk::Label::New(5, "Second group 2"), group);

    auto* groupImage = segmentation->GetGroupImage(0);
    const Index marked = { 20, 20, 20 };
    SetPixel(groupImage, 0, marked, 2);

    // The pair of the first group is valid and comes first, the conflict lies in the pair of the second group.
    CPPUNIT_ASSERT_THROW_MESSAGE("A source label mapped to two targets in one group was accepted",
      mitk::TransferLabelContentAtTimeStep(segmentation, segmentation, 0, { {2, 3}, {1, 4}, {1, 5} }), mitk::Exception);

    CPPUNIT_ASSERT_EQUAL_MESSAGE("The valid pair of groups was transferred although the mapping was rejected",
      mitk::Label::PixelType(2), GetPixel(groupImage, 0, marked));
  }

  void TestTransfer_StaticSource_AtTimeStep()
  {
    auto segmentation = CreateSurfaceTestSegmentation(2);
    const auto* groupImage = segmentation->GetGroupImage(0);
    const Index marked = { 20, 20, 20 };

    mitk::TransferLabelContentAtTimeStep(CreateLabelImage({ 40, 40, 40 }, marked, marked, 1), segmentation->GetGroupImage(0),
      segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0)), 1,
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, { {1, 3} });

    CPPUNIT_ASSERT_EQUAL_MESSAGE("Marked voxel was not assigned at the time step",
      mitk::Label::PixelType(3), GetPixel(groupImage, 1, marked));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Voxel of another time step was assigned",
      mitk::Label::PixelType(0), GetPixel(groupImage, 0, marked));
  }

  void TestTransfer_InvalidInput()
  {
    auto segmentation = CreateSurfaceTestSegmentation(1);
    auto* groupImage = segmentation->GetGroupImage(0);
    const auto labels = segmentation->GetConstLabelsByValue(segmentation->GetLabelValuesByGroup(0));
    const auto source = CreateLabelImage({ 40, 40, 40 }, { 20, 20, 20 }, { 20, 20, 20 }, 1);

    CPPUNIT_ASSERT_THROW_MESSAGE("A source label mapped to two targets was accepted",
      mitk::TransferLabelContentAtTimeStep(source, groupImage, labels, 0, 0, 0, false, { {1, 2}, {1, 3} }), mitk::Exception);

    auto misaligned = CreateSubSource();
    auto origin = misaligned->GetGeometry()->GetOrigin();
    origin[0] += 0.5;
    misaligned->GetGeometry()->SetOrigin(origin);

    CPPUNIT_ASSERT_THROW_MESSAGE("A source off the grid of the destination was accepted",
      mitk::TransferLabelContentAtTimeStep(misaligned, groupImage, labels, 0, 0, 0, false, { {1, 3} }), mitk::Exception);

    auto charDestination = mitk::Image::New();
    unsigned int dimensions[3] = { 40, 40, 40 };
    charDestination->Initialize(mitk::MakeScalarPixelType<char>(), 3, dimensions);

    CPPUNIT_ASSERT_THROW_MESSAGE("A destination without the pixel type of labels was accepted",
      mitk::TransferLabelContentAtTimeStep(source, charDestination, labels, 0, 0, 0, false, { {1, 3} }), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkTransferLabel)
