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


#include "mitkNodeDepententPointSetInteractor.h"
#include "mitkStateEvent.h"
#include "mitkEvent.h"

mitk::NodeDepententPointSetInteractor
::NodeDepententPointSetInteractor(const char * type, DataNode* dataNode, DataNode* dependentDataNode, int n)
:PointSetInteractor(type, dataNode, n), m_DependentDataNode(dependentDataNode)
{
}

mitk::NodeDepententPointSetInteractor::~NodeDepententPointSetInteractor()
{
}

float mitk::NodeDepententPointSetInteractor::CanHandleEvent(StateEvent const* stateEvent) const
{
  if (m_DependentDataNode.IsNull())
    return 0;

  // use sender to check if the specified dependentDataNode is visible there
  mitk::BaseRenderer* sender = stateEvent->GetEvent()->GetSender();
  if (sender == NULL)
    return 0;

  bool value = false;

  if ( m_DependentDataNode->GetPropertyList(sender)->GetBoolProperty("visible", value) == false)//property doesn't exist
    return 0;
  if (value == false) //the dependent node is invisible
    return 0;

  //ok, it is visible, so check the standard way.
  return Superclass::CanHandleEvent(stateEvent);
}


