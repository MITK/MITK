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

#include "QmitkCreateSimulationAction.h"

QmitkCreateSimulationAction::QmitkCreateSimulationAction()
{
}

QmitkCreateSimulationAction::~QmitkCreateSimulationAction()
{
}

void QmitkCreateSimulationAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  mitk::DataNode::Pointer dataNode;

  foreach(dataNode, selectedNodes)
  {
    // TODO
  }
}

void QmitkCreateSimulationAction::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkCreateSimulationAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkCreateSimulationAction::SetDecimated(bool)
{
}

void QmitkCreateSimulationAction::SetSmoothed(bool)
{
}
