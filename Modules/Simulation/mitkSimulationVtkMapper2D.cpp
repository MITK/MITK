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

#include "mitkSimulation.h"
#include "mitkSimulationVtkMapper2D.h"
#include "mitkVtkSimulationPolyDataMapper2D.h"

mitk::SimulationVtkMapper2D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New())
{
}

mitk::SimulationVtkMapper2D::LocalStorage::~LocalStorage()
{
}

void mitk::SimulationVtkMapper2D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node != NULL)
  {
    node->AddProperty("Simulation.Visual.2D Rendering", BoolProperty::New(false), renderer, overwrite);
    Superclass::SetDefaultProperties(node, renderer, overwrite);
  }
}

mitk::SimulationVtkMapper2D::SimulationVtkMapper2D()
{
}

mitk::SimulationVtkMapper2D::~SimulationVtkMapper2D()
{
}

void mitk::SimulationVtkMapper2D::ApplyColorAndOpacityProperties(BaseRenderer*, vtkActor*)
{
}

void mitk::SimulationVtkMapper2D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  DataNode* dataNode = this->GetDataNode();

  if (dataNode == NULL)
    return;

  Simulation* simulation = dynamic_cast<Simulation*>(dataNode->GetData());

  if (simulation == NULL)
    return;

  LocalStorage* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

  bool enabled = false;
  dataNode->GetBoolProperty("Simulation.Visual.2D Rendering", enabled, renderer);

  if (!enabled)
  {
    localStorage->m_Actor->SetMapper(NULL);
    return;
  }

  if (localStorage->m_Mapper == NULL)
  {
    localStorage->m_Mapper = vtkSmartPointer<vtkSimulationPolyDataMapper2D>::New();
    localStorage->m_Mapper->SetSimulation(simulation);
  }

  if (localStorage->m_Actor->GetMapper() == NULL)
    localStorage->m_Actor->SetMapper(localStorage->m_Mapper);
}

vtkProp* mitk::SimulationVtkMapper2D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor;
}
