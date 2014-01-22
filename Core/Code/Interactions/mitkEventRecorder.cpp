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

#include "mitkEventRecorder.h"
#include "mitkEventFactory.h"
#include "mitkInteractionEvent.h"


void mitk::EventRecorder::Notify(mitk::InteractionEvent *interactionEvent, bool /*isHandled*/)
{
  std::cout << EventFactory::EventToXML(interactionEvent) << "\n";
}

void mitk::EventRecorder::SetEventIgnoreList(std::vector<std::string> list)
{
 m_IgnoreList = list;
}
