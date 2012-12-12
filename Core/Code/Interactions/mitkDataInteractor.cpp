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

#include "mitkDataInteractor.h"
#include "mitkInteractor.h"

#include "mitkDataNode.h"

mitk::DataInteractor::DataInteractor()
{
  m_DataNode = NULL;
}

mitk::DataNode::Pointer mitk::DataInteractor::GetDataNode()
{
  return m_DataNode;
}

void mitk::DataInteractor::SetDataNode(DataNode::Pointer dataNode)
{
  mitk::DataNode::Pointer tmpDN = m_DataNode;
  m_DataNode = dataNode;
  if (dataNode.IsNotNull())
  {
    dataNode->SetDataInteractor(this);
  }
  else if (tmpDN.IsNotNull())
  { // if DataInteractors' DataNode is set to null, the "old" DataNode has to be notified (else the Dispatcher won't be notified either)
    tmpDN->SetDataInteractor(NULL);
  }
}

int mitk::DataInteractor::GetLayer()
{
  int layer = -1;
  if (m_DataNode.IsNotNull())
  {
    m_DataNode->GetIntProperty("layer", layer);
  }
  return layer;
}

bool mitk::DataInteractor::operator <(const DataInteractor::Pointer dataInteractor)
{
  return (GetLayer() > dataInteractor->GetLayer()); // intentionally greater than !
}

mitk::DataInteractor::~DataInteractor()
{
  if (m_DataNode.IsNotNull())
  {
    m_DataNode->SetInteractor(NULL);
    m_DataNode = NULL;
  }
}
