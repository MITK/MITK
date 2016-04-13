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

#include "mitkGetSimulationService.h"
#include "mitkISimulationService.h"
#include "mitkPlaneIntersectionVisitor.h"
#include "mitkVtkSimulationPolyDataMapper2D.h"
#include <mitkBaseRenderer.h>
#include <mitkSliceNavigationController.h>
#include <sofa/helper/system/gl.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

namespace mitk
{
  vtkStandardNewMacro(vtkSimulationPolyDataMapper2D);
}

mitk::vtkSimulationPolyDataMapper2D::vtkSimulationPolyDataMapper2D()
  : m_SimulationService(GetSimulationService())
{
}

mitk::vtkSimulationPolyDataMapper2D::~vtkSimulationPolyDataMapper2D()
{
}

void mitk::vtkSimulationPolyDataMapper2D::Render(vtkRenderer* renderer, vtkActor*)
{
  typedef PlaneIntersectionVisitor::Edge Edge;
  typedef PlaneIntersectionVisitor::Intersection Intersection;

  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();

  if (renderWindow->CheckAbortStatus() == 1)
    return;

  if (renderWindow->SupportsOpenGL() == 0)
    return;

  if (m_Simulation.IsNull())
    return;

  if (!renderWindow->IsCurrent())
    renderWindow->MakeCurrent();

  BaseRenderer* mitkRenderer = BaseRenderer::GetInstance(renderer->GetRenderWindow());

  if (mitkRenderer == NULL)
    return;

  SliceNavigationController* sliceNavigationController = mitkRenderer->GetSliceNavigationController();

  if (sliceNavigationController == NULL)
    return;

  const PlaneGeometry* planeGeometry = sliceNavigationController->GetCurrentPlaneGeometry();

  if (planeGeometry == NULL)
    return;

  renderer->GetRenderWindow()->MakeCurrent();

  m_SimulationService->SetActiveSimulation(m_Simulation);

  PlaneIntersectionVisitor planeIntersectionVisitor(planeGeometry->GetOrigin(), planeGeometry->GetNormal());
  m_Simulation->GetRootNode()->executeVisitor(&planeIntersectionVisitor);

  glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  glEnable(GL_COLOR_MATERIAL);

  const std::vector<Intersection>& intersections = planeIntersectionVisitor.GetIntersections();
  std::vector<Intersection>::const_iterator intersectionsEnd = intersections.end();

  for (std::vector<Intersection>::const_iterator intersectionIt = intersections.begin(); intersectionIt != intersectionsEnd; ++intersectionIt)
  {
    const std::vector<Edge>& edges = intersectionIt->edges;
    std::vector<Edge>::const_iterator edgesEnd = edges.end();

    glColor4fv(intersectionIt->color);
    glBegin(GL_LINES);

    for (std::vector<Edge>::const_iterator edgeIt = edges.begin(); edgeIt != edgesEnd; ++edgeIt)
    {
      glVertex3dv(edgeIt->v0.GetDataPointer());
      glVertex3dv(edgeIt->v1.GetDataPointer());
    }

    glEnd();
  }

  glDisable(GL_COLOR_MATERIAL);
}

void mitk::vtkSimulationPolyDataMapper2D::RenderPiece(vtkRenderer*, vtkActor*)
{
}

void mitk::vtkSimulationPolyDataMapper2D::SetSimulation(Simulation::Pointer simulation)
{
  m_Simulation = simulation;
}

double* mitk::vtkSimulationPolyDataMapper2D::GetBounds()
{
  if (m_Simulation.IsNull())
    return Superclass::GetBounds();

  sofa::simulation::Node::SPtr rootNode = m_Simulation->GetRootNode();
  const sofa::defaulttype::BoundingBox& bbox = rootNode->f_bbox.getValue();
  const sofa::defaulttype::Vector3& min = bbox.minBBox();
  const sofa::defaulttype::Vector3& max = bbox.maxBBox();

  Bounds[0] = min.x();
  Bounds[1] = max.x();
  Bounds[2] = min.y();
  Bounds[3] = max.y();
  Bounds[4] = min.z();
  Bounds[5] = max.z();

  return this->Bounds;
}
