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
#include "mitkGeometry3D.h"

static sofa::simulation::Simulation::SPtr CreateSimulation()
{
  const std::string key = "MultiMappingObject";

  if (sofa::simulation::xml::BaseElement::NodeFactory::HasKey(key))
    sofa::simulation::xml::BaseElement::NodeFactory::ResetEntry(key);

  return sofa::core::objectmodel::New<sofa::simulation::tree::TreeSimulation>();
}

mitk::Simulation::Simulation()
  : m_SOFASimulation(CreateSimulation())
{
}

mitk::Simulation::~Simulation()
{
  if (m_RootNode)
  {
    if (m_SOFASimulation)
      m_SOFASimulation->unload(m_RootNode);
  }
}

void mitk::Simulation::Animate()
{
  if (!m_RootNode || !m_SOFASimulation)
    return;

  boost::chrono::high_resolution_clock::time_point t0 = boost::chrono::high_resolution_clock::now();
  m_SOFASimulation->animate(m_RootNode.get(), m_RootNode->getDt());
  this->SetElapsedTime(boost::chrono::high_resolution_clock::now() - t0);

  this->UpdateOutputInformation();
}

sofa::core::visual::DrawTool* mitk::Simulation::GetDrawTool()
{
  return &m_DrawTool;
}

sofa::simulation::Node::SPtr mitk::Simulation::GetRootNode() const
{
  return m_RootNode;
}

sofa::simulation::Simulation::SPtr mitk::Simulation::GetSOFASimulation() const
{
  return m_SOFASimulation;
}

void mitk::Simulation::Reset()
{
  if (!m_RootNode || !m_SOFASimulation)
    return;

  m_SOFASimulation->reset(m_RootNode.get());
  m_RootNode->setTime(0.0);
  m_SOFASimulation->updateContext(m_RootNode.get());
}

bool mitk::Simulation::GetAnimationFlag() const
{
  return m_RootNode
    ? m_RootNode->getContext()->getAnimate()
    : false;
}

void mitk::Simulation::SetAnimationFlag(bool animate)
{
  if (m_RootNode)
    m_RootNode->getContext()->setAnimate(animate);
}

void mitk::Simulation::SetDt(double dt)
{
  if (m_RootNode)
    m_RootNode->setDt(dt);
}

void mitk::Simulation::SetRootNode(sofa::simulation::Node::SPtr rootNode)
{
  m_RootNode = rootNode;
}

bool mitk::Simulation::RequestedRegionIsOutsideOfTheBufferedRegion()
{
  return false;
}

void mitk::Simulation::SetRequestedRegion(const itk::DataObject*)
{
}

void mitk::Simulation::SetRequestedRegionToLargestPossibleRegion()
{
}

void mitk::Simulation::UpdateOutputInformation()
{
  using sofa::defaulttype::BoundingBox;
  using sofa::defaulttype::Vector3;

  if (this->GetSource().IsNotNull())
    this->GetSource()->UpdateOutputInformation();

  if (m_RootNode)
  {
    const BoundingBox& boundingBox = m_RootNode->f_bbox.getValue();
    mitk::Geometry3D::BoundsArrayType bounds;

    if (boundingBox.isValid())
    {
      const Vector3& min = boundingBox.minBBox();
      const Vector3& max = boundingBox.maxBBox();

      bounds[0] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(min.x());
      bounds[1] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(max.x());
      bounds[2] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(min.y());
      bounds[3] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(max.y());
      bounds[4] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(min.z());
      bounds[5] = static_cast<mitk::Geometry3D::BoundsArrayType::ValueType>(max.z());
    }
    else
    {
      bounds.Fill(0.0f);
    }

    mitk::BaseGeometry::Pointer geometry = this->GetGeometry();

    if (geometry.IsNull())
    {
      geometry = Geometry3D::New();
      geometry->SetBounds(bounds);
      this->SetGeometry(geometry);
    }
    else
    {
      geometry->SetBounds(bounds);
    }
  }

  this->GetTimeGeometry()->Update();
}

bool mitk::Simulation::VerifyRequestedRegion()
{
  return true;
}
