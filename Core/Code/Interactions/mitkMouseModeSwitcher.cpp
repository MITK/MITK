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
#include "mitkGetModuleContext.h"
#include "mitkModule.h"
#include "mitkModuleRegistry.h"
#include "mitkInformer.h"

mitk::MouseModeSwitcher::MouseModeSwitcher() :
    m_ActiveInteractionScheme(MITK), m_ActiveMouseMode(MousePointer)
{
  m_CurrentObserver = NULL;
  this->InitializeListeners();
  this->SetInteractionScheme(m_ActiveInteractionScheme);
}

mitk::MouseModeSwitcher::~MouseModeSwitcher()
{
}

void mitk::MouseModeSwitcher::InitializeListeners()
{
  if (m_CurrentObserver.IsNull())
  {
    m_CurrentObserver = mitk::DisplayInteractor::New();
    m_CurrentObserver->LoadStateMachine("DisplayInteraction.xml");
    m_CurrentObserver->LoadEventConfig("DisplayConfigMITK.xml");
    // Register as listener
    mitk::ModuleContext* context = mitk::ModuleRegistry::GetModule(1)->GetModuleContext();
    mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::InformerService>();
    mitk::InformerService* service = dynamic_cast<mitk::InformerService*>(context->GetService(serviceRef));
    service->RegisterObserver(m_CurrentObserver.GetPointer());
  }
}

void mitk::MouseModeSwitcher::SetInteractionScheme(InteractionScheme scheme)
{
  switch (scheme)
  {
  case MITK:
  {
    m_CurrentObserver->LoadEventConfig("DisplayConfigMITK.xml");
  }
    break;
  case PACS:
  {
    m_CurrentObserver->LoadEventConfig("DisplayConfigPACS.xml");
  }
    break;
  }
  m_ActiveInteractionScheme = scheme;
  this->InvokeEvent( MouseModeChangedEvent() );
}

void mitk::MouseModeSwitcher::SelectMouseMode(MouseMode mode)
{
  if (m_ActiveInteractionScheme != PACS)
    return;

  switch (mode)
  {
  case MousePointer:
  {
    m_CurrentObserver->LoadEventConfig("DisplayConfigPACS.xml");
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
  this->InvokeEvent( MouseModeChangedEvent() );
}

mitk::MouseModeSwitcher::MouseMode mitk::MouseModeSwitcher::GetCurrentMouseMode() const
{
  return m_ActiveMouseMode;
}

