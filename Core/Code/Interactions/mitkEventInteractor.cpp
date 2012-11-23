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

#include "mitkEventInteractor.h"

mitk::EventInteractor::EventInteractor()
{
  m_DataNode = NULL;
}

mitk::DataNode::Pointer mitk::EventInteractor::GetDataNode()
{
  return m_DataNode;
}

void mitk::EventInteractor::SetDataNode(DataNode::Pointer dataNode)
{
  m_DataNode = dataNode;
  if (m_DataNode.IsNotNull()) {
    m_DataNode->SetInteractor(dynamic_cast <mitk::Interactor*>(this));
  }
}

int mitk::EventInteractor::GetLayer()
{
  int layer = -1;
  if (m_DataNode.IsNotNull()) {
    m_DataNode->GetIntProperty("layer",layer);
  }
  return layer;
}

bool mitk::EventInteractor::operator <(const EventInteractor::Pointer eventInteractor)
{
  return (GetLayer() < eventInteractor->GetLayer());
}

mitk::EventInteractor::~EventInteractor()
{
  // TODO Auto-generated destructor stub
}

