/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-09-14 09:48:51 +0200 (Di, 14 Sep 2010) $
Version:   $Revision: 26074 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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


