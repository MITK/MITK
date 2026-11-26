/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitknnInteractiveBoxInteractor.h>
#include <mitknnInteractiveLassoInteractor.h>
#include <mitknnInteractivePointInteractor.h>
#include <mitknnInteractiveScribbleInteractor.h>

#include "mitknnInteractiveBoundingBoxHelpers.h"

#include <mitkContourModel.h>
#include <mitkImageReadAccessor.h>
#include <mitkPlanarFigure.h>
#include <mitkPointSet.h>
#include <mitkStandaloneDataStorage.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>
#include <mitkToolManager.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace
{
  mitk::Point3D MakePoint(double x, double y, double z)
  {
    mitk::Point3D point;
    point[0] = x;
    point[1] = y;
    point[2] = z;
    return point;
  }

  /** The value of a mask at an index of the reference image. */
  unsigned char GetValue(const mitk::Image* mask, const mitk::nnInteractive::InteractionBoundingBox& box, int x, int y, int z)
  {
    // The bounding box is in (Z, Y, X) order.
    const auto maskX = static_cast<unsigned int>(x - box[2][0]);
    const auto maskY = static_cast<unsigned int>(y - box[1][0]);
    const auto maskZ = static_cast<unsigned int>(z - box[0][0]);

    mitk::ImageReadAccessor accessor(mask);
    const auto* data = static_cast<const unsigned char*>(accessor.GetData());
    return data[maskX + maskY * mask->GetDimension(0) + maskZ * mask->GetDimension(0) * mask->GetDimension(1)];
  }
}

class mitknnInteractiveInteractorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitknnInteractiveInteractorTestSuite);
  MITK_TEST(PointsMustLieInOneSlice);
  MITK_TEST(PointsInNeighboringSlicesAreRefused);
  MITK_TEST(PlaneLiesAtTheCenterOfTheSliceOfThePoints);
  MITK_TEST(PointsHalfwayLieInTheSliceTheyRoundTo);
  MITK_TEST(PointIsDrawnAtTheCenterOfItsVoxel);
  MITK_TEST(LassoOffTheSliceCenterIsDrawnInItsPlane);
  MITK_TEST(AxialBoxKeepsItsCorners);
  MITK_TEST(SagittalBoxKeepsItsCorners);
  MITK_TEST(ScribbleIsPaintedWithTheBrush);
  MITK_TEST(LassoIsFilled);
  MITK_TEST(AddedPromptIsRemovable);
  MITK_TEST(AddingWithoutPlaneOrReferenceImageThrows);
  CPPUNIT_TEST_SUITE_END();

  mitk::DataStorage::Pointer m_DataStorage;
  mitk::ToolManager::Pointer m_ToolManager;
  mitk::Image::Pointer m_Image;

  /** Checks that a box shows its corners in the slice along the axis. */
  void CheckBox(const mitk::Point3D& corner, const mitk::Point3D& oppositeCorner, int sliceAxis, double sliceSpacing)
  {
    mitk::nnInteractive::BoxInteractor interactor;
    interactor.SetToolManager(m_ToolManager);

    const auto plane = mitk::nnInteractive::CreateSlicePlane({ corner, oppositeCorner }, m_Image->GetGeometry());
    const auto* box = interactor.AddBox(plane, corner, oppositeCorner, mitk::nnInteractive::PromptType::Positive);

    CPPUNIT_ASSERT(interactor.HasInteractions());

    // nnInteractive gets the box spanned by these two control points.
    const auto controlPoint = box->GetWorldControlPoint(0);
    const auto oppositeControlPoint = box->GetWorldControlPoint(1);

    for (int axis = 0; axis < 3; ++axis)
    {
      CPPUNIT_ASSERT_DOUBLES_EQUAL(std::min(corner[axis], oppositeCorner[axis]), std::min(controlPoint[axis], oppositeControlPoint[axis]), 1e-6);
      CPPUNIT_ASSERT_DOUBLES_EQUAL(std::max(corner[axis], oppositeCorner[axis]), std::max(controlPoint[axis], oppositeControlPoint[axis]), 1e-6);
    }

    // The planar figure mapper shows the box in slices closer than half of
    // this thickness, so it must be the spacing along the slice axis.
    const auto* boxPlane = box->GetPlaneGeometry();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(sliceSpacing, boxPlane->GetExtentInMM(2), 1e-6);

    auto normal = boxPlane->GetNormal();
    normal.Normalize();
    CPPUNIT_ASSERT_DOUBLES_EQUAL(1.0, std::abs(normal[sliceAxis]), 1e-6);
  }

public:
  void setUp() override
  {
    // Different spacings along the axes reveal axes mixed up in the plane.
    const unsigned int dimensions[3] = { 20, 30, 40 };

    m_Image = mitk::Image::New();
    m_Image->Initialize(mitk::MakeScalarPixelType<short>(), 3, dimensions);
    m_Image->AllocateZeroedVolume();

    mitk::Vector3D spacing;
    spacing[0] = 0.5;
    spacing[1] = 1.0;
    spacing[2] = 2.0;
    m_Image->SetSpacing(spacing);

    auto node = mitk::DataNode::New();
    node->SetData(m_Image);

    m_DataStorage = mitk::StandaloneDataStorage::New();
    m_DataStorage->Add(node);

    m_ToolManager = mitk::ToolManager::New(m_DataStorage);
    m_ToolManager->SetReferenceData(node);
  }

  void tearDown() override
  {
    m_ToolManager = nullptr;
    m_DataStorage = nullptr;
    m_Image = nullptr;
  }

  void PointsMustLieInOneSlice()
  {
    CPPUNIT_ASSERT_THROW(mitk::nnInteractive::CreateSlicePlane(
      { MakePoint(1.0, 1.0, 10.0), MakePoint(5.0, 20.0, 60.0) }, m_Image->GetGeometry()), mitk::Exception);
  }

  void PointsInNeighboringSlicesAreRefused()
  {
    // Slices 10 and 11 along z, and different voxels along x and y.
    CPPUNIT_ASSERT_THROW(mitk::nnInteractive::CreateSlicePlane(
      { MakePoint(2.0, 5.0, 20.0), MakePoint(6.5, 12.0, 22.0) }, m_Image->GetGeometry()), mitk::Exception);
  }

  void PlaneLiesAtTheCenterOfTheSliceOfThePoints()
  {
    // Within the voxels of slice 10 along z, whose center is at z = 20.
    const auto plane = mitk::nnInteractive::CreateSlicePlane(
      { MakePoint(2.0, 5.0, 20.7), MakePoint(6.5, 12.0, 20.9) }, m_Image->GetGeometry());

    CPPUNIT_ASSERT(plane.IsNotNull());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, plane->SignedDistanceFromPlane(MakePoint(4.0, 8.0, 20.0)), 1e-6);
  }

  void PointsHalfwayLieInTheSliceTheyRoundTo()
  {
    // Halfway between slices 10 and 11, which nnInteractive rounds to 11.
    const auto plane = mitk::nnInteractive::CreateSlicePlane(
      { MakePoint(2.0, 5.0, 21.0), MakePoint(6.5, 12.0, 21.0) }, m_Image->GetGeometry());

    CPPUNIT_ASSERT(plane.IsNotNull());
    CPPUNIT_ASSERT_DOUBLES_EQUAL(0.0, plane->SignedDistanceFromPlane(MakePoint(4.0, 8.0, 22.0)), 1e-6);
  }

  void PointIsDrawnAtTheCenterOfItsVoxel()
  {
    mitk::nnInteractive::PointInteractor interactor;
    interactor.SetToolManager(m_ToolManager);

    // In the voxel (4, 5, 10), but not at its center.
    interactor.AddPoint(MakePoint(2.2, 5.3, 20.7), mitk::nnInteractive::PromptType::Positive);

    const auto point = interactor.GetLastPoint();
    CPPUNIT_ASSERT(point.has_value());

    CPPUNIT_ASSERT_DOUBLES_EQUAL(2.0, (*point)[0], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(5.0, (*point)[1], 1e-6);
    CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, (*point)[2], 1e-6);
  }

  void LassoOffTheSliceCenterIsDrawnInItsPlane()
  {
    mitk::nnInteractive::LassoInteractor interactor;
    interactor.SetToolManager(m_ToolManager);

    // The rectangle of LassoIsFilled(), 0.9 mm above the center of slice 10.
    const std::vector<mitk::Point3D> points = {
      MakePoint(2.0, 5.0, 20.9), MakePoint(6.0, 5.0, 20.9), MakePoint(6.0, 15.0, 20.9), MakePoint(2.0, 15.0, 20.9) };

    const auto plane = mitk::nnInteractive::CreateSlicePlane(points, m_Image->GetGeometry());
    interactor.AddLasso(plane, points, mitk::nnInteractive::PromptType::Positive);

    const auto mask = interactor.GetLastLassoMask();
    const auto* box = interactor.GetLastLassoBoundingBox();
    CPPUNIT_ASSERT(mask.IsNotNull() && box != nullptr);
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(1), GetValue(mask, *box, 8, 10, 10));

    // The render windows show the contour only close to their plane.
    const mitk::ContourModel* contour = nullptr;
    const auto nodes = m_DataStorage->GetAll();

    for (const auto& node : *nodes)
    {
      if (const auto* candidate = dynamic_cast<const mitk::ContourModel*>(node->GetData()); candidate != nullptr)
        contour = candidate;
    }

    CPPUNIT_ASSERT(contour != nullptr);
    CPPUNIT_ASSERT_EQUAL(4, contour->GetNumberOfVertices());

    for (int index = 0; index < contour->GetNumberOfVertices(); ++index)
      CPPUNIT_ASSERT_DOUBLES_EQUAL(20.0, contour->GetVertexAt(index)->Coordinates[2], 1e-6);
  }

  void AxialBoxKeepsItsCorners()
  {
    // In slice 10 along z.
    this->CheckBox(MakePoint(2.0, 5.0, 20.0), MakePoint(6.5, 12.0, 20.0), 2, 2.0);
  }

  void SagittalBoxKeepsItsCorners()
  {
    // In slice 7 along x.
    this->CheckBox(MakePoint(3.5, 4.0, 10.0), MakePoint(3.5, 20.0, 50.0), 0, 0.5);
  }

  void ScribbleIsPaintedWithTheBrush()
  {
    mitk::nnInteractive::ScribbleInteractor interactor;
    interactor.SetToolManager(m_ToolManager);

    // Along x through index y = 5 in slice 10 along z.
    const std::vector<mitk::Point3D> points = { MakePoint(2.0, 5.0, 20.0), MakePoint(6.5, 5.0, 20.0) };
    const auto plane = mitk::nnInteractive::CreateSlicePlane(points, m_Image->GetGeometry());
    interactor.AddScribble(plane, points, mitk::nnInteractive::PromptType::Positive);

    const auto mask = interactor.GetLastScribbleMask();
    const auto* box = interactor.GetLastScribbleBoundingBox();
    CPPUNIT_ASSERT(mask.IsNotNull() && box != nullptr);

    // One voxel thick along z.
    CPPUNIT_ASSERT((*box)[0] == (std::array<int, 2>{ 10, 11 }));

    // Painted between the points, but not three rows away from them.
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(1), GetValue(mask, *box, 8, 5, 10));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), GetValue(mask, *box, 8, 8, 10));
  }

  void LassoIsFilled()
  {
    mitk::nnInteractive::LassoInteractor interactor;
    interactor.SetToolManager(m_ToolManager);

    // A rectangle from index (4, 5) to (12, 15) in slice 10 along z.
    const std::vector<mitk::Point3D> points = {
      MakePoint(2.0, 5.0, 20.0), MakePoint(6.0, 5.0, 20.0), MakePoint(6.0, 15.0, 20.0), MakePoint(2.0, 15.0, 20.0) };

    const auto plane = mitk::nnInteractive::CreateSlicePlane(points, m_Image->GetGeometry());
    interactor.AddLasso(plane, points, mitk::nnInteractive::PromptType::Positive);

    const auto mask = interactor.GetLastLassoMask();
    const auto* box = interactor.GetLastLassoBoundingBox();
    CPPUNIT_ASSERT(mask.IsNotNull() && box != nullptr);

    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(1), GetValue(mask, *box, 8, 10, 10));
    CPPUNIT_ASSERT_EQUAL(static_cast<unsigned char>(0), GetValue(mask, *box, 14, 10, 10));
  }

  void AddedPromptIsRemovable()
  {
    mitk::nnInteractive::ScribbleInteractor interactor;
    interactor.SetToolManager(m_ToolManager);

    const auto nodeCount = m_DataStorage->GetAll()->Size();

    const std::vector<mitk::Point3D> points = { MakePoint(2.0, 5.0, 20.0), MakePoint(6.5, 5.0, 20.0) };
    const auto plane = mitk::nnInteractive::CreateSlicePlane(points, m_Image->GetGeometry());
    interactor.AddScribble(plane, points, mitk::nnInteractive::PromptType::Negative);
    CPPUNIT_ASSERT_EQUAL(nodeCount + 1, m_DataStorage->GetAll()->Size());

    interactor.RemoveLastInteraction(mitk::nnInteractive::PromptType::Negative);
    CPPUNIT_ASSERT(!interactor.HasInteractions());
    CPPUNIT_ASSERT_EQUAL(nodeCount, m_DataStorage->GetAll()->Size());
  }

  void AddingWithoutPlaneOrReferenceImageThrows()
  {
    const std::vector<mitk::Point3D> points = { MakePoint(2.0, 5.0, 20.0), MakePoint(6.5, 5.0, 20.0) };
    const auto promptType = mitk::nnInteractive::PromptType::Positive;

    mitk::nnInteractive::BoxInteractor box;
    box.SetToolManager(m_ToolManager);
    CPPUNIT_ASSERT_THROW(box.AddBox(nullptr, points[0], points[1], promptType), mitk::Exception);

    mitk::nnInteractive::ScribbleInteractor scribble;
    scribble.SetToolManager(m_ToolManager);
    CPPUNIT_ASSERT_THROW(scribble.AddScribble(nullptr, points, promptType), mitk::Exception);

    mitk::nnInteractive::LassoInteractor lasso;
    lasso.SetToolManager(m_ToolManager);
    CPPUNIT_ASSERT_THROW(lasso.AddLasso(nullptr, points, promptType), mitk::Exception);

    mitk::nnInteractive::PointInteractor point;
    CPPUNIT_ASSERT_THROW(point.AddPoint(points[0], promptType), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitknnInteractiveInteractor)
