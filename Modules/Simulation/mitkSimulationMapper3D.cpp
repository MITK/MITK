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
#include "mitkSimulationMapper3D.h"
#include "mitkSimulationModel.h"
#include "mitkSimulationPropAssemblyVisitor.h"
#include <sofa/component/visualmodel/VisualStyle.h>
#include <sofa/core/visual/VisualParams.h>

mitk::SimulationMapper3D::LocalStorage::LocalStorage()
  : m_VtkProp(vtkSmartPointer<vtkPropAssembly>::New())
{
}

mitk::SimulationMapper3D::LocalStorage::~LocalStorage()
{
}

void mitk::SimulationMapper3D::SetDefaultProperties(DataNode* node, BaseRenderer* renderer, bool overwrite)
{
  if (node != NULL)
  {
    mitk::Simulation* simulation = dynamic_cast<mitk::Simulation*>(node->GetData());

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
      node->AddProperty("Simulation.Visual.Visual Models", BoolProperty::New(displayFlags.getShowVisualModels()), renderer, overwrite);
    }

    Superclass::SetDefaultProperties(node, renderer, overwrite);
  }
}

mitk::SimulationMapper3D::SimulationMapper3D()
{
}

mitk::SimulationMapper3D::~SimulationMapper3D()
{
}

void mitk::SimulationMapper3D::ApplyProperties(vtkActor* actor, mitk::BaseRenderer* renderer)
{
  if (actor == NULL)
  {
    mitk::DataNode* node = this->GetDataNode();

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

    mitk::Simulation* simulation = static_cast<mitk::Simulation*>(this->GetData());
    sofa::component::visualmodel::VisualStyle::SPtr visualStyle;
    simulation->GetRootNode()->get(visualStyle);

    bool update = false;

    sofa::core::visual::DisplayFlags* displayFlags = visualStyle->displayFlags.beginEdit();

    if (showBehaviorModels != displayFlags->getShowBehaviorModels())
    {
      displayFlags->setShowBehaviorModels(showBehaviorModels);
      update = true;
    }

    if (showForceFields != displayFlags->getShowForceFields())
    {
      displayFlags->setShowForceFields(showForceFields);
      update = true;
    }

    if (showInteractionForceFields != displayFlags->getShowInteractionForceFields())
    {
      displayFlags->setShowInteractionForceFields(showInteractionForceFields);
      update = true;
    }

    if (showBoundingCollisionModels != displayFlags->getShowBoundingCollisionModels())
    {
      displayFlags->setShowBoundingCollisionModels(showBoundingCollisionModels);
      update = true;
    }

    if (showCollisionModels != displayFlags->getShowCollisionModels())
    {
      displayFlags->setShowCollisionModels(showCollisionModels);
      update = true;
    }

    if (showMechanicalMappings != displayFlags->getShowMechanicalMappings())
    {
      displayFlags->setShowMechanicalMappings(showMechanicalMappings);
      update = true;
    }

    if (showMappings != displayFlags->getShowMappings())
    {
      displayFlags->setShowMappings(showMappings);
      update = true;
    }

    displayFlags->setShowNormals(showNormals);

    if (showWireFrame != displayFlags->getShowWireFrame())
    {
      displayFlags->setShowWireFrame(showWireFrame);
      update = true;
    }

    displayFlags->setShowVisualModels(showVisualModels);

    visualStyle->displayFlags.endEdit();

    if (update)
      simulation->GetDrawTool()->Reset();
  }
}

void mitk::SimulationMapper3D::GenerateDataForRenderer(mitk::BaseRenderer* renderer)
{
  mitk::Simulation* simulation = dynamic_cast<mitk::Simulation*>(this->GetData());

  if (sofa::simulation::getSimulation() == NULL)
  {
    if (simulation != NULL)
    {
      simulation->SetAsActiveSimulation();
      simulation->GetDrawTool()->Reset();
    }
    else
    {
      return;
    }
  }

  LocalStorage* localStorage = m_LocalStorageHandler.GetLocalStorage(renderer);
  localStorage->m_VtkProp = vtkSmartPointer<vtkPropAssembly>::New();

  if (simulation != NULL)
  {
    sofa::simulation::Simulation::SPtr sofaSimulation = simulation->GetSimulation();
    sofa::simulation::Node::SPtr rootNode = simulation->GetRootNode();
    sofa::core::visual::VisualParams* vParams = sofa::core::visual::VisualParams::defaultInstance();
    SimulationDrawTool* drawTool = simulation->GetDrawTool();

    sofa::simulation::Simulation* backupSimulation = NULL;
    sofa::core::visual::DrawTool* backupDrawTool = NULL;

    if (sofa::simulation::getSimulation() != sofaSimulation.get())
    {
      backupSimulation = sofa::simulation::getSimulation();
      backupDrawTool = vParams->drawTool();

      sofa::simulation::setSimulation(sofaSimulation.get());
      vParams->drawTool() = drawTool;
    }

    this->ApplyProperties(NULL, renderer);

    sofaSimulation->updateVisual(rootNode.get());
    sofaSimulation->draw(vParams, rootNode.get());
    drawTool->DisableUpdate();

    SimulationPropAssemblyVisitor propAssemblyVisitor(localStorage->m_VtkProp);
    rootNode->executeVisitor(&propAssemblyVisitor);

    std::vector<vtkActor*> actors = drawTool->GetActors();

    for (std::vector<vtkActor*>::const_iterator actor = actors.begin(); actor != actors.end(); ++actor)
      localStorage->m_VtkProp->AddPart(*actor);

    if (backupSimulation != NULL)
    {
      sofa::simulation::setSimulation(backupSimulation);
      vParams->drawTool() = backupDrawTool;
    }
  }
}

vtkProp* mitk::SimulationMapper3D::GetVtkProp(mitk::BaseRenderer* renderer)
{
  return m_LocalStorageHandler.GetLocalStorage(renderer)->m_VtkProp;
}
