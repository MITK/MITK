/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDataInteractor.h"
#include "mitkDataNode.h"
#include "mitkStateMachineState.h"

namespace mitk
{
  itkEventMacroDefinition(DataInteractorEvent, itk::AnyEvent);
  itkEventMacroDefinition(StartInteraction, DataInteractorEvent);
  itkEventMacroDefinition(ResultReady, DataInteractorEvent);
}

// Predefined internal events/signals
const std::string mitk::DataInteractor::IntDeactivateMe = "DeactivateMe";
const std::string mitk::DataInteractor::IntLeaveWidget = "LeaveWidget";
const std::string mitk::DataInteractor::IntEnterWidget = "EnterWidget";

mitk::DataInteractor::DataInteractor()
{
}

mitk::DataInteractor::~DataInteractor()
{
  auto dataNode = m_DataNode.Lock();

  if (dataNode.IsNotNull())
  {
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

  auto lockedDataNode = m_DataNode.Lock();

  if (lockedDataNode.IsNotNull())
    lockedDataNode->SetDataInteractor(nullptr);

  m_DataNode = dataNode;

  lockedDataNode = m_DataNode.Lock();

  if (lockedDataNode.IsNotNull())
    lockedDataNode->SetDataInteractor(this);

  this->DataNodeChanged();
}

int mitk::DataInteractor::GetLayer() const
{
  int layer = -1;

  auto dataNode = m_DataNode.Lock();

  if (dataNode.IsNotNull())
    dataNode->GetIntProperty("layer", layer);

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
