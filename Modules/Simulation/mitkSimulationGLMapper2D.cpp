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
#include "mitkSetVtkRendererVisitor.h"
#include "mitkSimulation.h"
#include "mitkSimulationGLMapper2D.h"
#include <sofa/core/visual/VisualParams.h>

void mitk::SimulationGLMapper2D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node != NULL)
  {
    Superclass::SetDefaultProperties(node, renderer, overwrite);
  }
}

mitk::SimulationGLMapper2D::SimulationGLMapper2D()
  : m_SimulationService(GetSimulationService())
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
  if (renderer == NULL)
    return;

  SliceNavigationController* sliceNavigationController = renderer->GetSliceNavigationController();

  if (sliceNavigationController == NULL)
    return;

  const PlaneGeometry* planeGeometry = sliceNavigationController->GetCurrentPlaneGeometry();

  if (planeGeometry == NULL)
    return;

  DataNode* dataNode = this->GetDataNode();

  if (dataNode == NULL)
    return;

  Simulation* simulation = static_cast<Simulation*>(dataNode->GetData());

  if (simulation == NULL)
    return;

  SetVtkRendererVisitor setVtkRendererVisitor(renderer->GetVtkRenderer(), planeGeometry->GetOrigin(), planeGeometry->GetNormal());
  simulation->GetRootNode()->executeVisitor(&setVtkRendererVisitor);

  m_SimulationService->SetActiveSimulation(simulation);

  sofa::core::visual::VisualParams* vParams = sofa::core::visual::VisualParams::defaultInstance();
  sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSOFASimulation();
  sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();

  sofaSimulation->updateVisual(rootNode.get()); // TODO: Check if this is happening only once per time step.
  sofaSimulation->draw(vParams, rootNode.get());
}
