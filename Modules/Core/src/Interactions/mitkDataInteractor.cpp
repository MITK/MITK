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

mitk::DataInteractor::~DataInteractor()
{
  if (!m_DataNode.IsExpired())
  {
    auto dataNode = m_DataNode.Lock();

    if (dataNode->GetDataInteractor() == this)
      dataNode->SetDataInteractor(nullptr);
  }
}

mitk::DataNode *mitk::DataInteractor::GetDataNode() const
{
  return m_DataNode.Lock();
}

void mitk::DataInteractor::SetDataNode(DataNode *dataNode)
{
  if (dataNode == m_DataNode)
    return;

  if (!m_DataNode.IsExpired())
    m_DataNode.Lock()->SetDataInteractor(nullptr);

  m_DataNode = dataNode;

  if (dataNode != nullptr)
    m_DataNode.Lock()->SetDataInteractor(this);

  this->DataNodeChanged();
}

int mitk::DataInteractor::GetLayer() const
{
  int layer = -1;

  if (!m_DataNode.IsExpired())
    m_DataNode.Lock()->GetIntProperty("layer", layer);

  return layer;
}

void mitk::DataInteractor::ConnectActionsAndFunctions()
{
  MITK_WARN << "DataInteractor::ConnectActionsAndFunctions() is not implemented.";
}

mitk::ProcessEventMode mitk::DataInteractor::GetMode() const
{
  auto mode = this->GetCurrentState()->GetMode();

  if (mode == "PREFER_INPUT")
    return PREFERINPUT;

  if (mode == "GRAB_INPUT")
    return GRABINPUT;

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
