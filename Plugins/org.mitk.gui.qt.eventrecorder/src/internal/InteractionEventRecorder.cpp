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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "InteractionEventRecorder.h"

// Qt
#include <QMessageBox>
// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"

#include <usModuleInitialization.h>

US_INITIALIZE_MODULE("InteractionEventRecorder","liborg_mitk_gui_qt_eventrecorder")


const std::string InteractionEventRecorder::VIEW_ID = "org.mitk.views.interactioneventrecorder";

void InteractionEventRecorder::SetFocus()
{
  m_Controls.textFileName->setFocus();
}

void InteractionEventRecorder::StartRecording()
{
  MITK_INFO << "Start Recording";
  m_CurrentObserver->SetOutputFile(m_Controls.textFileName->text().toStdString());
  m_CurrentObserver->StartRecording();
}

void InteractionEventRecorder::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.btnStartRecording, SIGNAL(clicked()), this, SLOT(StartRecording()) );

  m_CurrentObserver = new mitk::EventRecorder();
  // Register as listener via micro services
  us::ServiceProperties props;

  props["name"] = std::string("EventRecorder");
  m_ServiceRegistration = us::GetModuleContext()->RegisterService<mitk::InteractionEventObserver>(m_CurrentObserver,props);


  /*

delete m_CurrentObserverDEBUG;
  m_ServiceRegistrationDEBUG.Unregister();
  */
}


