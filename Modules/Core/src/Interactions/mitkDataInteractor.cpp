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
#include "mitkDataNode.h"
#include "mitkStateMachineState.h"

// Predefined internal events/signals
const std::string mitk::DataInteractor::IntDeactivateMe = "DeactivateMe";
const std::string mitk::DataInteractor::IntLeaveWidget = "LeaveWidget";
const std::string mitk::DataInteractor::IntEnterWidget = "EnterWidget";

mitk::DataInteractor::DataInteractor()
{
}

mitk::DataInteractor::NodeType mitk::DataInteractor::GetDataNode() const
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



int mitk::DataInteractor::GetLayer() const
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
  if (m_DataNode.IsNotNull() )
  {
    // only reset if this interactor is still the current one
    if (m_DataNode->GetDataInteractor() == this)
      m_DataNode->SetDataInteractor(NULL);
  }
}

void mitk::DataInteractor::ConnectActionsAndFunctions()
{
  MITK_WARN<< "ConnectActionsAndFunctions in DataInteractor not implemented.\n DataInteractor will not be able to process any events.";
}

mitk::ProcessEventMode mitk::DataInteractor::GetMode() const
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

void mitk::DataInteractor::NotifyStart()
{
  this->GetDataNode()->InvokeEvent(StartInteraction());
}

void mitk::DataInteractor::NotifyResultReady()
{
  this->GetDataNode()->InvokeEvent(ResultReady());
}




void mitk::DataInteractor::DataNodeChanged()
{
}

void mitk::DataInteractor::DeletedNode()
{
  m_DataNode = nullptr;
}
