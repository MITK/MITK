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
#include <QFileDialog>
// us
#include "usGetModuleContext.h"
#include "usModuleContext.h"
#include "usModuleResource.h"

#include <usModuleInitialization.h>
#include <mitkXML2EventParser.h>
#include <vtkSmartPointer.h>
#include "QmitkRenderWindow.h"


US_INITIALIZE_MODULE


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

void InteractionEventRecorder::StopRecording()
{
  MITK_INFO << "Stop Recording";
  m_CurrentObserver->StopRecording();
}

void InteractionEventRecorder::Play()
{
  std::ifstream xmlStream(m_Controls.textFileName->text().toStdString().c_str());
  mitk::XML2EventParser parser(xmlStream);
  mitk::XML2EventParser::EventContainerType events = parser.GetInteractions();

  MITK_INFO << "parsed events";
  for (int i=0; i < events.size(); ++i)
  {
    //this->GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer()->GetDispatcher()->ProcessEvent(events.at(i));
    events.at(i)->GetSender()->GetDispatcher()->ProcessEvent(events.at(i));
  }
  MITK_INFO << "DONE";

}

void InteractionEventRecorder::OpenFile()
{
  QString fn = QFileDialog::getOpenFileName(NULL, "Open File...",
                                                QString(), "All Files (*)");
      if (!fn.isEmpty())
        this->m_Controls.textFileName->setText(fn);
}

void InteractionEventRecorder::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.btnStopRecording, SIGNAL(clicked()), this, SLOT(StopRecording()) );
  connect( m_Controls.btnStartRecording, SIGNAL(clicked()), this, SLOT(StartRecording()) );
  connect( m_Controls.btnPlay, SIGNAL(clicked()), this, SLOT(Play()) );
  connect( m_Controls.btnOpenFile, SIGNAL(clicked()), this, SLOT(OpenFile()) );

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


