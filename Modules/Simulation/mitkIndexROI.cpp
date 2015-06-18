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

#include "mitkIndexROI.h"
#include <mitkLogMacros.h>
#include <sofa/core/loader/MeshLoader.h>
#include <sofa/core/visual/VisualParams.h>

mitk::IndexROI::IndexROI()
  : m_First(initData(&m_First, "first", "")),
    m_Last(initData(&m_Last, "last", "")),
    m_Individual(initData(&m_Individual, "individual", "")),
    m_Indices(initData(&m_Indices, "indices", "")),
    m_PointsInROI(initData(&m_PointsInROI, "pointsInROI", "")),
    m_Position(initData(&m_Position, "position", ""))
{
}

mitk::IndexROI::~IndexROI()
{
}

void mitk::IndexROI::init()
{
  using sofa::core::loader::MeshLoader;
  using sofa::core::objectmodel::BaseContext;
  using sofa::core::objectmodel::BaseData;
  typedef sofa::core::topology::BaseMeshTopology::SetIndex SetIndex;
  typedef sofa::defaulttype::ExtVec3fTypes::VecCoord VecCoord;

  if (!m_Position.isSet())
  {
    MeshLoader* loader = nullptr;
    this->getContext()->get(loader, BaseContext::Local);

    if (loader != nullptr)
    {
      BaseData* parent = loader->findField("position");

      if (parent != nullptr)
      {
        m_Position.setParent(parent);
        m_Position.setReadOnly(true);
      }
    }
  }

  if (!m_Indices.isSet())
  {
    sofa::helper::WriteAccessor<sofa::core::objectmodel::Data<SetIndex> > indices = m_Indices;

    const VecCoord& position = m_Position.getValue();
    unsigned int numPositions = static_cast<unsigned int>(position.size());
    unsigned int first = std::min(m_First.getValue(), numPositions);

    for (size_t i = 0; i < first; ++i)
      indices.push_back(i);

    unsigned int last = numPositions - std::min(m_Last.getValue(), numPositions - first);

    for (size_t i = last; i < numPositions; ++i)
      indices.push_back(i);

    SetIndex individual = m_Individual.getValue();
    size_t numIndividuals = individual.size();

    for (size_t i = 0; i < numIndividuals; ++i)
    {
      unsigned int index = individual[i];

      if (index >= first && index < last)
        indices.push_back(index);
    }
  }

  this->addInput(&m_Indices);
  this->addInput(&m_Position);

  this->addOutput(&m_PointsInROI);

  this->setDirtyValue();
}

void mitk::IndexROI::update()
{
  typedef sofa::core::topology::BaseMeshTopology::SetIndex SetIndex;
  typedef sofa::defaulttype::ExtVec3fTypes::VecCoord VecCoord;

  this->cleanDirty();

  sofa::helper::WriteAccessor<sofa::core::objectmodel::Data<VecCoord> > pointsInROI = m_PointsInROI;
  pointsInROI.clear();

  const VecCoord& position = m_Position.getValue();
  SetIndex indices = m_Indices.getValue();
  size_t numIndices = indices.size();

  for (size_t i = 0; i < numIndices; ++i)
    pointsInROI.push_back(position[indices[i]]);
}
