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
#include "mitkPointSet.h"
#include "mitkDataNode.h"
#include "mitkStateMachineState.h"

mitk::DataInteractor::DataInteractor()
{
}

const mitk::DataNode::Pointer mitk::DataInteractor::GetDataNode()
{
  return m_DataNode;
}

void mitk::DataInteractor::SetDataNode(DataNode::Pointer dataNode)
{
  if (dataNode == m_DataNode)
    return;

  if (m_DataNode.IsNotNull())
  { // if DataInteractors' DataNode is set to null, the "old" DataNode has to be notified (else the Dispatcher won't be notified either)
    m_DataNode->SetDataInteractor(NULL);
  }

  m_DataNode = dataNode;

  if (m_DataNode.IsNotNull())
  {
    m_DataNode->SetDataInteractor(this);
  }

  // notify implementations ...
  DataNodeChanged();
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

mitk::DataInteractor::~DataInteractor()
{
  if (m_DataNode.IsNotNull())
  {
    m_DataNode->SetInteractor(NULL);
  }
}

void mitk::DataInteractor::ConnectActionsAndFunctions()
{
  MITK_WARN<< "ConnectActionsAndFunctions in DataInteractor not implemented.\n DataInteractor will not be able to process any events.";
}

mitk::ProcessEventMode mitk::DataInteractor::GetMode()
{
  if (GetCurrentState()->GetMode() == "PREFER_INPUT")
  {
    return PREFERINPUT;
  }
  if (GetCurrentState()->GetMode() == "GRAB_INPUT")
  {
    return GRABINPUT;
  }
  return REGULAR;
}



void mitk::DataInteractor::DataNodeChanged()
{
}
