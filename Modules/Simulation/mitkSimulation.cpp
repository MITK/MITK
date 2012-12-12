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
#include <sofa/simulation/tree/TreeSimulation.h>

const float mitk::Simulation::ScaleFactor = 1000.0f;

static sofa::simulation::Simulation::SPtr CreateSimulation()
{
  std::string key = "MultiMappingObject";

  if (sofa::simulation::xml::BaseElement::NodeFactory::HasKey(key))
    sofa::simulation::xml::BaseElement::NodeFactory::ResetEntry(key);

  return sofa::core::objectmodel::New<sofa::simulation::tree::TreeSimulation>();
}

void mitk::Simulation::SetActiveSimulation(mitk::Simulation* simulation)
{
  if (simulation == NULL)
  {
    sofa::simulation::setSimulation(NULL);
    sofa::core::visual::VisualParams::defaultInstance()->drawTool() = NULL;
  }
  else
  {
    sofa::simulation::Simulation* sofaSimulation = simulation->m_Simulation.get();

    if (sofa::simulation::getSimulation() != sofaSimulation)
    {
      sofa::simulation::setSimulation(sofaSimulation);
      sofa::core::visual::VisualParams::defaultInstance()->drawTool() = &simulation->m_DrawTool;
    }
  }
}

mitk::Simulation::Simulation()
  : m_Simulation(CreateSimulation()),
    m_DefaultDT(0.0)
{
}

mitk::Simulation::~Simulation()
{
  if (m_Simulation != NULL)
  {
    if (m_RootNode != NULL)
      m_Simulation->unload(m_RootNode);

    if (sofa::simulation::getSimulation() == m_Simulation.get())
      SetActiveSimulation(NULL);
  }
}

double mitk::Simulation::GetDefaultDT() const
{
  return m_DefaultDT;
}

mitk::SimulationDrawTool* mitk::Simulation::GetDrawTool()
{
  return &m_DrawTool;
}

sofa::simulation::Node::SPtr mitk::Simulation::GetRootNode() const
{
  return m_RootNode;
}

sofa::simulation::Simulation::SPtr mitk::Simulation::GetSimulation() const
{
  return m_Simulation;
}

bool mitk::Simulation::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

void mitk::Simulation::SetAsActiveSimulation()
{
  SetActiveSimulation(this);
}

void mitk::Simulation::SetDefaultDT(double dt)
{
  m_DefaultDT = std::max(0.0, dt);
}

void mitk::Simulation::SetRequestedRegion(itk::DataObject*)
{
}

void mitk::Simulation::SetRequestedRegionToLargestPossibleRegion()
{
}

void mitk::Simulation::SetRootNode(sofa::simulation::Node* rootNode)
{
  m_RootNode.reset(rootNode);
}

void mitk::Simulation::UpdateOutputInformation()
{
  if (this->GetSource().IsNotNull())
    this->GetSource()->UpdateOutputInformation();

  if (m_RootNode != NULL)
  {
    const sofa::defaulttype::BoundingBox& boundingBox = m_RootNode->f_bbox.getValue();
    const sofa::defaulttype::Vector3& min = boundingBox.minBBox();
    const sofa::defaulttype::Vector3& max = boundingBox.maxBBox();

    mitk::Geometry3D::BoundsArrayType bounds;

    bounds[0] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(min.x() * ScaleFactor);
    bounds[1] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(max.x() * ScaleFactor);
    bounds[2] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(min.y() * ScaleFactor);
    bounds[3] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(max.y() * ScaleFactor);
    bounds[4] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(min.z() * ScaleFactor);
    bounds[5] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(max.z() * ScaleFactor);

    if(this->GetGeometry() != NULL)
    {
      this->GetGeometry()->SetBounds(bounds);
    }
    else
    {
      Geometry3D::Pointer geometry = Geometry3D::New();
      geometry->SetBounds(bounds);
      this->SetGeometry(geometry);
    }
  }

  this->GetTimeSlicedGeometry()->UpdateInformation();
}

bool mitk::Simulation::VerifyRequestedRegion()
{
  return true;
}
