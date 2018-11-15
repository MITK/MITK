/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical Image Computing.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#include "mitkInteractionSchemeSwitcher.h"

// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"

// mitk core
#include "mitkInteractionEventObserver.h"

mitk::InteractionSchemeSwitcher::InteractionSchemeSwitcher()
  : m_InteractionScheme(MITKStandard)
{
  // nothing here
}

mitk::InteractionSchemeSwitcher::~InteractionSchemeSwitcher()
{
  // nothing here
}

void mitk::InteractionSchemeSwitcher::SetInteractionScheme(mitk::InteractionEventHandler* interactionEventHandler, InteractionScheme interactionScheme)
{
  switch (interactionScheme)
  {
    // MITK MODE
    case MITKStandard:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITK.xml");
      break;
    }
    case MITKRotationUncoupled:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKRotationUnCoupled.xml");
      break;
    }
    case MITKRotationCoupled:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKRotation.xml");
      break;
    }
    case MITKSwivel:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKSwivel.xml");
      break;
    }
    // PACS MODE
    case PACSStandard:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACS.xml");
      break;
    }
    case PACSLevelWindow:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACS.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSLevelWindow.xml");
      break;
    }
    case PACSPan:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACS.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSPan.xml");
      break;
    }
    case PACSScroll:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACS.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSScroll.xml");
      break;
    }
    case PACSZoom:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACS.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSZoom.xml");
      break;
    }
  }

  m_InteractionScheme = interactionScheme;
  InvokeEvent(InteractionSchemeChangedEvent());
}
