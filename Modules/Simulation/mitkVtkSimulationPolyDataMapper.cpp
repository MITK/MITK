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
#include "mitkVtkSimulationPolyDataMapper.h"
#include <sofa/core/visual/VisualParams.h>
#include <vtkObjectFactory.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>

namespace mitk
{
  vtkStandardNewMacro(vtkSimulationPolyDataMapper);
}

mitk::vtkSimulationPolyDataMapper::vtkSimulationPolyDataMapper()
  : m_SimulationService(GetSimulationService())
{
}

mitk::vtkSimulationPolyDataMapper::~vtkSimulationPolyDataMapper()
{
}

void mitk::vtkSimulationPolyDataMapper::Render(vtkRenderer* renderer, vtkActor* actor)
{
  if (renderer->GetRenderWindow()->CheckAbortStatus())
    return;

  if (m_Simulation.IsNull())
    return;

  renderer->GetRenderWindow()->MakeCurrent();

  m_SimulationService->SetSimulation(m_Simulation);

  sofa::core::visual::VisualParams* vParams = sofa::core::visual::VisualParams::defaultInstance();
  sofa::simulation::Simulation::SPtr sofaSimulation = m_Simulation->GetSimulation();
  sofa::simulation::Node::SPtr rootNode = m_Simulation->GetRootNode();

  sofaSimulation->updateVisual(rootNode.get());
  sofaSimulation->draw(vParams, rootNode.get());
}

void mitk::vtkSimulationPolyDataMapper::RenderPiece(vtkRenderer*, vtkActor*)
{
}

void mitk::vtkSimulationPolyDataMapper::SetSimulation(Simulation::Pointer simulation)
{
  m_Simulation = simulation;
}
