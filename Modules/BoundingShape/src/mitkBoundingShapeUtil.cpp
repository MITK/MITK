/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkBoundingShapeUtil.h"

#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkGeometry3D.h>
#include <mitkNumericConstants.h>
#include <mitkPlaneGeometry.h>

#include <vtkAppendPolyData.h>
#include <vtkCamera.h>
#include <vtkCubeSource.h>
#include <vtkMath.h>
#include <vtkMatrix4x4.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkTransformFilter.h>

#include <cmath>
#include <span>
#include <vector>

namespace
{
  /**
   * \brief Return the four corner-point indices of the box face that the given face handle
   *        sits on (the face it moves when dragged), ordered along the face perimeter so
   *        that consecutive indices (and the last-to-first pair) form the four face edges.
   */
  std::array<int, 4> GetHandleFaceCornerIndices(int handleIndex)
  {
    switch (handleIndex)
    {
      case 0: return {4, 5, 7, 6}; // x-max face
      case 1: return {0, 1, 3, 2}; // x-min face
      case 2: return {0, 4, 6, 2}; // z-min face
      case 3: return {1, 5, 7, 3}; // z-max face
      case 4: return {2, 3, 7, 6}; // y-max face
      case 5: return {0, 1, 5, 4}; // y-min face
      default: return {0, 0, 0, 0};
    }
  }

  // corner-point index pairs of the 12 box edges, ordered by the edge id table documented
  // at mitk::Handle
  constexpr std::array<std::array<int, 2>, 12> BoxEdgeCornerIndices = {{
    {0, 4}, {1, 5}, {2, 6}, {3, 7}, // running along x
    {0, 2}, {1, 3}, {4, 6}, {5, 7}, // running along y
    {0, 1}, {2, 3}, {4, 5}, {6, 7}  // running along z
  }};

  /**
   * \brief Derive the bounds a handle moves from the corner indices of the box geometry it
   *        sits on (face corners, edge corners, or a single corner): an axis whose bit is
   *        set in all (none) of the corners lies on the maximum (minimum) bound and is
   *        moved; an axis with mixed bits is spanned by the handle's geometry and is not.
   */
  mitk::AxisMovedBounds MovedBoundsFromCornerIndices(std::span<const int> cornerIndices)
  {
    constexpr std::array<int, 3> axisBits = {4, 2, 1}; // corner index = 4x + 2y + z

    mitk::AxisMovedBounds movedBounds;
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      std::size_t numSet = 0;
      for (int cornerIndex : cornerIndices)
      {
        if (cornerIndex & axisBits[axis])
          ++numSet;
      }

      movedBounds[axis] = numSet == cornerIndices.size() ? mitk::MovedBound::Maximum
                        : numSet == 0                    ? mitk::MovedBound::Minimum
                                                         : mitk::MovedBound::None;
    }

    return movedBounds;
  }

  /**
   * \brief Signed distances of the 8 box corners to the slice plane in mm, with values
   *        within \p tolerance snapped to zero.
   *
   * Without the snapping, a slice lying exactly on a box face would fail every strict
   * crossing test and hide all handles in that render window. Slice positions and box
   * faces are both derived from image geometries, so such a coincidence is exact only up
   * to the round-off accumulated in the transforms, which is what the tolerance absorbs.
   */
  std::array<double, 8> GetCornerPlaneDistances(const std::array<mitk::Point3D, 8> &cornerPoints,
                                                const mitk::Point3D &planeOrigin,
                                                const mitk::Vector3D &unitPlaneNormal,
                                                double tolerance)
  {
    std::array<double, 8> distances;
    for (std::size_t i = 0; i < 8; ++i)
    {
      const double distance = unitPlaneNormal * (cornerPoints[i] - planeOrigin);
      distances[i] = std::abs(distance) < tolerance ? 0.0 : distance;
    }
    return distances;
  }

  /**
   * \brief Intersect a box face (a planar, convex quad given by perimeter-ordered corner
   *        indices) with the slice plane and return the center of the intersection segment.
   *
   * \return true if the plane crosses the face, false otherwise. A face lying entirely in
   *         the plane yields false: its cross-section is the whole face, so there is no
   *         segment to place a handle on.
   */
  bool GetFacePlaneIntersectionCenter(const std::array<mitk::Point3D, 8> &cornerPoints,
                                      const std::array<int, 4> &faceCornerIndices,
                                      const std::array<double, 8> &cornerDistances,
                                      mitk::Point3D &center)
  {
    int numOnPlane = 0;
    for (int cornerIndex : faceCornerIndices)
    {
      if (cornerDistances[cornerIndex] == 0.0)
        ++numOnPlane;
    }

    if (numOnPlane == 4)
      return false;

    std::vector<mitk::Point3D> hits;
    hits.reserve(2);

    // corners on the plane are intersection points themselves ...
    for (int cornerIndex : faceCornerIndices)
    {
      if (cornerDistances[cornerIndex] == 0.0)
        hits.push_back(cornerPoints[cornerIndex]);
    }

    // ... and each face edge whose endpoints lie strictly on opposite sides adds one
    for (int i = 0; i < 4; ++i)
    {
      const double distA = cornerDistances[faceCornerIndices[i]];
      const double distB = cornerDistances[faceCornerIndices[(i + 1) % 4]];

      if (distA * distB < 0.0)
      {
        const double t = distA / (distA - distB);
        const mitk::Point3D &a = cornerPoints[faceCornerIndices[i]];
        const mitk::Point3D &b = cornerPoints[faceCornerIndices[(i + 1) % 4]];
        hits.push_back(a + (b - a) * t);
      }
    }

    if (hits.size() < 2)
      return false;

    center.Fill(0.0);
    for (const auto &hit : hits)
    {
      center[0] += hit[0];
      center[1] += hit[1];
      center[2] += hit[2];
    }
    const double count = static_cast<double>(hits.size());
    center[0] /= count;
    center[1] /= count;
    center[2] /= count;

    return true;
  }

  /**
   * \brief Create the marker polydata for one handle: a cube of edge length \p size,
   *        oriented with the direction cosines of \p geometry and centered at \p center.
   */
  vtkSmartPointer<vtkPolyData> CreateHandlePolyData(const mitk::BaseGeometry *geometry,
                                                    const mitk::Point3D &center,
                                                    double size)
  {
    auto cube = vtkSmartPointer<vtkCubeSource>::New();
    cube->SetCenter(0.0, 0.0, 0.0);
    cube->SetXLength(size);
    cube->SetYLength(size);
    cube->SetZLength(size);

    const mitk::Vector3D spacing = geometry->GetSpacing();
    vtkMatrix4x4 *imageTransform = geometry->GetVtkTransform()->GetMatrix();

    // orient the marker cube with the box and move it onto the handle position; the direction
    // cosines are the transform columns with the spacing scaled out
    auto handleMatrix = vtkSmartPointer<vtkMatrix4x4>::New();
    handleMatrix->Identity();
    for (int c = 0; c < 3; ++c)
      for (int r = 0; r < 3; ++r)
        handleMatrix->SetElement(r, c, imageTransform->GetElement(r, c) / spacing[c]);
    handleMatrix->SetElement(0, 3, center[0]);
    handleMatrix->SetElement(1, 3, center[1]);
    handleMatrix->SetElement(2, 3, center[2]);

    auto handleTransform = vtkSmartPointer<vtkTransform>::New();
    handleTransform->SetMatrix(handleMatrix);

    auto handleTransformFilter = vtkSmartPointer<vtkTransformFilter>::New();
    handleTransformFilter->SetInputConnection(cube->GetOutputPort());
    handleTransformFilter->SetTransform(handleTransform);
    handleTransformFilter->Update();

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(handleTransformFilter->GetPolyDataOutput());

    return polyData;
  }
}

mitk::Handle::Handle() : m_Index(-1), m_MovedBounds{}
{
  m_Position.Fill(0.0);
}

mitk::Handle::Handle(const Point3D &position, int index, const AxisMovedBounds &movedBounds)
  : m_Position(position), m_Index(index), m_MovedBounds(movedBounds)
{
}

int mitk::Handle::GetIndex() const
{
  return m_Index;
}

const mitk::AxisMovedBounds &mitk::Handle::GetMovedBounds() const
{
  return m_MovedBounds;
}

mitk::Point3D mitk::Handle::GetPosition() const
{
  return m_Position;
}

mitk::Point3D mitk::CalcAvgPoint(mitk::Point3D a, mitk::Point3D b)
{
  mitk::Point3D c;
  c[0] = (a[0] + b[0]) / 2.0;
  c[1] = (a[1] + b[1]) / 2.0;
  c[2] = (a[2] + b[2]) / 2.0;
  return c;
}

std::array<mitk::Point3D, 8> mitk::GetCornerPoints(mitk::BaseGeometry::Pointer geometry, bool visualizationOffset)
{
  if (geometry == nullptr)
    mitkThrow() << "Geometry is not valid.";

  mitk::BoundingBox::ConstPointer boundingBox = geometry->GetBoundingBox();
  mitk::Point3D BBmin = boundingBox->GetMinimum();
  mitk::Point3D BBmax = boundingBox->GetMaximum();

  // use 0.5 offset because the vtkCubeSource is not center pixel based (only for visualization purpose)
  if (visualizationOffset)
  {
    BBmin -= mitk::Vector(0.5);
    BBmax -= mitk::Vector(0.5);
  }
  mitk::Point3D p0;
  p0[0] = BBmin[0];
  p0[1] = BBmin[1];
  p0[2] = BBmin[2]; // bottom - left - back corner
  mitk::Point3D p1;
  p1[0] = BBmin[0];
  p1[1] = BBmin[1];
  p1[2] = BBmax[2]; // top - left - back corner
  mitk::Point3D p2;
  p2[0] = BBmin[0];
  p2[1] = BBmax[1];
  p2[2] = BBmin[2]; // bottom - left - front corner
  mitk::Point3D p3;
  p3[0] = BBmin[0];
  p3[1] = BBmax[1];
  p3[2] = BBmax[2]; // top - left - front corner
  mitk::Point3D p4;
  p4[0] = BBmax[0];
  p4[1] = BBmin[1];
  p4[2] = BBmin[2]; // bottom - right - back corner
  mitk::Point3D p5;
  p5[0] = BBmax[0];
  p5[1] = BBmin[1];
  p5[2] = BBmax[2]; // top - right - back corner
  mitk::Point3D p6;
  p6[0] = BBmax[0];
  p6[1] = BBmax[1];
  p6[2] = BBmin[2]; // bottom - right - front corner
  mitk::Point3D p7;
  p7[0] = BBmax[0];
  p7[1] = BBmax[1];
  p7[2] = BBmax[2]; // top - right - front corner

  const mitk::AffineTransform3D *indexToWorld = geometry->GetIndexToWorldTransform();

  return {indexToWorld->TransformPoint(p0),
          indexToWorld->TransformPoint(p1),
          indexToWorld->TransformPoint(p2),
          indexToWorld->TransformPoint(p3),
          indexToWorld->TransformPoint(p4),
          indexToWorld->TransformPoint(p5),
          indexToWorld->TransformPoint(p6),
          indexToWorld->TransformPoint(p7)};
}

std::vector<mitk::Handle> mitk::ComputeHandles(const std::array<Point3D, 8> &cornerPoints,
                                               const PlaneGeometry *planeGeometry)
{
  std::vector<Handle> handles;

  if (planeGeometry == nullptr)
  {
    // 3D render window: face handles at the face centers. No handles on the box corners:
    // a 2D mouse movement cannot express which of the three directions a corner should
    // follow, making the interaction unpredictable.
    handles.reserve(6);

    for (int faceIndex = 0; faceIndex < 6; ++faceIndex)
    {
      const std::array<int, 4> faceCornerIndices = GetHandleFaceCornerIndices(faceIndex);
      // the face is a parallelogram, so the midpoint of one diagonal is the face center
      const Point3D center = CalcAvgPoint(cornerPoints[faceCornerIndices[0]], cornerPoints[faceCornerIndices[2]]);
      handles.emplace_back(center, faceIndex, MovedBoundsFromCornerIndices(faceCornerIndices));
    }

    return handles;
  }

  // 2D render window: handles ride the rendered cross-section outline, also for oblique
  // boxes - face handles on its sides, edge handles on its corners. Both use the same
  // snapped corner distances, so their visibility is mutually consistent.
  Vector3D planeNormal = planeGeometry->GetNormal();
  const double sliceThickness = planeNormal.GetNorm(); // GetNormal() is scaled by it, not unit length

  if (sliceThickness < eps)
    return handles; // degenerate slice plane, nothing to place handles on

  planeNormal /= sliceThickness;

  // A tolerance well below one slice and far above the round-off of the transforms that
  // produced the corners and the plane origin.
  constexpr double coincidenceTolerance = 1e-3;

  const std::array<double, 8> cornerDistances = GetCornerPlaneDistances(
    cornerPoints, planeGeometry->GetOrigin(), planeNormal, coincidenceTolerance * sliceThickness);

  for (int faceIndex = 0; faceIndex < 6; ++faceIndex)
  {
    const std::array<int, 4> faceCornerIndices = GetHandleFaceCornerIndices(faceIndex);
    Point3D position;
    if (GetFacePlaneIntersectionCenter(cornerPoints, faceCornerIndices, cornerDistances, position))
      handles.emplace_back(position, faceIndex, MovedBoundsFromCornerIndices(faceCornerIndices));
  }

  // An edge that reaches the plane with one of its endpoints puts its handle on that box
  // corner, where it would coincide with the handles of the two other edges meeting there.
  // Only the most transverse of the three is kept.
  constexpr int noEdge = -1;
  std::array<int, 8> cornerEdge;
  cornerEdge.fill(noEdge);
  std::array<double, 8> cornerEdgeTransversality;
  cornerEdgeTransversality.fill(0.0);

  for (int edgeIndex = 0; edgeIndex < 12; ++edgeIndex)
  {
    const auto &edgeCornerIndices = BoxEdgeCornerIndices[edgeIndex];
    const double distA = cornerDistances[edgeCornerIndices[0]];
    const double distB = cornerDistances[edgeCornerIndices[1]];

    // an edge lying entirely in the plane gets no handle: its in-plane resize direction is
    // already covered by the face handles adjacent to it
    if (distA == 0.0 && distB == 0.0)
      continue;

    const Point3D &a = cornerPoints[edgeCornerIndices[0]];
    const Point3D &b = cornerPoints[edgeCornerIndices[1]];

    if (distA * distB < 0.0)
    {
      // the edge spans the plane, so its handle sits between the corners and is unique
      handles.emplace_back(
        a + (b - a) * (distA / (distA - distB)), 6 + edgeIndex, MovedBoundsFromCornerIndices(edgeCornerIndices));
      continue;
    }

    if (distA != 0.0 && distB != 0.0)
      continue; // the edge does not reach the plane

    // The endpoints differ in distance (one is snapped to zero, the other is not), so the
    // edge has a non-zero length and the ratio is the cosine of its angle to the normal.
    const int touchedCorner = distA == 0.0 ? edgeCornerIndices[0] : edgeCornerIndices[1];
    const double transversality = std::abs(distA - distB) / (b - a).GetNorm();

    if (transversality > cornerEdgeTransversality[touchedCorner])
    {
      cornerEdge[touchedCorner] = edgeIndex;
      cornerEdgeTransversality[touchedCorner] = transversality;
    }
  }

  for (int cornerIndex = 0; cornerIndex < 8; ++cornerIndex)
  {
    const int edgeIndex = cornerEdge[cornerIndex];

    if (edgeIndex != noEdge)
      handles.emplace_back(
        cornerPoints[cornerIndex], 6 + edgeIndex, MovedBoundsFromCornerIndices(BoxEdgeCornerIndices[edgeIndex]));
  }

  return handles;
}

double mitk::GetHandleSize(const BaseRenderer *renderer, const DataNode *node)
{
  double sizeFactor = DefaultHandleSizeFactor;
  node->GetDoubleProperty(BoundingShapeHandleSizeFactorPropertyName, sizeFactor, renderer);

  if (renderer->GetMapperID() == BaseRenderer::Standard2D)
  {
    const Point2D displaySize = renderer->GetDisplaySizeInMM();
    return (displaySize[0] + displaySize[1]) / 2.0 * sizeFactor;
  }

  // half the height of the view frustum at the focal plane
  vtkCamera *camera = renderer->GetVtkRenderer()->GetActiveCamera();
  return camera->GetDistance() * std::tan(vtkMath::RadiansFromDegrees(camera->GetViewAngle()) / 2.0) * sizeFactor;
}

mitk::HandleMarkers mitk::CreateHandleMarkers(const DataNode *node,
                                              const BaseRenderer *renderer,
                                              const BaseGeometry *geometry,
                                              const std::array<Point3D, 8> &cornerPoints,
                                              const PlaneGeometry *planeGeometry)
{
  int activeHandleId = -1;

  if (!node->GetIntProperty(BoundingShapeActiveHandleIdPropertyName, activeHandleId, renderer))
    return {}; // no interactor attached, so no handles to offer

  const double handleSize = GetHandleSize(renderer, node);

  HandleMarkers markers;
  auto idleHandleAppender = vtkSmartPointer<vtkAppendPolyData>::New();
  bool hasIdleHandles = false;

  for (const auto &handle : ComputeHandles(cornerPoints, planeGeometry))
  {
    auto handlePolyData = CreateHandlePolyData(geometry, handle.GetPosition(), handleSize);

    if (activeHandleId == handle.GetIndex())
    {
      markers.selectedHandle = handlePolyData;
    }
    else
    {
      idleHandleAppender->AddInputData(handlePolyData);
      hasIdleHandles = true;
    }
  }

  // vtkAppendPolyData requires at least one input, so it is only run once there is one
  if (hasIdleHandles)
  {
    idleHandleAppender->Update();
    markers.idleHandles = idleHandleAppender->GetOutput();
  }

  return markers;
}

void mitk::GetBoundingShapeColor(const DataNode *node, const BaseRenderer *renderer, float color[3])
{
  color[0] = 1.0f;
  color[1] = 0.0f;
  color[2] = 0.0f;

  bool selected = false;
  node->GetBoolProperty(BoundingShapeSelectedPropertyName, selected, renderer);

  if (selected && node->GetColor(color, renderer, BoundingShapeSelectedColorPropertyName))
    return;

  node->GetColor(color, renderer, "color");
}
