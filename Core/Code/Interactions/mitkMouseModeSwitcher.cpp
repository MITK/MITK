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

#include "mitkDisplayInteractor.h"
#include "mitkDisplayVectorInteractor.h"
#include "mitkDisplayVectorInteractorLevelWindow.h"
#include "mitkDisplayVectorInteractorScroll.h"

// us
#include "mitkGetModuleContext.h"
#include "mitkModule.h"
#include "mitkModuleRegistry.h"
#include "mitkInformer.h"

mitk::MouseModeSwitcher::MouseModeSwitcher(mitk::GlobalInteraction* gi) :
    m_GlobalInteraction(gi), m_ActiveInteractionScheme(MITK), m_ActiveMouseMode(MousePointer), m_LeftMouseButtonHandler(NULL)
{
  assert(gi);
  m_CurrentObserver = NULL;
  //this->InitializeListeners();
  this->SetInteractionScheme(m_ActiveInteractionScheme);
}

mitk::MouseModeSwitcher::~MouseModeSwitcher()
{
}

void mitk::MouseModeSwitcher::InitializeListeners()
{
  if (m_CurrentObserver.IsNull())
  {
    m_CurrentObserver = mitk::DisplayVectorInteractor::New();
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
  switch ( scheme )
  {
  case MITK :
    {
     m_CurrentObserver->LoadEventConfig("DisplayConfigMITK.xml");
    }
    break;
  case PACS :
    {
      m_CurrentObserver->LoadEventConfig("DisplayConfigPACS.xml");
    }
    break;
  }

  m_ActiveInteractionScheme = scheme;
}

void mitk::MouseModeSwitcher::SelectMouseMode(MouseMode /*mode*/)
{
//  if ( m_ActiveInteractionScheme != PACS )
//    return;
//
//  switch ( mode )
//  {
//  case MousePointer :
//    {
//      m_GlobalInteraction->RemoveListener( m_LeftMouseButtonHandler );
//      break;
//    } // case 0
//  case Scroll :
//    {
//      m_GlobalInteraction->RemoveListener( m_LeftMouseButtonHandler );
//
//      mitk::DisplayVectorInteractorScroll::Pointer scrollInteractor = mitk::DisplayVectorInteractorScroll::New(
//        "Scroll", new mitk::DisplayInteractor() );
//      m_LeftMouseButtonHandler = scrollInteractor;
//
//      m_GlobalInteraction->AddListener( m_LeftMouseButtonHandler );
//
//      break;
//    } // case 1
//  case LevelWindow :
//    {
//      m_GlobalInteraction->RemoveListener( m_LeftMouseButtonHandler );
//
//      mitk::DisplayVectorInteractorLevelWindow::Pointer lwInteractor = mitk::DisplayVectorInteractorLevelWindow::New(
//        "LevelWindow" );
//      m_LeftMouseButtonHandler = lwInteractor;
//
//      m_GlobalInteraction->AddListener( m_LeftMouseButtonHandler );
//
//      break;
//    } // case 2
//  case Zoom :
//    {
//      m_GlobalInteraction->RemoveListener( m_LeftMouseButtonHandler );
//
//      mitk::DisplayVectorInteractor::Pointer zoomInteractor = mitk::DisplayVectorInteractor::New(
//        "Zoom", new mitk::DisplayInteractor() );
//      m_LeftMouseButtonHandler = zoomInteractor;
//
//      m_GlobalInteraction->AddListener( m_LeftMouseButtonHandler );
//
//      break;
//    } // case 3
//  case Pan :
//    {
//      m_GlobalInteraction->RemoveListener( m_LeftMouseButtonHandler );
//
//      mitk::DisplayVectorInteractor::Pointer panInteractor = mitk::DisplayVectorInteractor::New(
//        "Pan", new mitk::DisplayInteractor() );
//      m_LeftMouseButtonHandler = panInteractor;
//
//      m_GlobalInteraction->AddListener( m_LeftMouseButtonHandler );
//
//      break;
//    } // case 4
//
//  } // switch (mode)
//
//  m_ActiveMouseMode = mode;
//
//  this->InvokeEvent( MouseModeChangedEvent() );

}

mitk::MouseModeSwitcher::MouseMode mitk::MouseModeSwitcher::GetCurrentMouseMode() const
{
  return m_ActiveMouseMode;
}

