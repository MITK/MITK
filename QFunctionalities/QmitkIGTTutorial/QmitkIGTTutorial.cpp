/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkIGTTutorial.h"
#include "QmitkIGTTutorialControls.h"
#include <qaction.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"
#include "QmitkStdMultiWidget.h"
#include "mitkStatusBar.h"
#include "mitkProgressBar.h"

#include "mitkNDIPassiveTool.h"
#include "mitkNDITrackingDevice.h"
#include "mitkStandardFileLocations.h"
#include "mitkSerialCommunication.h"
#include "mitkCone.h"

#include "qtimer.h"

QmitkIGTTutorial::QmitkIGTTutorial(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL),
 m_Source(NULL), m_Visualizer(NULL), m_Timer(NULL)
{
  SetAvailability(true);
}


QmitkIGTTutorial::~QmitkIGTTutorial()
{}


QWidget * QmitkIGTTutorial::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}


QWidget * QmitkIGTTutorial::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkIGTTutorialControls(parent);
  }
  return m_Controls;
}


void QmitkIGTTutorial::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StartButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnDoIGT()));
    connect( (QObject*)(m_Controls->m_StopButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStop()));
  }
}


QAction * QmitkIGTTutorial::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Tutorial functionality for MITK-IGT" ), QPixmap((const char**)icon_xpm), tr( "QmitkIGTTutorial menu" ), 0, parent, "QmitkIGTTutorial" );
  return action;
}


void QmitkIGTTutorial::TreeChanged()
{
}


void QmitkIGTTutorial::Activated()
{
  QmitkFunctionality::Activated();
}



void QmitkIGTTutorial::OnDoIGT() 
{
  try
  {
    mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
    tracker->SetPortNumber(mitk::SerialCommunication::COM4);
    tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200);
    tracker->SetType(mitk::NDIPolaris);

    mitk::NDIPassiveTool::Pointer tool = mitk::NDIPassiveTool::New();
    tool->SetToolName("MyInstrument");
    tool->LoadSROMFile("c:\\myinstrument.rom");
    //tool->LoadSROMFile(mitk::StandardFileLocations::GetInstance()->FindFile("myToolDefinitionFile.srom").c_str());
    tracker->Add6DTool(tool);

    m_Source = mitk::TrackingDeviceSource::New();  // we need the filter objects to stay alive, therefore they must be members
    m_Source->SetTrackingDevice(tracker);

    m_Source->Connect();


    mitk::Cone::Pointer cone = mitk::Cone::New();
    float scale[] = {10.0, 10.0, 10.0};
    cone->GetGeometry()->SetSpacing(scale);
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData(cone);
    node->SetName("My tracked object");
    node->SetColor(1.0, 0.0, 0.0); //red
    mitk::DataStorage::GetInstance()->Add(node);

    m_Visualizer = mitk::NavigationDataVisualizationByBaseDataTransformFilter::New();
    m_Visualizer->SetInput(0, m_Source->GetOutput(0));
    m_Visualizer->SetBaseData(m_Source->GetOutput(0), cone);

    //start the tracking
    m_Source->StartTracking();

    if (m_Timer == NULL)
    {
      m_Timer = new QTimer(this);
    }    
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
    m_Timer->start(100); // 100ms -> 10fps
  }
  catch (std::exception& e)
  {
    // add cleanup
    std::cout << "Error in QmitkIGTTutorial::OnDoIGT():" << e.what() << std::endl;
  }
}


void QmitkIGTTutorial::OnTimer()
{
  m_Visualizer->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


void QmitkIGTTutorial::OnStop()
{
  m_Timer->stop();
  disconnect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
  m_Timer = NULL;
  m_Source->StopTracking();
  m_Source->Disconnect();
  m_Source = NULL;
  m_Visualizer = NULL;
  m_Source = NULL;
  mitk::DataStorage::GetInstance()->Remove(mitk::DataStorage::GetInstance()->GetNamedNode("My tracked object"));
}
