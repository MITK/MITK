/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkPlaneGeometryDataMapper2D.h"

// mitk includes
#include "mitkVtkPropRenderer.h"
#include <mitkAbstractTransformGeometry.h>
#include <mitkDataNode.h>
#include <mitkLine.h>
#include <mitkPlaneGeometry.h>
#include <mitkPlaneOrientationProperty.h>
#include <mitkPointSet.h>
#include <mitkProperties.h>
#include <mitkResliceMethodProperty.h>
#include <mitkSlicedGeometry3D.h>

// vtk includes
#include <mitkIPropertyAliases.h>
#include <vtkActor2D.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkLine.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper2D.h>
#include <vtkProperty2D.h>
#include <vtkTriangle.h>

///
#include <algorithm>
#include <array>
#include <cassert>
#include <set>

namespace
{
  /// Some simple interval arithmetic
  template <typename T>
  class SimpleInterval
  {
  public:
    SimpleInterval(T start = T(), T end = T())
      : m_LowerBoundary(std::min(start, end)), m_UpperBoundary(std::max(start, end))
    {
    }

    T GetLowerBoundary() const { return m_LowerBoundary; }
    T GetUpperBoundary() const { return m_UpperBoundary; }
    bool empty() const { return m_LowerBoundary == m_UpperBoundary; }
    bool operator<(const SimpleInterval &otherInterval) const
    {
      return this->m_UpperBoundary < otherInterval.GetLowerBoundary();
    }

  private:
    T m_LowerBoundary;
    T m_UpperBoundary;
  };

  template <typename T>
  class IntervalSet
  {
  public:
    typedef SimpleInterval<T> IntervalType;

    IntervalSet(IntervalType startingInterval) { m_IntervalsContainer.insert(std::move(startingInterval)); }
    void operator-=(const IntervalType &interval)
    {
      // equal_range will find all the intervals in the interval set which intersect with the input interval
      //   due to the nature of operator< of SimpleInterval
      auto range = m_IntervalsContainer.equal_range(interval);

      for (auto iter = range.first; iter != range.second;)
      {
        auto subtractionResult = SubtractIntervals(*iter, interval);

        // Remove the old interval from the set
        iter = m_IntervalsContainer.erase(iter);
        for (auto &&interval : subtractionResult)
        {
          if (!interval.empty())
          {
            // Add the new interval to the set
            // emplace_hint adds the element at the closest valid place before the hint iterator,
            //   which is exactly where the new interval should be
            iter = m_IntervalsContainer.insert(iter, std::move(interval));
            ++iter;
          }
        }
      }
    }

    IntervalSet operator-(const IntervalType &interval)
    {
      IntervalSet result = *this;
      result -= interval;
      return result;
    }

    typedef std::set<IntervalType> IntervalsContainer;

    const IntervalsContainer &getIntervals() const { return m_IntervalsContainer; }
  private:
    IntervalsContainer m_IntervalsContainer;

    std::array<IntervalType, 2> SubtractIntervals(const IntervalType &firstInterval, const IntervalType &secondInterval)
    {
      assert(secondInterval.GetUpperBoundary() >= firstInterval.GetLowerBoundary() &&
             firstInterval.GetUpperBoundary() >=
               secondInterval.GetLowerBoundary()); // Non-intersecting intervals should never reach here

      if (secondInterval.GetLowerBoundary() < firstInterval.GetLowerBoundary())
      {
        if (firstInterval.GetUpperBoundary() < secondInterval.GetUpperBoundary())
        {
          std::array<IntervalType, 2> result = {{IntervalType(), IntervalType()}};
          return result; // firstInterval completely enclosed
        }
        std::array<IntervalType, 2> result = {
          {IntervalType(firstInterval.GetUpperBoundary(), secondInterval.GetUpperBoundary()), IntervalType()}};
        return result; // secondInterval removes the beginning of firstInterval
      }

      if (firstInterval.GetUpperBoundary() < secondInterval.GetUpperBoundary())
      {
        std::array<IntervalType, 2> result = {
          {IntervalType(firstInterval.GetLowerBoundary(), secondInterval.GetLowerBoundary()), IntervalType()}};
        return result; // secondInterval removes the end of firstInterval
      }
      std::array<IntervalType, 2> result = {
        {IntervalType(firstInterval.GetLowerBoundary(), secondInterval.GetLowerBoundary()),
         IntervalType(secondInterval.GetUpperBoundary(), firstInterval.GetUpperBoundary())}};
      return result; // secondInterval is completely enclosed in firstInterval and removes the middle
    }
  };
}

mitk::PlaneGeometryDataMapper2D::AllInstancesContainer mitk::PlaneGeometryDataMapper2D::s_AllInstances;

// input for this mapper ( = PlaneGeometryData)
const mitk::PlaneGeometryData *mitk::PlaneGeometryDataMapper2D::GetInput() const
{
  return static_cast<PlaneGeometryData *>(GetDataNode()->GetData());
}

mitk::PlaneGeometryDataMapper2D::PlaneGeometryDataMapper2D()
  : m_RenderOrientationArrows(false), m_ArrowOrientationPositive(true), m_DepthValue(1.0f)
{
  s_AllInstances.insert(this);
}

mitk::PlaneGeometryDataMapper2D::~PlaneGeometryDataMapper2D()
{
  s_AllInstances.erase(this);
}

vtkProp *mitk::PlaneGeometryDataMapper2D::GetVtkProp(mitk::BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  return ls->m_CrosshairAssembly;
}

void mitk::PlaneGeometryDataMapper2D::GenerateDataForRenderer(mitk::BaseRenderer *renderer)
{
  BaseLocalStorage *ls = m_LSH.GetLocalStorage(renderer);

  // The PlaneGeometryDataMapper2D mapper is special in that the rendering of
  // OTHER PlaneGeometryDatas affects how we render THIS PlaneGeometryData
  // (for the gap at the point where they intersect). A change in any of the
  // other PlaneGeometryData nodes could mean that we render ourself
  // differently, so we check for that here.
  for (auto it = s_AllInstances.begin(); it != s_AllInstances.end(); ++it)
  {
    bool generateDataRequired = ls->IsGenerateDataRequired(renderer, this, (*it)->GetDataNode());
    if (generateDataRequired)
      break;
  }

  ls->UpdateGenerateDataTime();

  // Collect all other PlaneGeometryDatas that are being mapped by this mapper
  m_OtherPlaneGeometries.clear();

  for (auto it = s_AllInstances.begin(); it != s_AllInstances.end(); ++it)
  {
    Self *otherInstance = *it;

    // Skip ourself
    if (otherInstance == this)
      continue;

    mitk::DataNode *otherNode = otherInstance->GetDataNode();
    if (!otherNode)
      continue;

    // Skip other PlaneGeometryData nodes that are not visible on this renderer
    if (!otherNode->IsVisible(renderer))
      continue;

    auto *otherData = dynamic_cast<PlaneGeometryData *>(otherNode->GetData());
    if (!otherData)
      continue;

    auto *otherGeometry = dynamic_cast<PlaneGeometry *>(otherData->GetPlaneGeometry());
    if (otherGeometry && !dynamic_cast<AbstractTransformGeometry *>(otherData->GetPlaneGeometry()))
    {
      m_OtherPlaneGeometries.push_back(otherNode);
    }
  }

  CreateVtkCrosshair(renderer);

  ApplyAllProperties(renderer);
}

void mitk::PlaneGeometryDataMapper2D::CreateVtkCrosshair(mitk::BaseRenderer *renderer)
{
  bool visible = true;
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ls->m_CrosshairActor->SetVisibility(0);
  ls->m_ArrowActor->SetVisibility(0);
  ls->m_CrosshairHelperLineActor->SetVisibility(0);

  GetDataNode()->GetVisibility(visible, renderer, "visible");

  if (!visible)
  {
    return;
  }

  PlaneGeometryData::ConstPointer input = this->GetInput();
  mitk::DataNode *geometryDataNode = renderer->GetCurrentWorldPlaneGeometryNode();
  const PlaneGeometryData *rendererWorldPlaneGeometryData =
    dynamic_cast<PlaneGeometryData *>(geometryDataNode->GetData());

  // intersecting with ourself?
  if (input.IsNull() || input.GetPointer() == rendererWorldPlaneGeometryData)
  {
    return; // nothing to do in this case
  }

  const auto *inputPlaneGeometry = dynamic_cast<const PlaneGeometry *>(input->GetPlaneGeometry());

  const auto *worldPlaneGeometry =
    dynamic_cast<const PlaneGeometry *>(rendererWorldPlaneGeometryData->GetPlaneGeometry());

  if (worldPlaneGeometry && dynamic_cast<const AbstractTransformGeometry *>(worldPlaneGeometry) == nullptr &&
      inputPlaneGeometry && dynamic_cast<const AbstractTransformGeometry *>(input->GetPlaneGeometry()) == nullptr)
  {
    const BaseGeometry *referenceGeometry = inputPlaneGeometry->GetReferenceGeometry();

    // calculate intersection of the plane data with the border of the
    // world geometry rectangle
    Point3D point1, point2;

    Line3D crossLine;

    // Calculate the intersection line of the input plane with the world plane
    if (worldPlaneGeometry->IntersectionLine(inputPlaneGeometry, crossLine))
    {
      bool hasIntersection = referenceGeometry ? CutCrossLineWithReferenceGeometry(referenceGeometry, crossLine) :
                                                 CutCrossLineWithPlaneGeometry(inputPlaneGeometry, crossLine);

      if (!hasIntersection)
      {
        return;
      }

      point1 = crossLine.GetPoint1();
      point2 = crossLine.GetPoint2();

      vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
      vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
      vtkSmartPointer<vtkPolyData> linesPolyData = vtkSmartPointer<vtkPolyData>::New();

      // Now iterate through all other lines displayed in this window and
      // calculate the positions of intersection with the line to be
      // rendered; these positions will be stored in lineParams to form a
      // gap afterwards.
      auto otherPlanesIt = m_OtherPlaneGeometries.begin();
      auto otherPlanesEnd = m_OtherPlaneGeometries.end();

      int gapSize = 32;
      this->GetDataNode()->GetPropertyValue("Crosshair.Gap Size", gapSize, nullptr);

      auto intervals = IntervalSet<double>(SimpleInterval<double>(0, 1));

      ScalarType lineLength = point1.EuclideanDistanceTo(point2);
      ScalarType gapInMM = gapSize * renderer->GetScaleFactorMMPerDisplayUnit();
      float gapSizeParam = gapInMM / lineLength;

      if (gapSize != 0)
      {
        while (otherPlanesIt != otherPlanesEnd)
        {
          bool ignorePlane = false;
          (*otherPlanesIt)->GetPropertyValue("Crosshair.Ignore", ignorePlane);
          if (ignorePlane)
          {
            ++otherPlanesIt;
            continue;
          }

          auto *otherPlaneGeometry = static_cast<PlaneGeometry *>(
            static_cast<PlaneGeometryData *>((*otherPlanesIt)->GetData())->GetPlaneGeometry());

          if (otherPlaneGeometry != inputPlaneGeometry && otherPlaneGeometry != worldPlaneGeometry)
          {
            double intersectionParam;
            if (otherPlaneGeometry->IntersectionPointParam(crossLine, intersectionParam) && intersectionParam > 0 &&
                intersectionParam < 1)
            {
              Point3D point = crossLine.GetPoint() + intersectionParam * crossLine.GetDirection();

              bool intersectionPointInsideOtherPlane =
                otherPlaneGeometry->HasReferenceGeometry() ?
                  TestPointInReferenceGeometry(otherPlaneGeometry->GetReferenceGeometry(), point) :
                  TestPointInPlaneGeometry(otherPlaneGeometry, point);

              if (intersectionPointInsideOtherPlane)
              {
                intervals -= SimpleInterval<double>(intersectionParam - gapSizeParam, intersectionParam + gapSizeParam);
              }
            }
          }
          ++otherPlanesIt;
        }
      }

      for (const auto &interval : intervals.getIntervals())
      {
        this->DrawLine(crossLine.GetPoint(interval.GetLowerBoundary()),
                       crossLine.GetPoint(interval.GetUpperBoundary()),
                       lines,
                       points);
      }

      // Add the points to the dataset
      linesPolyData->SetPoints(points);
      // Add the lines to the dataset
      linesPolyData->SetLines(lines);

      Vector3D orthogonalVector;
      orthogonalVector = inputPlaneGeometry->GetNormal();
      worldPlaneGeometry->Project(orthogonalVector, orthogonalVector);
      orthogonalVector.Normalize();

      // Visualize
      ls->m_Mapper->SetInputData(linesPolyData);
      ls->m_CrosshairActor->SetMapper(ls->m_Mapper);

      // Determine if we should draw the area covered by the thick slicing, default is false.
      // This will also show the area of slices that do not have thick slice mode enabled
      bool showAreaOfThickSlicing = false;
      GetDataNode()->GetBoolProperty("reslice.thickslices.showarea", showAreaOfThickSlicing);

      // determine the pixelSpacing in that direction
      double thickSliceDistance = SlicedGeometry3D::CalculateSpacing(
        referenceGeometry ? referenceGeometry->GetSpacing() : inputPlaneGeometry->GetSpacing(), orthogonalVector);

      IntProperty *intProperty = nullptr;
      if (GetDataNode()->GetProperty(intProperty, "reslice.thickslices.num") && intProperty)
        thickSliceDistance *= intProperty->GetValue() + 0.5;
      else
        showAreaOfThickSlicing = false;

      // not the nicest place to do it, but we have the width of the visible bloc in MM here
      // so we store it in this fancy property
      GetDataNode()->SetFloatProperty("reslice.thickslices.sizeinmm", thickSliceDistance * 2);

      ls->m_CrosshairActor->SetVisibility(1);

      vtkSmartPointer<vtkPolyData> arrowPolyData = vtkSmartPointer<vtkPolyData>::New();
      ls->m_Arrowmapper->SetInputData(arrowPolyData);
      if (this->m_RenderOrientationArrows)
      {
        ScalarType triangleSizeMM = 7.0 * renderer->GetScaleFactorMMPerDisplayUnit();

        vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
        vtkSmartPointer<vtkPoints> triPoints = vtkSmartPointer<vtkPoints>::New();

        DrawOrientationArrow(triangles, triPoints, triangleSizeMM, orthogonalVector, point1, point2);
        DrawOrientationArrow(triangles, triPoints, triangleSizeMM, orthogonalVector, point2, point1);
        arrowPolyData->SetPoints(triPoints);
        arrowPolyData->SetPolys(triangles);
        ls->m_ArrowActor->SetVisibility(1);
      }

      // Visualize
      vtkSmartPointer<vtkPolyData> helperlinesPolyData = vtkSmartPointer<vtkPolyData>::New();
      ls->m_HelperLinesmapper->SetInputData(helperlinesPolyData);
      if (showAreaOfThickSlicing)
      {
        vtkSmartPointer<vtkCellArray> helperlines = vtkSmartPointer<vtkCellArray>::New();
        // vectorToHelperLine defines how to reach the helperLine from the mainLine
        // got the right direction, so we multiply the width
        Vector3D vecToHelperLine = orthogonalVector * thickSliceDistance;

        this->DrawLine(point1 - vecToHelperLine, point2 - vecToHelperLine, helperlines, points);
        this->DrawLine(point1 + vecToHelperLine, point2 + vecToHelperLine, helperlines, points);

        // Add the points to the dataset
        helperlinesPolyData->SetPoints(points);

        // Add the lines to the dataset
        helperlinesPolyData->SetLines(helperlines);

        ls->m_CrosshairActor->GetProperty()->SetLineStipplePattern(0xf0f0);
        ls->m_CrosshairActor->GetProperty()->SetLineStippleRepeatFactor(1);
        ls->m_CrosshairHelperLineActor->SetVisibility(1);
      }
    }
  }
}

bool mitk::PlaneGeometryDataMapper2D::TestPointInPlaneGeometry(const PlaneGeometry *planeGeometry, const Point3D &point)
{
  Point2D mappedPoint;
  planeGeometry->Map(point, mappedPoint);
  planeGeometry->WorldToIndex(mappedPoint, mappedPoint);

  return (planeGeometry->GetBounds()[0] < mappedPoint[0] && mappedPoint[0] < planeGeometry->GetBounds()[1] &&
          planeGeometry->GetBounds()[2] < mappedPoint[1] && mappedPoint[1] < planeGeometry->GetBounds()[3]);
}

bool mitk::PlaneGeometryDataMapper2D::TestPointInReferenceGeometry(const BaseGeometry *referenceGeometry,
                                                                   const Point3D &point)
{
  return referenceGeometry->IsInside(point);
}

bool mitk::PlaneGeometryDataMapper2D::CutCrossLineWithPlaneGeometry(const PlaneGeometry *planeGeometry,
                                                                    Line3D &crossLine)
{
  Point2D indexLinePoint;
  Vector2D indexLineDirection;

  planeGeometry->Map(crossLine.GetPoint(), indexLinePoint);
  planeGeometry->Map(crossLine.GetPoint(), crossLine.GetDirection(), indexLineDirection);

  planeGeometry->WorldToIndex(indexLinePoint, indexLinePoint);
  planeGeometry->WorldToIndex(indexLineDirection, indexLineDirection);

  mitk::Point2D intersectionPoints[2];

  // Then, clip this line with the (transformed) bounding box of the
  // reference geometry.
  int nIntersections = Line3D::RectangleLineIntersection(planeGeometry->GetBounds()[0],
                                                         planeGeometry->GetBounds()[2],
                                                         planeGeometry->GetBounds()[1],
                                                         planeGeometry->GetBounds()[3],
                                                         indexLinePoint,
                                                         indexLineDirection,
                                                         intersectionPoints[0],
                                                         intersectionPoints[1]);

  if (nIntersections < 2)
  {
    return false;
  }

  planeGeometry->IndexToWorld(intersectionPoints[0], intersectionPoints[0]);
  planeGeometry->IndexToWorld(intersectionPoints[1], intersectionPoints[1]);

  Point3D point1, point2;

  planeGeometry->Map(intersectionPoints[0], point1);
  planeGeometry->Map(intersectionPoints[1], point2);
  crossLine.SetPoints(point1, point2);

  return true;
}

bool mitk::PlaneGeometryDataMapper2D::CutCrossLineWithReferenceGeometry(const BaseGeometry *referenceGeometry,
                                                                        Line3D &crossLine)
{
  Point3D boundingBoxMin, boundingBoxMax;
  boundingBoxMin = referenceGeometry->GetCornerPoint(0);
  boundingBoxMax = referenceGeometry->GetCornerPoint(7);

  Point3D indexLinePoint;
  Vector3D indexLineDirection;

  referenceGeometry->WorldToIndex(crossLine.GetPoint(), indexLinePoint);
  referenceGeometry->WorldToIndex(crossLine.GetDirection(), indexLineDirection);

  referenceGeometry->WorldToIndex(boundingBoxMin, boundingBoxMin);
  referenceGeometry->WorldToIndex(boundingBoxMax, boundingBoxMax);

  Point3D point1, point2;

  // Then, clip this line with the (transformed) bounding box of the
  // reference geometry.
  int nIntersections = Line3D::BoxLineIntersection(boundingBoxMin[0],
                                                   boundingBoxMin[1],
                                                   boundingBoxMin[2],
                                                   boundingBoxMax[0],
                                                   boundingBoxMax[1],
                                                   boundingBoxMax[2],
                                                   indexLinePoint,
                                                   indexLineDirection,
                                                   point1,
                                                   point2);

  if (nIntersections < 2)
  {
    return false;
  }

  referenceGeometry->IndexToWorld(point1, point1);
  referenceGeometry->IndexToWorld(point2, point2);
  crossLine.SetPoints(point1, point2);

  return true;
}

void mitk::PlaneGeometryDataMapper2D::DrawLine(mitk::Point3D p0,
                                               mitk::Point3D p1,
                                               vtkCellArray *lines,
                                               vtkPoints *points)
{
  vtkIdType pidStart = points->InsertNextPoint(p0[0], p0[1], p0[2]);
  vtkIdType pidEnd = points->InsertNextPoint(p1[0], p1[1], p1[2]);

  vtkSmartPointer<vtkLine> lineVtk = vtkSmartPointer<vtkLine>::New();
  lineVtk->GetPointIds()->SetId(0, pidStart);
  lineVtk->GetPointIds()->SetId(1, pidEnd);

  lines->InsertNextCell(lineVtk);
}

void mitk::PlaneGeometryDataMapper2D::DrawOrientationArrow(vtkSmartPointer<vtkCellArray> triangles,
                                                           vtkSmartPointer<vtkPoints> triPoints,
                                                           double triangleSizeMM,
                                                           Vector3D &orthogonalVector,
                                                           Point3D &point1,
                                                           Point3D &point2)
{
  // Draw arrows to indicate plane orientation
  // Vector along line
  Vector3D v1 = point2 - point1;
  v1.Normalize();
  v1 *= triangleSizeMM;

  // Orthogonal vector
  Vector3D v2 = orthogonalVector;
  v2 *= triangleSizeMM;
  if (!this->m_ArrowOrientationPositive)
    v2 *= -1.0;

  // Initialize remaining triangle coordinates accordingly
  Point3D p1 = point1 + v1 * 2.0;
  Point3D p2 = point1 + v1 + v2;

  vtkIdType t0 = triPoints->InsertNextPoint(point1[0], point1[1], point1[2]); // start of the line
  vtkIdType t1 = triPoints->InsertNextPoint(p1[0], p1[1], p1[2]);             // point on line
  vtkIdType t2 = triPoints->InsertNextPoint(p2[0], p2[1], p2[2]);             // direction point

  vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
  triangle->GetPointIds()->SetId(0, t0);
  triangle->GetPointIds()->SetId(1, t1);
  triangle->GetPointIds()->SetId(2, t2);

  triangles->InsertNextCell(triangle);
}

int mitk::PlaneGeometryDataMapper2D::DetermineThickSliceMode(DataNode *dn, int &thickSlicesNum)
{
  int thickSlicesMode = 0;
  // determine the state and the extend of the thick-slice mode
  mitk::ResliceMethodProperty *resliceMethodEnumProperty = nullptr;
  if (dn->GetProperty(resliceMethodEnumProperty, "reslice.thickslices") && resliceMethodEnumProperty)
    thickSlicesMode = resliceMethodEnumProperty->GetValueAsId();

  IntProperty *intProperty = nullptr;
  if (dn->GetProperty(intProperty, "reslice.thickslices.num") && intProperty)
  {
    thickSlicesNum = intProperty->GetValue();
    if (thickSlicesNum < 1)
      thickSlicesNum = 0;
    if (thickSlicesNum > 10)
      thickSlicesNum = 10;
  }

  if (thickSlicesMode == 0)
    thickSlicesNum = 0;

  return thickSlicesMode;
}

void mitk::PlaneGeometryDataMapper2D::ApplyAllProperties(BaseRenderer *renderer)
{
  LocalStorage *ls = m_LSH.GetLocalStorage(renderer);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_CrosshairActor);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_CrosshairHelperLineActor);
  ApplyColorAndOpacityProperties2D(renderer, ls->m_ArrowActor);

  float thickness;
  this->GetDataNode()->GetFloatProperty("Line width", thickness, renderer);
  ls->m_CrosshairActor->GetProperty()->SetLineWidth(thickness);
  ls->m_CrosshairHelperLineActor->GetProperty()->SetLineWidth(thickness);

  PlaneOrientationProperty *decorationProperty;
  this->GetDataNode()->GetProperty(decorationProperty, "decoration", renderer);
  if (decorationProperty != nullptr)
  {
    if (decorationProperty->GetPlaneDecoration() == PlaneOrientationProperty::PLANE_DECORATION_POSITIVE_ORIENTATION)
    {
      m_RenderOrientationArrows = true;
      m_ArrowOrientationPositive = true;
    }
    else if (decorationProperty->GetPlaneDecoration() ==
             PlaneOrientationProperty::PLANE_DECORATION_NEGATIVE_ORIENTATION)
    {
      m_RenderOrientationArrows = true;
      m_ArrowOrientationPositive = false;
    }
    else
    {
      m_RenderOrientationArrows = false;
    }
  }
}

void mitk::PlaneGeometryDataMapper2D::ApplyColorAndOpacityProperties2D(BaseRenderer *renderer, vtkActor2D *actor)
{
  float rgba[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  DataNode *node = GetDataNode();

  // check for color prop and use it for rendering if it exists
  node->GetColor(rgba, renderer, "color");
  // check for opacity prop and use it for rendering if it exists
  node->GetOpacity(rgba[3], renderer, "opacity");

  double drgba[4] = {rgba[0], rgba[1], rgba[2], rgba[3]};
  actor->GetProperty()->SetColor(drgba);
  actor->GetProperty()->SetOpacity(drgba[3]);
}

void mitk::PlaneGeometryDataMapper2D::SetDefaultProperties(mitk::DataNode *node,
                                                           mitk::BaseRenderer *renderer,
                                                           bool overwrite)
{
  mitk::IPropertyAliases *aliases = mitk::CoreServices::GetPropertyAliases();
  node->AddProperty("Line width", mitk::FloatProperty::New(1), renderer, overwrite);
  aliases->AddAlias("line width", "Crosshair.Line Width", "");
  node->AddProperty("Crosshair.Gap Size", mitk::IntProperty::New(32), renderer, overwrite);
  node->AddProperty("decoration",
                    mitk::PlaneOrientationProperty::New(PlaneOrientationProperty::PLANE_DECORATION_NONE),
                    renderer,
                    overwrite);
  aliases->AddAlias("decoration", "Crosshair.Orientation Decoration", "");

  Superclass::SetDefaultProperties(node, renderer, overwrite);
}

void mitk::PlaneGeometryDataMapper2D::UpdateVtkTransform(mitk::BaseRenderer * /*renderer*/)
{
}

mitk::PlaneGeometryDataMapper2D::LocalStorage::LocalStorage()
{
  m_CrosshairAssembly = vtkSmartPointer<vtkPropAssembly>::New();

  m_CrosshairActor = vtkSmartPointer<vtkActor2D>::New();
  m_ArrowActor = vtkSmartPointer<vtkActor2D>::New();
  m_CrosshairHelperLineActor = vtkSmartPointer<vtkActor2D>::New();

  m_HelperLinesmapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_Mapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();
  m_Arrowmapper = vtkSmartPointer<vtkPolyDataMapper2D>::New();

  m_CrosshairActor->SetMapper(m_Mapper);
  m_ArrowActor->SetMapper(m_Arrowmapper);
  m_CrosshairHelperLineActor->SetMapper(m_HelperLinesmapper);

  m_CrosshairActor->SetVisibility(0);
  m_ArrowActor->SetVisibility(0);
  m_CrosshairHelperLineActor->SetVisibility(0);

  m_CrosshairAssembly->AddPart(m_CrosshairActor);
  m_CrosshairAssembly->AddPart(m_ArrowActor);
  m_CrosshairAssembly->AddPart(m_CrosshairHelperLineActor);

  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToWorld();
  m_HelperLinesmapper->SetTransformCoordinate(tcoord);
  m_Mapper->SetTransformCoordinate(tcoord);
  //  tcoord->SetCoordinateSystemToNormalizedDisplay();
  m_Arrowmapper->SetTransformCoordinate(tcoord);
  tcoord->Delete();
}

mitk::PlaneGeometryDataMapper2D::LocalStorage::~LocalStorage()
{
}
