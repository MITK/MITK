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

#include "mitkMouseModeSwitcher.h"
// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"

#include "mitkInteractionEventObserver.h"

mitk::MouseModeSwitcher::MouseModeSwitcher(const std::string& rendererName) :
    m_ActiveInteractionScheme(MITK), m_ActiveMouseMode(MousePointer)
{
  this->AddRenderer(rendererName);
  this->SetInteractionScheme(m_ActiveInteractionScheme);
}

mitk::MouseModeSwitcher::~MouseModeSwitcher()
{
  for (auto serviceRegistration : m_ServiceRegistrations) {
    serviceRegistration.Unregister();
  }
}

void mitk::MouseModeSwitcher::AddRenderer(const std::string& rendererName)
{
  DisplayInteractor::Pointer currentObserver = mitk::DisplayInteractor::New();
  currentObserver->LoadStateMachine("DisplayInteraction.xml");
  currentObserver->SetEventConfig("DisplayConfigMITK.xml");
  // Register as listener via micro services
  us::ServiceProperties props;
  props["name"] = std::string("DisplayInteractor");
  if (!rendererName.empty()) {
    props["rendererName"] = rendererName;
  }
  us::ServiceRegistration<InteractionEventObserver> serviceRegistration = 
    us::GetModuleContext()->RegisterService<InteractionEventObserver>(currentObserver.GetPointer(),props);

  m_Observers.push_back(currentObserver);
  m_ServiceRegistrations.push_back(serviceRegistration);
}

void mitk::MouseModeSwitcher::SetInteractionScheme(InteractionScheme scheme)
{
  std::string eventConfig;
  switch (scheme)
  {
  case MITK:
  {
    eventConfig = "DisplayConfigMITK.xml";
  }
    break;
  case PACS:
  {
    eventConfig = "DisplayConfigPACS.xml";
  }
    break;

  case ROTATION:
  {
    eventConfig = "DisplayConfigMITKRotationUnCoupled.xml";
  }
    break;

  case ROTATIONLINKED:
  {
    eventConfig = "DisplayConfigMITKRotation.xml";
  }
    break;

  case SWIVEL:
  {
    eventConfig = "DisplayConfigMITKSwivel.xml";
  }
    break;
  }

  if (!eventConfig.empty()) {
    for (auto observer : m_Observers) {
      observer->SetEventConfig(eventConfig);
    }
  }
  m_ActiveInteractionScheme = scheme;
  this->InvokeEvent(MouseModeChangedEvent());
}

void mitk::MouseModeSwitcher::SetSelectionMode(bool selection)
{
  for (auto observer : m_Observers) {
    observer->SetSelectionMode(selection);
  }
}

void mitk::MouseModeSwitcher::SelectMouseMode(MouseMode mode)
{
  if (m_ActiveInteractionScheme != PACS)
    return;

  std::string eventConfig;
  switch (mode)
  {
  case MousePointer:
  {
    eventConfig = "DisplayConfigPACS.xml";
    break;
  } // case 0
  case Scroll:
  {
    eventConfig = "DisplayConfigPACSScroll.xml";
    break;
  }
  case LevelWindow:
  {
    eventConfig = "DisplayConfigPACSLevelWindow.xml";
    break;
  }
  case Zoom:
  {
    eventConfig = "DisplayConfigPACSZoom.xml";
    break;
  }
  case Pan:
  {
    eventConfig = "DisplayConfigPACSPan.xml";
    break;
  }
  } // end switch (mode)

  if (!eventConfig.empty()) {
    for (auto observer : m_Observers) {
      observer->AddEventConfig(eventConfig);
    }
  }

  m_ActiveMouseMode = mode;
  this->InvokeEvent(MouseModeChangedEvent());
}

mitk::MouseModeSwitcher::MouseMode mitk::MouseModeSwitcher::GetCurrentMouseMode() const
{
  return m_ActiveMouseMode;
}

