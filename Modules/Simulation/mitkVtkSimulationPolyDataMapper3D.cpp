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
#include "mitkVtkSimulationPolyDataMapper3D.h"
#include <sofa/core/visual/VisualParams.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

namespace mitk
{
  vtkStandardNewMacro(vtkSimulationPolyDataMapper3D);
}

mitk::vtkSimulationPolyDataMapper3D::vtkSimulationPolyDataMapper3D()
  : m_SimulationService(GetSimulationService())
{
}

mitk::vtkSimulationPolyDataMapper3D::~vtkSimulationPolyDataMapper3D()
{
}

void mitk::vtkSimulationPolyDataMapper3D::Render(vtkRenderer* renderer, vtkActor*)
{
  vtkRenderWindow* renderWindow = renderer->GetRenderWindow();

  if (renderWindow->CheckAbortStatus() == 1)
    return;

  if (renderWindow->SupportsOpenGL() == 0)
    return;

  if (m_Simulation.IsNull())
    return;

  if (!renderWindow->IsCurrent())
    renderWindow->MakeCurrent();

  m_SimulationService->SetActiveSimulation(m_Simulation);

  sofa::core::visual::VisualParams* vParams = sofa::core::visual::VisualParams::defaultInstance();
  sofa::simulation::Simulation::SPtr sofaSimulation = m_Simulation->GetSOFASimulation();
  sofa::simulation::Node::SPtr rootNode = m_Simulation->GetRootNode();

  sofaSimulation->updateVisual(rootNode.get());
  sofaSimulation->draw(vParams, rootNode.get());

  // SOFA potentially disables GL_BLEND but VTK relies on it.
  glEnable(GL_BLEND);
}

void mitk::vtkSimulationPolyDataMapper3D::RenderPiece(vtkRenderer*, vtkActor*)
{
}

void mitk::vtkSimulationPolyDataMapper3D::SetSimulation(Simulation::Pointer simulation)
{
  m_Simulation = simulation;
}

double* mitk::vtkSimulationPolyDataMapper3D::GetBounds()
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
