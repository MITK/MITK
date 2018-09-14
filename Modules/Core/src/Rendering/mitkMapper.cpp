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

#include "mitkMapper.h"
#include "mitkBaseRenderer.h"
#include "mitkDataNode.h"
#include "mitkProperties.h"

mitk::Mapper::Mapper() : m_DataNode(nullptr), m_TimeStep(0)
{
}

mitk::Mapper::~Mapper()
{
}

mitk::BaseData *mitk::Mapper::GetData() const
{
  return m_DataNode == nullptr ? nullptr : m_DataNode->GetData();
}

mitk::DataNode *mitk::Mapper::GetDataNode() const
{
  return this->m_DataNode;
}

bool mitk::Mapper::GetColor(float rgb[3], mitk::BaseRenderer *renderer, const char *name) const
{
  const mitk::DataNode *node = GetDataNode();
  if (node == nullptr)
    return false;

  return node->GetColor(rgb, renderer, name);
}

bool mitk::Mapper::GetVisibility(bool &visible, mitk::BaseRenderer *renderer, const char *name) const
{
  const mitk::DataNode *node = GetDataNode();
  if (node == nullptr)
    return false;

  return node->GetVisibility(visible, renderer, name);
}

bool mitk::Mapper::GetOpacity(float &opacity, mitk::BaseRenderer *renderer, const char *name) const
{
  const mitk::DataNode *node = GetDataNode();
  if (node == nullptr)
    return false;

  return node->GetOpacity(opacity, renderer, name);
}

bool mitk::Mapper::GetLevelWindow(mitk::LevelWindow &levelWindow, mitk::BaseRenderer *renderer, const char *name) const
{
  const mitk::DataNode *node = GetDataNode();
  if (node == nullptr)
    return false;

  return node->GetLevelWindow(levelWindow, renderer, name);
}

bool mitk::Mapper::IsVisible(mitk::BaseRenderer *renderer, const char *name) const
{
  bool visible = true;
  GetDataNode()->GetVisibility(visible, renderer, name);
  return visible;
}

void mitk::Mapper::CalculateTimeStep(mitk::BaseRenderer *renderer)
{
  if ((renderer != nullptr) && (m_DataNode != nullptr))
  {
    m_TimeStep = renderer->GetTimeStep(m_DataNode->GetData());
  }
  else
  {
    m_TimeStep = 0;
  }
}

void mitk::Mapper::Update(mitk::BaseRenderer *renderer)
{
  const DataNode *node = GetDataNode();

  assert(node != nullptr);

  auto *data = static_cast<mitk::BaseData *>(node->GetData());

  if (!data)
    return;

  // Calculate time step of the input data for the specified renderer (integer value)
  this->CalculateTimeStep(renderer);

  // Check if time step is valid
  const TimeGeometry *dataTimeGeometry = data->GetTimeGeometry();
  if ((dataTimeGeometry == nullptr) || (dataTimeGeometry->CountTimeSteps() == 0) ||
      (!dataTimeGeometry->IsValidTimeStep(m_TimeStep)))
  {
    // TimeGeometry or time step is not valid for this data:
    // reset mapper so that nothing is displayed
    this->ResetMapper(renderer);
    return;
  }

  this->GenerateDataForRenderer(renderer);
}

bool mitk::Mapper::BaseLocalStorage::IsGenerateDataRequired(mitk::BaseRenderer *renderer,
                                                            mitk::Mapper *mapper,
                                                            mitk::DataNode *dataNode) const
{
  if (mapper && m_LastGenerateDataTime < mapper->GetMTime())
    return true;

  if (dataNode)
  {
    if (m_LastGenerateDataTime < dataNode->GetDataReferenceChangedTime())
      return true;

    mitk::BaseData *data = dataNode->GetData();

    if (data && m_LastGenerateDataTime < data->GetMTime())
      return true;
  }

  if (renderer && m_LastGenerateDataTime < renderer->GetTimeStepUpdateTime())
    return true;

  return false;
}

void mitk::Mapper::SetDefaultProperties(mitk::DataNode *node, mitk::BaseRenderer *renderer, bool overwrite)
{
  node->AddProperty("visible", mitk::BoolProperty::New(true), renderer, overwrite);
  node->AddProperty("layer", mitk::IntProperty::New(0), renderer, overwrite);
  node->AddProperty("name", mitk::StringProperty::New(DataNode::NO_NAME_VALUE()), renderer, overwrite);
}
