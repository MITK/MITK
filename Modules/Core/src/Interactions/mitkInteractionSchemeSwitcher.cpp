/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInteractionSchemeSwitcher.h"

// us
#include <usGetModuleContext.h>
#include <usModuleContext.h>

// mitk core
#include <mitkInteractionEventObserver.h>
#include <mitkExceptionMacro.h>

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
  if (nullptr == interactionEventHandler)
  {
    mitkThrow() << "Not a valid interaction event handler to set the interaction scheme.";
  }

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
    default:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITK.xml");
    }
  }

  m_InteractionScheme = interactionScheme;
  InvokeEvent(InteractionSchemeChangedEvent());
}
