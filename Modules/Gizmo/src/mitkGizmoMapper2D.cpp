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

#include "mitkGizmoMapper2D.h"

#include "mitkGizmo.h"

// MITK includes
#include <mitkVtkRepresentationProperty.h>
#include <mitkVtkInterpolationProperty.h>
#include <mitkLookupTableProperty.h>
#include <mitkVtkScalarModeProperty.h>
#include <mitkBaseRenderer.h>

// VTK includes
#include <vtkSphereSource.h>
#include <vtkConeSource.h>
#include <vtkCamera.h>
#include <vtkVectorOperators.h>
#include <vtkAppendPolydata.h>
#include <vtkCharArray.h>
#include <vtkPointData.h>
#include <vtkCellArray.h>
#include <vtkMath.h>

mitk::GizmoMapper2D::LocalStorage::LocalStorage()
{
    m_VtkPolyDataMapper = vtkSmartPointer<vtkPainterPolyDataMapper>::New();
    m_Actor = vtkSmartPointer<vtkActor>::New();
    m_Actor->SetMapper(m_VtkPolyDataMapper);
}

const mitk::Gizmo* mitk::GizmoMapper2D::GetInput()
{
    return static_cast<const Gizmo*>(GetDataNode()->GetData());
}

void mitk::GizmoMapper2D::ResetMapper(mitk::BaseRenderer* renderer)
{
    LocalStorage* ls = m_LSH.GetLocalStorage(renderer);
    ls->m_Actor->VisibilityOff();
}

namespace {

void AssignScalarValueTo(vtkPolyData* polydata, char value)
{
    vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();

    int numberOfPoints = polydata->GetNumberOfPoints();
    pointData->SetNumberOfComponents(1);
    pointData->SetNumberOfTuples(numberOfPoints);
    pointData->FillComponent(0, value);
    polydata->GetPointData()->SetScalars(pointData);

}

vtkSmartPointer<vtkPolyData> Create2DDisk(mitk::Vector3D cameraDirection,
                                          mitk::Vector3D viewRight,
                                          mitk::Vector3D viewUp,
                                          mitk::Point3D center,
                                          double radius)

{
  // build the axis itself (as a tube around the line defining the axis)
  vtkSmartPointer<vtkPolyData> disk = vtkSmartPointer<vtkPolyData>::New();

  mitk::Vector3D ringPointer;
  unsigned int numberOfRingPoints = 36;
  vtkSmartPointer<vtkPoints> ringPoints = vtkSmartPointer<vtkPoints>::New();
  vtkSmartPointer<vtkCellArray> ringPoly = vtkSmartPointer<vtkCellArray>::New();
  ringPoly->InsertNextCell(numberOfRingPoints+1);
  for (unsigned int segment = 0; segment < numberOfRingPoints; ++segment)
  {
    double x = std::cos( (double)(segment)/(double)numberOfRingPoints * 2.0 * vtkMath::Pi() );
    double y = std::sin( (double)(segment)/(double)numberOfRingPoints * 2.0 * vtkMath::Pi() );

    ringPointer = viewRight * x + viewUp * y;

    ringPoints->InsertPoint(segment, (center + ringPointer * radius).GetDataPointer());
    ringPoly->InsertCellPoint(segment);
  }
  ringPoly->InsertCellPoint(0);

  disk->SetPoints(ringPoints);
  disk->SetPolys(ringPoly);

  return disk;
}


vtkSmartPointer<vtkPolyData> Create2DArrow(mitk::Vector3D cameraDirection,
                                           mitk::Point3D arrowStart,
                                           mitk::Point3D arrowTip,
                                           int vertexValueMove,
                                           int vertexValueScale)
{
  mitk::Vector3D arrowDirection = arrowTip - arrowStart;
  mitk::Vector3D arrowOrthogonal = itk::CrossProduct(cameraDirection, arrowDirection);
  arrowOrthogonal.Normalize();

  double triangleFraction = 0.2;

  vtkSmartPointer<vtkPolyData> arrow = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->InsertPoint(0, arrowStart.GetDataPointer());
  points->InsertPoint(1, (arrowStart + arrowDirection * (1.0 - triangleFraction)).GetDataPointer());

  points->InsertPoint(2, arrowTip.GetDataPointer());
  points->InsertPoint(3, (arrowStart
                          + (1.0-triangleFraction) * arrowDirection
                          + arrowOrthogonal * (0.5 * triangleFraction * arrowDirection.GetNorm())).GetDataPointer());
  points->InsertPoint(4, (arrowStart
                          + (1.0-triangleFraction) * arrowDirection
                          - arrowOrthogonal * (0.5 * triangleFraction * arrowDirection.GetNorm())).GetDataPointer());
  arrow->SetPoints(points);

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();
  vtkIdType shaftLinePoints[] = {0,1};
  lines->InsertNextCell(2, shaftLinePoints);
  arrow->SetLines(lines);

  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  vtkIdType tipLinePoints[] = {2,3,4};
  polys->InsertNextCell(3, tipLinePoints);
  arrow->SetPolys(polys);

  vtkSmartPointer<vtkCharArray> pointData = vtkSmartPointer<vtkCharArray>::New();
  pointData->SetNumberOfComponents(1);
  pointData->SetNumberOfTuples(5);
  pointData->FillComponent(0, vertexValueScale);
  pointData->SetTuple1(0, vertexValueMove);
  pointData->SetTuple1(1, vertexValueMove);
  arrow->GetPointData()->SetScalars(pointData);

  return arrow;
}

}

vtkPolyData* mitk::GizmoMapper2D::GetVtkPolyData(mitk::BaseRenderer* renderer)
{
  return m_LSH.GetLocalStorage(renderer)->m_VtkPolyDataMapper->GetInput();
}

void mitk::GizmoMapper2D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  auto gizmo = GetInput();
  auto node = GetDataNode();

  LocalStorage* ls = m_LSH.GetLocalStorage(renderer);

  //check if something important has changed and we need to re-render
  if ( (ls->m_LastUpdateTime >= node->GetMTime())
      && (ls->m_LastUpdateTime >= gizmo->GetPipelineMTime())
      && (ls->m_LastUpdateTime >= renderer->GetCurrentWorldPlaneGeometryUpdateTime())
      && (ls->m_LastUpdateTime >= renderer->GetCurrentWorldPlaneGeometry()->GetMTime())
      && (ls->m_LastUpdateTime >= node->GetPropertyList()->GetMTime())
      && (ls->m_LastUpdateTime >= node->GetPropertyList(renderer)->GetMTime()) ) {
      return;
  }

  ls->m_LastUpdateTime.Modified();

  auto camera = renderer->GetVtkRenderer()->GetActiveCamera();

  auto plane = renderer->GetCurrentWorldGeometry2D();

  Point3D viewCenter;
  camera->GetFocalPoint(viewCenter.GetDataPointer());

  Point3D gizmoCenter3D = gizmo->GetCenter();
  Point3D gizmoCenterView = plane->ProjectPointOntoPlane(gizmoCenter3D);

  double diagonal = std::min( renderer->GetSizeX(), renderer->GetSizeY() );

  Vector3D viewUp;
  camera->GetViewUp(viewUp.GetDataPointer());
  Vector3D cameraDirection;
  camera->GetDirectionOfProjection(cameraDirection.GetDataPointer());
  Vector3D viewRight = itk::CrossProduct(viewUp, cameraDirection);

  auto appender = vtkSmartPointer<vtkAppendPolyData>::New();

  double arrowLength = 0.1 * diagonal; // fixed in relation to window size
  auto disk = Create2DDisk(cameraDirection, viewRight, viewUp,
                           gizmoCenterView - cameraDirection,
                           0.1 * arrowLength);
  AssignScalarValueTo(disk, Gizmo::MoveFreely);
  appender->AddInputData(disk);

  for (double direction = -1.0; direction < 2.0; direction += 2.0)
  {
    auto axisX = Create2DArrow(cameraDirection, gizmoCenterView,
                               plane->ProjectPointOntoPlane( gizmo->GetCenter() +
                                                            (  gizmo->GetAxisX()
                                                             * arrowLength) * direction ),
                               Gizmo::MoveAlongAxisX,
                               Gizmo::ScaleX
                              );
    appender->AddInputData(axisX);

    auto axisY = Create2DArrow(cameraDirection, gizmoCenterView,
                               plane->ProjectPointOntoPlane( gizmo->GetCenter() +
                                                            (  gizmo->GetAxisY()
                                                             * arrowLength) * direction ),
                               Gizmo::MoveAlongAxisY,
                               Gizmo::ScaleY
                              );
    appender->AddInputData(axisY);

    auto axisZ = Create2DArrow(cameraDirection, gizmoCenterView,
                               plane->ProjectPointOntoPlane( gizmo->GetCenter() +
                                                            (  gizmo->GetAxisZ()
                                                             * arrowLength) * direction ),
                               Gizmo::MoveAlongAxisZ,
                               Gizmo::ScaleZ
                              );
    appender->AddInputData(axisZ);
  }

  ls->m_VtkPolyDataMapper->SetInputConnection( appender->GetOutputPort() );

  float lineWidth = 3.0f;
  ls->m_Actor->GetProperty()->SetLineWidth(lineWidth);

  mitk::LookupTableProperty::Pointer lookupTableProp;
  this->GetDataNode()->GetProperty(lookupTableProp, "LookupTable", renderer);
  if ( lookupTableProp.IsNotNull() )
  {
    ls->m_VtkPolyDataMapper->SetLookupTable(lookupTableProp->GetLookupTable()->GetVtkLookupTable());
  }

  bool scalarVisibility = false;
  this->GetDataNode()->GetBoolProperty("scalar visibility", scalarVisibility);
  ls->m_VtkPolyDataMapper->SetScalarVisibility((scalarVisibility ? 1 : 0));

  if ( scalarVisibility )
  {
      mitk::VtkScalarModeProperty* scalarMode;
      if ( this->GetDataNode()->GetProperty(scalarMode, "scalar mode", renderer) )
          ls->m_VtkPolyDataMapper->SetScalarMode(scalarMode->GetVtkScalarMode());
      else
          ls->m_VtkPolyDataMapper->SetScalarModeToDefault();

      bool colorMode = false;
      this->GetDataNode()->GetBoolProperty("color mode", colorMode);
      ls->m_VtkPolyDataMapper->SetColorMode((colorMode ? 1 : 0));

      double scalarsMin = 0;
      this->GetDataNode()->GetDoubleProperty("ScalarsRangeMinimum", scalarsMin, renderer);

      double scalarsMax = 1.0;
      this->GetDataNode()->GetDoubleProperty("ScalarsRangeMaximum", scalarsMax, renderer);

      ls->m_VtkPolyDataMapper->SetScalarRange(scalarsMin, scalarsMax);
  }
}

