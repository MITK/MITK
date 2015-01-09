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

#include <mitkGL.h>
#include "mitkPlaneIntersectionVisitor.h"
#include "mitkSimulation.h"
#include "mitkSimulationGLMapper2D.h"

void mitk::SimulationGLMapper2D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node != NULL)
  {
    Superclass::SetDefaultProperties(node, renderer, overwrite);
  }
}

mitk::SimulationGLMapper2D::SimulationGLMapper2D()
{
}

mitk::SimulationGLMapper2D::~SimulationGLMapper2D()
{
}

void mitk::SimulationGLMapper2D::ApplyColorAndOpacityProperties(BaseRenderer*, vtkActor*)
{
}

void mitk::SimulationGLMapper2D::Paint(BaseRenderer* renderer)
{
  typedef PlaneIntersectionVisitor::Edge Edge;
  typedef PlaneIntersectionVisitor::Intersection Intersection;

  if (renderer == NULL)
    return;

  SliceNavigationController* sliceNavigationController = renderer->GetSliceNavigationController();

  if (sliceNavigationController == NULL)
    return;

  const PlaneGeometry* planeGeometry = sliceNavigationController->GetCurrentPlaneGeometry();

  if (planeGeometry == NULL)
    return;

  DataNode* node = this->GetDataNode();

  if (node == NULL)
    return;

  Simulation* simulation = static_cast<Simulation*>(node->GetData());

  if (simulation == NULL)
    return;

  PlaneIntersectionVisitor planeIntersectionVisitor(planeGeometry->GetOrigin(), planeGeometry->GetNormal());
  simulation->GetRootNode()->executeVisitor(&planeIntersectionVisitor);

  mitk::DisplayGeometry::Pointer displayGeometry = renderer->GetDisplayGeometry();
  Point2D point2D;

  const std::vector<Intersection>& intersections = planeIntersectionVisitor.GetIntersections();
  std::vector<Intersection>::const_iterator intersectionsEnd = intersections.end();

  for (std::vector<Intersection>::const_iterator intersectionIt = intersections.begin(); intersectionIt != intersectionsEnd; ++intersectionIt)
  {
    const std::vector<Edge>& edges = intersectionIt->edges;
    std::vector<Edge>::const_iterator edgesEnd = edges.end();

    glColor3dv(intersectionIt->color);
    glBegin(GL_LINES);

    for (std::vector<Edge>::const_iterator edgeIt = edges.begin(); edgeIt != edgesEnd; ++edgeIt)
    {
      displayGeometry->Map(edgeIt->v0, point2D);
      displayGeometry->WorldToDisplay(point2D, point2D);

      glVertex2dv(point2D.GetDataPointer());

      displayGeometry->Map(edgeIt->v1, point2D);
      displayGeometry->WorldToDisplay(point2D, point2D);

      glVertex2dv(point2D.GetDataPointer());
    }

    glEnd();
  }
}
