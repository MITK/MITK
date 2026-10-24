/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkPlanarFigureVtkMapper3D.h>

#include "mitkPlanarFigureViewHelpers.h"

#include <mitkPlanarFigure.h>
#include <mitkPlaneGeometry.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkContourTriangulator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkShaderProperty.h>
#include <vtkTransform.h>

#include <algorithm>

namespace
{
  double GetSignedArea(const std::vector<mitk::Point2D> &points)
  {
    double area = 0.0;

    for (std::size_t i = 0; i < points.size(); ++i)
    {
      const auto &p0 = points[i];
      const auto &p1 = points[(i + 1) % points.size()];
      area += p0[0] * p1[1] - p1[0] * p0[1];
    }

    return 0.5 * area;
  }

  /* The contours as lines in world coordinates, closed ones first and
   * repeating their first point. With fill, their triangulation is added.
   * The triangulation takes contours that wind clockwise around the normal
   * for holes, so outlines are turned counterclockwise and holes clockwise.
   */
  vtkSmartPointer<vtkPolyData> CreatePolyData(const mitk::PlanarFigure *figure, bool fill)
  {
    const auto *plane = figure->GetPlaneGeometry();
    auto contours = figure->GetContours();

    std::stable_partition(contours.begin(), contours.end(), [](const mitk::PlanarFigure::Contour &contour) {
      return contour.Closed;
    });

    for (auto &contour : contours)
    {
      const auto isHole = mitk::PlanarFigure::ContourRole::Hole == contour.Role;

      if (contour.Closed && (GetSignedArea(contour.Points) < 0.0) != isHole)
        std::reverse(contour.Points.begin(), contour.Points.end());
    }

    auto points = vtkSmartPointer<vtkPoints>::New();
    auto lines = vtkSmartPointer<vtkCellArray>::New();
    vtkIdType numberOfClosedContours = 0;

    for (const auto &contour : contours)
    {
      if (contour.Points.size() < 2)
        continue;

      const auto firstId = points->GetNumberOfPoints();
      lines->InsertNextCell(static_cast<vtkIdType>(contour.Points.size() + (contour.Closed ? 1 : 0)));

      for (const auto &point : contour.Points)
      {
        mitk::Point3D worldPoint;
        plane->Map(point, worldPoint);
        lines->InsertCellPoint(points->InsertNextPoint(worldPoint.GetDataPointer()));
      }

      if (contour.Closed)
      {
        lines->InsertCellPoint(firstId);
        ++numberOfClosedContours;
      }
    }

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->SetPoints(points);
    polyData->SetLines(lines);

    if (fill && numberOfClosedContours > 0)
    {
      // Counterclockwise in plane coordinates winds around this normal.
      const auto normal = itk::CrossProduct(plane->GetAxisVector(0), plane->GetAxisVector(1));
      const double vtkNormal[3] = { normal[0], normal[1], normal[2] };

      auto polygons = vtkSmartPointer<vtkCellArray>::New();
      vtkContourTriangulator::TriangulateContours(polyData, 0, numberOfClosedContours, polygons, vtkNormal);
      polyData->SetPolys(polygons);
    }

    return polyData;
  }

  // The center of the outlines in world coordinates, around which the figure pulses as in 2D.
  mitk::Point3D GetPulseCenter(const mitk::PlanarFigure *figure)
  {
    const auto contours = figure->GetContours();
    const auto hasOutline = std::any_of(contours.begin(), contours.end(), [](const mitk::PlanarFigure::Contour &contour) {
      return mitk::PlanarFigure::ContourRole::Outline == contour.Role;
    });

    mitk::Point2D center;
    center.Fill(0.0);
    std::size_t numberOfPoints = 0;

    for (const auto &contour : contours)
    {
      if (hasOutline && mitk::PlanarFigure::ContourRole::Outline != contour.Role)
        continue;

      for (const auto &point : contour.Points)
      {
        center[0] += point[0];
        center[1] += point[1];
        ++numberOfPoints;
      }
    }

    if (numberOfPoints > 0)
    {
      center[0] /= numberOfPoints;
      center[1] /= numberOfPoints;
    }

    mitk::Point3D worldCenter;
    figure->GetPlaneGeometry()->Map(center, worldCenter);
    return worldCenter;
  }
}

mitk::PlanarFigureVtkMapper3D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New())
{
  m_Actor->SetMapper(vtkSmartPointer<vtkPolyDataMapper>::New());
  m_Actor->SetVisibility(false);

  // A figure lies in the plane of an image slice, which it would fight for depth. Like a polygon
  // offset, it is moved a little towards the camera, more where the plane is seen at a flat angle.
  // VTK applies its own polygon offset only if it is switched on for all mappers. The constant part
  // is a few steps of a 24 bit depth buffer: VTK puts the near plane close to the camera, where a
  // larger constant amounts to millimeters and lets figures show through other slices.
  auto *shaderProperty = m_Actor->GetShaderProperty();
  shaderProperty->AddFragmentShaderReplacement("//VTK::UniformFlow::Impl", true,
    "float depthSlope = length(vec2(dFdx(gl_FragCoord.z), dFdy(gl_FragCoord.z)));\n"
    "  //VTK::UniformFlow::Impl\n",
    false);
  shaderProperty->AddFragmentShaderReplacement("//VTK::Depth::Impl", true,
    "gl_FragDepth = gl_FragCoord.z - 2.0 * depthSlope - 4.0 / 16777215.0;\n",
    false);
}

mitk::PlanarFigureVtkMapper3D::LocalStorage::~LocalStorage()
{
}

mitk::PlanarFigureVtkMapper3D::PlanarFigureVtkMapper3D()
{
}

mitk::PlanarFigureVtkMapper3D::~PlanarFigureVtkMapper3D()
{
}

void mitk::PlanarFigureVtkMapper3D::SetDefaultProperties(DataNode *node, BaseRenderer *renderer, bool overwrite)
{
  Superclass::SetDefaultProperties(node, renderer, overwrite);

  node->AddProperty("planarfigure.3drendering", BoolProperty::New(false), renderer, overwrite);
  node->AddProperty("planarfigure.3drendering.fill", BoolProperty::New(false), renderer, overwrite);
}

vtkProp *mitk::PlanarFigureVtkMapper3D::GetVtkProp(BaseRenderer *renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor;
}

void mitk::PlanarFigureVtkMapper3D::UpdateVtkTransform(BaseRenderer *)
{
}

void mitk::PlanarFigureVtkMapper3D::GenerateDataForRenderer(BaseRenderer *renderer)
{
  auto *localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  const auto *node = this->GetDataNode();
  const auto *figure = dynamic_cast<const PlanarFigure *>(node->GetData());

  bool isShownIn3D = false;
  node->GetBoolProperty("planarfigure.3drendering", isShownIn3D, renderer);

  if (figure == nullptr || !isShownIn3D || !node->IsVisible(renderer))
  {
    localStorage->m_Actor->SetVisibility(false);
    return;
  }

  bool isFilled = false;
  node->GetBoolProperty("planarfigure.3drendering.fill", isFilled, renderer);

  // The modification time of the figure leaves out its plane.
  const auto planeTime = figure->GetPlaneGeometry()->GetMTime();

  if (figure->GetMTime() != localStorage->m_FigureTime || planeTime != localStorage->m_PlaneTime || isFilled != localStorage->m_IsFilled)
  {
    localStorage->m_FigureTime = figure->GetMTime();
    localStorage->m_PlaneTime = planeTime;
    localStorage->m_IsFilled = isFilled;

    auto *mapper = static_cast<vtkPolyDataMapper *>(localStorage->m_Actor->GetMapper());
    mapper->SetInputData(CreatePolyData(figure, isFilled));
    localStorage->m_PulseCenter = GetPulseCenter(figure);
  }

  bool pulse = false;
  node->GetBoolProperty(PlanarFigureViewHelpers::PulseProperty, pulse, renderer);

  vtkSmartPointer<vtkTransform> pulseTransform;

  if (pulse)
  {
    auto *renderingManager = RenderingManager::GetInstance();
    renderingManager->RequestAnimationFrame(renderer->GetRenderWindow());

    float frequency = PlanarFigureViewHelpers::DefaultPulseFrequency;
    node->GetFloatProperty(PlanarFigureViewHelpers::PulseFrequencyProperty, frequency, renderer);

    const auto scale = PlanarFigureViewHelpers::GetPulseScale(renderingManager->GetAnimationTime(), frequency);
    const auto &center = localStorage->m_PulseCenter;

    pulseTransform = vtkSmartPointer<vtkTransform>::New();
    pulseTransform->Translate(center[0], center[1], center[2]);
    pulseTransform->Scale(scale, scale, scale);
    pulseTransform->Translate(-center[0], -center[1], -center[2]);
  }

  localStorage->m_Actor->SetUserTransform(pulseTransform);

  const auto *defaultColor = PlanarFigureViewHelpers::DefaultColor;
  float color[3] = { defaultColor[0], defaultColor[1], defaultColor[2] };
  node->GetColor(color, renderer);

  float opacity = 1.0f;
  node->GetOpacity(opacity, renderer);

  float lineWidth = 2.0f;
  node->GetFloatProperty("line width", lineWidth, renderer);

  auto *property = localStorage->m_Actor->GetProperty();
  property->SetColor(color[0], color[1], color[2]);
  property->SetOpacity(opacity);
  property->SetLineWidth(lineWidth * PlanarFigureViewHelpers::GetPixelScale(renderer));

  localStorage->m_Actor->SetVisibility(true);
}
