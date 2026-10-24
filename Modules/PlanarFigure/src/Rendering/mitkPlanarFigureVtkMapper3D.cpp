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

#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkContourTriangulator.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

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
}

mitk::PlanarFigureVtkMapper3D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New())
{
  m_Actor->SetMapper(vtkSmartPointer<vtkPolyDataMapper>::New());
  m_Actor->SetVisibility(false);
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

  if (figure->GetMTime() != localStorage->m_FigureTime || isFilled != localStorage->m_IsFilled)
  {
    localStorage->m_FigureTime = figure->GetMTime();
    localStorage->m_IsFilled = isFilled;

    auto *mapper = static_cast<vtkPolyDataMapper *>(localStorage->m_Actor->GetMapper());
    mapper->SetInputData(CreatePolyData(figure, isFilled));
  }

  float color[3] = { 1.0f, 1.0f, 1.0f };
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
