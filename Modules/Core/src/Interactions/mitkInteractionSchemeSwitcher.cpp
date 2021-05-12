/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkInteractionSchemeSwitcher.h"

// mitk core
#include <mitkInteractionEventObserver.h>
#include <mitkExceptionMacro.h>

namespace mitk
{
  itkEventMacroDefinition(InteractionSchemeChangedEvent, itk::AnyEvent);
}

mitk::InteractionSchemeSwitcher::InteractionSchemeSwitcher()
{
  // nothing here
}

mitk::InteractionSchemeSwitcher::~InteractionSchemeSwitcher()
{
  // nothing here
}

void mitk::InteractionSchemeSwitcher::SetInteractionScheme(InteractionEventHandler* interactionEventHandler, InteractionScheme interactionScheme)
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
      interactionEventHandler->SetEventConfig("DisplayConfigMITKBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigCrosshair.xml");
      break;
    }
    case MITKRotationUncoupled:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigRotation.xml");
      break;
    }
    case MITKRotationCoupled:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigRotation.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigActivateCoupling.xml");
      break;
    }
    case MITKSwivel:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigSwivel.xml");
      break;
    }
    // PACS MODE
    case PACSBase:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACSBase.xml");
      break;
    }
    case PACSStandard:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACSBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigCrosshair.xml");
      break;
    }
    case PACSLevelWindow:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACSBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSLevelWindow.xml");
      break;
    }
    case PACSPan:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACSBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSPan.xml");
      break;
    }
    case PACSScroll:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACSBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSScroll.xml");
      break;
    }
    case PACSZoom:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigPACSBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigPACSZoom.xml");
      break;
    }
    default:
    {
      interactionEventHandler->SetEventConfig("DisplayConfigMITKBase.xml");
      interactionEventHandler->AddEventConfig("DisplayConfigCrosshair.xml");
    }
  }

  InvokeEvent(InteractionSchemeChangedEvent());
}
