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
    m_ActiveInteractionScheme(MITK), m_ActiveMouseMode(MousePointer), m_CurrentObserver(NULL)
{
  this->InitializeListeners(rendererName);
  this->SetInteractionScheme(m_ActiveInteractionScheme);
}

mitk::MouseModeSwitcher::~MouseModeSwitcher()
{
  m_ServiceRegistration.Unregister();
}

void mitk::MouseModeSwitcher::InitializeListeners(const std::string& rendererName)
{
  if (m_CurrentObserver.IsNull())
  {
    m_CurrentObserver = mitk::DisplayInteractor::New();
    m_CurrentObserver->LoadStateMachine("DisplayInteraction.xml");
    m_CurrentObserver->SetEventConfig("DisplayConfigMITK.xml");
    // Register as listener via micro services
    us::ServiceProperties props;
    props["name"] = std::string("DisplayInteractor");
    if (!rendererName.empty()) {
      props["rendererName"] = rendererName;
    }
    m_ServiceRegistration = us::GetModuleContext()->RegisterService<InteractionEventObserver>(
        m_CurrentObserver.GetPointer(),props);
  }
}

void mitk::MouseModeSwitcher::SetInteractionScheme(InteractionScheme scheme)
{
  switch (scheme)
  {
  case MITK:
  {
    m_CurrentObserver->SetEventConfig("DisplayConfigMITK.xml");
  }
    break;
  case PACS:
  {
    m_CurrentObserver->SetEventConfig("DisplayConfigPACS.xml");
  }
    break;

  case ROTATION:
  {
    m_CurrentObserver->SetEventConfig("DisplayConfigMITKRotationUnCoupled.xml");
  }
    break;

  case ROTATIONLINKED:
  {
    m_CurrentObserver->SetEventConfig("DisplayConfigMITKRotation.xml");
  }
    break;

  case SWIVEL:
  {
    m_CurrentObserver->SetEventConfig("DisplayConfigMITKSwivel.xml");
  }
    break;
  }
  m_ActiveInteractionScheme = scheme;
  this->InvokeEvent(MouseModeChangedEvent());
}

void mitk::MouseModeSwitcher::SetSelectionMode(bool selection)
{
  m_CurrentObserver->SetSelectionMode(selection);
}

void mitk::MouseModeSwitcher::SelectMouseMode(MouseMode mode)
{
  if (m_ActiveInteractionScheme != PACS)
    return;

  switch (mode)
  {
  case MousePointer:
  {
    m_CurrentObserver->SetEventConfig("DisplayConfigPACS.xml");
    break;
  } // case 0
  case Scroll:
  {
    m_CurrentObserver->AddEventConfig("DisplayConfigPACSScroll.xml");

    break;
  }
  case LevelWindow:
  {
    m_CurrentObserver->AddEventConfig("DisplayConfigPACSLevelWindow.xml");
    break;
  }
  case Zoom:
  {
    m_CurrentObserver->AddEventConfig("DisplayConfigPACSZoom.xml");
    break;
  }
  case Pan:
  {
    m_CurrentObserver->AddEventConfig("DisplayConfigPACSPan.xml");
    break;
  }
  } // end switch (mode)
  m_ActiveMouseMode = mode;
  this->InvokeEvent(MouseModeChangedEvent());
}

mitk::MouseModeSwitcher::MouseMode mitk::MouseModeSwitcher::GetCurrentMouseMode() const
{
  return m_ActiveMouseMode;
}

