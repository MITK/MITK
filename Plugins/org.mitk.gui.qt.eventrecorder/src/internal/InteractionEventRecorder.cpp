/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
  MITK_INFO << "Performing Reinit";
  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

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
  for (std::size_t i=0; i < events.size(); ++i)
  {
    //this->GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer()->GetDispatcher()->ProcessEvent(events.at(i));
    events.at(i)->GetSender()->GetDispatcher()->ProcessEvent(events.at(i));
  }
  MITK_INFO << "DONE";

}

void InteractionEventRecorder::OpenFile()
{
  QString fn = QFileDialog::getOpenFileName(nullptr, "Open File...",
                                            QString(), "All Files (*)");
  if (!fn.isEmpty())
    this->m_Controls.textFileName->setText(fn);
}

void InteractionEventRecorder::RotatePlanes()
{
  mitk::Point3D center;
  center.Fill(0);
  mitk::Vector3D firstVec;
  mitk::Vector3D secondVec;

  firstVec[0] = 1.0;
  firstVec[1] = .5;
  firstVec[2] = .25;

  secondVec[0] = 1;
  secondVec[1] = .25;
  secondVec[2] = 1;

  // Rotate Planes to a predefined state which can later be used again in tests
  this->GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetSliceNavigationController()->ReorientSlices( center, firstVec,secondVec );
  this->GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer()->RequestUpdate();

}

void InteractionEventRecorder::RotateView()
{
  // Rotate the view in 3D to a predefined state which can later be used again in tests
  // this simulates a prior VTK Interaction

  vtkRenderWindow* renderWindow = mitk::BaseRenderer::GetRenderWindowByName("stdmulti.widget3");

  if (renderWindow == nullptr)
    return;

  mitk::Stepper* stepper = mitk::BaseRenderer::GetInstance(renderWindow)->GetCameraRotationController()->GetSlice();

  if (stepper == nullptr)
    return;

  unsigned int newPos = 17;


  stepper->SetPos(newPos);

  this->GetRenderWindowPart()->GetQmitkRenderWindow("axial")->GetRenderer()->RequestUpdate();
}

void InteractionEventRecorder::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.btnStopRecording, SIGNAL(clicked()), this, SLOT(StopRecording()) );
  connect( m_Controls.btnStartRecording, SIGNAL(clicked()), this, SLOT(StartRecording()) );
  connect( m_Controls.btnPlay, SIGNAL(clicked()), this, SLOT(Play()) );
  connect( m_Controls.btnOpenFile, SIGNAL(clicked()), this, SLOT(OpenFile()) );
  connect( m_Controls.rotatePlanes, SIGNAL(clicked()), this, SLOT(RotatePlanes()) );
  connect( m_Controls.rotate3D, SIGNAL(clicked()), this, SLOT(RotateView()) );

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


