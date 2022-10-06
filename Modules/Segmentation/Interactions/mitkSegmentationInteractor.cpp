/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationInteractor.h"

#include "mitkInteractionPositionEvent.h"
#include <mitkInternalEvent.h>
#include <mitkSegmentationInteractionEvents.h>

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"

#include <cstring>

mitk::SegmentationInteractor::SegmentationInteractor()
{
  // register the class (itself) as an interaction event observer via micro services
  us::ServiceProperties props;
  props["name"] = std::string("SegmentationInteractor");
  m_ServiceRegistration = us::GetModuleContext()->RegisterService<InteractionEventObserver>(this, props);

  auto contextModule = us::GetModuleContext()->GetModule();

  this->LoadStateMachine("SegmentationInteraction.xml", contextModule);
  this->SetEventConfig("SegmentationConfig.xml", contextModule);
}

mitk::SegmentationInteractor::~SegmentationInteractor()
{
  m_ServiceRegistration.Unregister();
}

void mitk::SegmentationInteractor::ConnectActionsAndFunctions()
{
  Superclass::ConnectActionsAndFunctions();

  // CONNECT_FUNCTION("change_active_label", ChangeActiveLabel);
  CONNECT_FUNCTION("EnterRenderWindow", OnEnterRenderWindow);
  CONNECT_FUNCTION("LeaveRenderWindow", OnLeaveRenderWindow);
}

void mitk::SegmentationInteractor::OnEnterRenderWindow(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (this->IsEnabled())
  {
    auto* internalEvent = dynamic_cast<InternalEvent*>(interactionEvent);
    if (nullptr == internalEvent)
    {
      return;
    }

    this->InvokeEvent(SegmentationInteractionEvent(interactionEvent, true));
  }
}

void mitk::SegmentationInteractor::OnLeaveRenderWindow(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (this->IsEnabled())
  {
    auto* internalEvent = dynamic_cast<InternalEvent*>(interactionEvent);
    if (nullptr == internalEvent)
    {
      return;
    }

    this->InvokeEvent(SegmentationInteractionEvent(interactionEvent, false));
  }
}
