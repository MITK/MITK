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

#include "mitkSetVtkRendererVisitor.h"
#include "mitkSimulation.h"
#include "mitkSimulationVtkMapper3D.h"
#include "mitkVtkSimulationPolyDataMapper.h"
#include <sofa/component/visualmodel/VisualStyle.h>
#include <sofa/core/visual/VisualParams.h>

mitk::SimulationVtkMapper3D::LocalStorage::LocalStorage()
  : m_Actor(vtkSmartPointer<vtkActor>::New())
{
}

mitk::SimulationVtkMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::SimulationVtkMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node != NULL)
  {
    Simulation* simulation = dynamic_cast<Simulation*>(node->GetData());

    if (simulation != NULL)
    {
      sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();
      sofa::component::visualmodel::VisualStyle::SPtr visualStyle;

      rootNode->get(visualStyle);

      if (!visualStyle)
      {
        visualStyle = sofa::core::objectmodel::New<sofa::component::visualmodel::VisualStyle>();

        sofa::core::visual::DisplayFlags* displayFlags = visualStyle->displayFlags.beginEdit();
        displayFlags->setShowVisualModels();
        visualStyle->displayFlags.endEdit();

        rootNode->addObject(visualStyle);
      }

      const sofa::core::visual::DisplayFlags& displayFlags = visualStyle->displayFlags.getValue();

      node->AddProperty("Simulation.Behavior.Behavior Models", BoolProperty::New(displayFlags.getShowBehaviorModels()), renderer, overwrite);
      node->AddProperty("Simulation.Behavior.Force Fields", BoolProperty::New(displayFlags.getShowForceFields()), renderer, overwrite);
      node->AddProperty("Simulation.Behavior.Interactions", BoolProperty::New(displayFlags.getShowInteractionForceFields()), renderer, overwrite);
      node->AddProperty("Simulation.Collision.Bounding Trees", BoolProperty::New(displayFlags.getShowBoundingCollisionModels()), renderer, overwrite);
      node->AddProperty("Simulation.Collision.Collision Models", BoolProperty::New(displayFlags.getShowCollisionModels()), renderer, overwrite);
      node->AddProperty("Simulation.Mapping.Mechanical Mappings", BoolProperty::New(displayFlags.getShowMechanicalMappings()), renderer, overwrite);
      node->AddProperty("Simulation.Mapping.Visual Mappings", BoolProperty::New(displayFlags.getShowMappings()), renderer, overwrite);
      node->AddProperty("Simulation.Options.Normals", BoolProperty::New(displayFlags.getShowNormals()), renderer, overwrite);
      node->AddProperty("Simulation.Options.Wire Frame", BoolProperty::New(displayFlags.getShowWireFrame()), renderer, overwrite);
      node->AddProperty("Simulation.Visual.Visual Models", BoolProperty::New(displayFlags.getShowVisualModels() != sofa::core::visual::tristate::false_value), renderer, overwrite);
    }

    Superclass::SetDefaultProperties(node, renderer, overwrite);
  }
}

mitk::SimulationVtkMapper3D::SimulationVtkMapper3D()
{
}

mitk::SimulationVtkMapper3D::~SimulationVtkMapper3D()
{
}

void mitk::SimulationVtkMapper3D::ApplyColorAndOpacityProperties(vtkActor*, BaseRenderer*)
{
}

void mitk::SimulationVtkMapper3D::ApplySimulationProperties(BaseRenderer* renderer)
{
  DataNode* node = this->GetDataNode();

  bool showBehaviorModels;
  bool showForceFields;
  bool showInteractionForceFields;
  bool showBoundingCollisionModels;
  bool showCollisionModels;
  bool showMechanicalMappings;
  bool showMappings;
  bool showNormals;
  bool showWireFrame;
  bool showVisualModels;

  node->GetBoolProperty("Simulation.Behavior.Behavior Models", showBehaviorModels, renderer);
  node->GetBoolProperty("Simulation.Behavior.Force Fields", showForceFields, renderer);
  node->GetBoolProperty("Simulation.Behavior.Interactions", showInteractionForceFields, renderer);
  node->GetBoolProperty("Simulation.Collision.Bounding Trees", showBoundingCollisionModels, renderer);
  node->GetBoolProperty("Simulation.Collision.Collision Models", showCollisionModels, renderer);
  node->GetBoolProperty("Simulation.Mapping.Mechanical Mappings", showMechanicalMappings, renderer);
  node->GetBoolProperty("Simulation.Mapping.Visual Mappings", showMappings, renderer);
  node->GetBoolProperty("Simulation.Options.Normals", showNormals, renderer);
  node->GetBoolProperty("Simulation.Options.Wire Frame", showWireFrame, renderer);
  node->GetBoolProperty("Simulation.Visual.Visual Models", showVisualModels, renderer);

  Simulation* simulation = static_cast<Simulation*>(this->GetData());
  sofa::component::visualmodel::VisualStyle::SPtr visualStyle;
  simulation->GetRootNode()->get(visualStyle);

  sofa::core::visual::DisplayFlags* displayFlags = visualStyle->displayFlags.beginEdit();

  displayFlags->setShowBehaviorModels(showBehaviorModels);
  displayFlags->setShowForceFields(showForceFields);
  displayFlags->setShowInteractionForceFields(showInteractionForceFields);
  displayFlags->setShowBoundingCollisionModels(showBoundingCollisionModels);
  displayFlags->setShowCollisionModels(showCollisionModels);
  displayFlags->setShowMechanicalMappings(showMechanicalMappings);
  displayFlags->setShowMappings(showMappings);
  displayFlags->setShowNormals(showNormals);
  displayFlags->setShowWireFrame(showWireFrame);
  displayFlags->setShowVisualModels(showVisualModels);

  visualStyle->displayFlags.endEdit();
}

void mitk::SimulationVtkMapper3D::GenerateDataForRenderer(BaseRenderer* renderer)
{
  Simulation* simulation = static_cast<Simulation*>(this->GetData());

  if (simulation != NULL)
  {
    LocalStorage* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);

    if (localStorage->m_Mapper == NULL)
    {
      localStorage->m_Mapper = vtkSmartPointer<vtkSimulationPolyDataMapper>::New();
      localStorage->m_Mapper->SetSimulation(simulation);

      localStorage->m_Actor->SetMapper(localStorage->m_Mapper);

      SetVtkRendererVisitor initVisitor(renderer->GetVtkRenderer());
      simulation->GetRootNode()->executeVisitor(&initVisitor);
    }

    this->ApplySimulationProperties(renderer);
  }
}

vtkProp* mitk::SimulationVtkMapper3D::GetVtkProp(BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_Actor;
}
