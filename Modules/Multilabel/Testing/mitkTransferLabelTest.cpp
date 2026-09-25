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
};

MITK_TEST_SUITE_REGISTRATION(mitkTransferLabel)
