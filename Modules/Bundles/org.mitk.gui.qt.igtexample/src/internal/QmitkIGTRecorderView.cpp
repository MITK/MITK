/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-06 16:03:09 +0200 (Mi, 06 Mai 2009) $
Version:   $Revision: 17113 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QmitkIGTRecorderView.h"

//#include "resources/icon.xpm"

#include "QmitkStdMultiWidget.h"

#include "mitkCone.h"
#include "mitkInternalTrackingTool.h"
#include "mitkNDIPassiveTool.h"
#include "mitkNDITrackingDevice.h"
#include "mitkTrackingVolume.h"

#include "mitkProperties.h"

#include "QmitkNDIConfigurationWidget.h"

#include "mitkNodePredicateProperty.h"
#include "mitkNodePredicateDataType.h"
#include "mitkNodePredicateAND.h"
#include "mitkNodePredicateNOT.h"
#include "mitkGroupTagProperty.h"

#include <itksys/SystemTools.hxx>

#include <QPushButton>
#include <QTextEdit>
#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QFileDialog>
#include <QTime>

#include <vtkTextSource.h>
#include <vtkAppendPolyData.h>
#include <vtkSphereSource.h>
#include <vtkAxes.h>
#include <vtkTubeFilter.h>
#include <vtkConeSource.h>
#include <vtkCamera.h>

#include "mitkStatusBar.h"


QmitkIGTRecorderView::QmitkIGTRecorderView(QObject * /*parent*/, const char * /*name*/)
 : QmitkFunctionality()
{
  m_Timer = new QTimer(this);
  m_RecordingTimer = new QTimer(this);
  m_PlayingTimer = new QTimer(this);
  //m_XValues.clear();
  //m_YValues.clear();
  m_Controls = NULL;
}


QmitkIGTRecorderView::~QmitkIGTRecorderView()
{
  this->OnStopRecording();  // cleanup IGT pipeline, if tracking is in progress
  m_RecordingTimer->stop();
  m_PlayingTimer->stop();
  m_Recorder = NULL;
  //m_Player = NULL;
  m_RecordingTimer = NULL;
  m_PlayingTimer = NULL;
}

void QmitkIGTRecorderView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    m_Controls = new Ui::QmitkIGTRecorderControls;
    m_Controls->setupUi(parent);
    m_ConfigWidget = new QmitkNDIConfigurationWidget(parent);
    mitk::NodePredicateAND::Pointer pred = mitk::NodePredicateAND::New(
      mitk::NodePredicateNOT::New(mitk::NodePredicateProperty::New("already Assigned")), 
      mitk::NodePredicateDataType::New("Surface"));  // does not have assigned tag and is surface
    m_ConfigWidget->SetDataStorage(this->GetDefaultDataStorage());
    m_ConfigWidget->SetPredicate(pred.GetPointer());
    m_ConfigWidget->SetTagPropertyName("already Assigned");
    m_ConfigWidget->SetTagProperty(mitk::GroupTagProperty::New().GetPointer());  // 
    m_ConfigWidget->SetToolTypes(QStringList() << "Instrument");
    m_Controls->m_DeviceConfigGroup->layout()->addWidget(m_ConfigWidget); // add ndi config widget
    

    this->CreateConnections();
  }
}


void QmitkIGTRecorderView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkIGTRecorderView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkIGTRecorderView::CreateConnections()
{
  if ( m_Controls )
  {
    connect((QObject*)(m_Controls->m_StartRecording), SIGNAL(clicked()), (QObject*) this, SLOT(OnStartRecording()));
    connect((QObject*)(m_Controls->m_PauseRecording), SIGNAL(toggled (bool)), (QObject*) this, SLOT(OnPauseRecording(bool)));
    connect((QObject*)(m_Controls->m_StopRecording), SIGNAL(clicked()), (QObject*) this, SLOT(OnStopRecording()));
    connect((QObject*)(m_Controls->m_StartReplay), SIGNAL(clicked()), (QObject*) this, SLOT(OnStartReplaying()));
    connect((QObject*)(m_Controls->m_StopReplay), SIGNAL(clicked()), (QObject*) this, SLOT(OnStopReplaying()));
    connect((QObject*)(m_RecordingTimer), SIGNAL(timeout()), (QObject*) this, SLOT(RecordFrame()));
    connect((QObject*)(m_ConfigWidget), SIGNAL(Connected()), this, SLOT(OnConnect()));
    connect((QObject*)(m_ConfigWidget), SIGNAL(Disconnected()), this, SLOT(OnDisconnect()));
  }
}


void QmitkIGTRecorderView::OnStartRecording()
{
  if (m_Source.IsNotNull())
    if (m_Source->IsTracking())
      return;

  if ((m_ConfigWidget == NULL))
    return; 
  mitk::TrackingDevice::Pointer tracker = m_ConfigWidget->GetTracker();

  QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save tracking data to"),"D:/home/jochen/Versuche/2010.01.25 EM Messungen in der Speiseröhre" /*QDir::homePath()*/, tr("MITK Navigation Data XML logfile(*.xml)"));
  if (fileName.isEmpty())
    return;
// recorder!

  if (this->GetDefaultDataStorage()->GetNamedNode("Tracking Volume") == NULL) // add tracking volume node
  {
    mitk::TrackingVolume::Pointer tv = mitk::TrackingVolume::New();
    tv->SetTrackingDeviceType(tracker->GetType());
    mitk::DataTreeNode::Pointer n = mitk::DataTreeNode::New();
    n->SetData(tv);
    n->SetName("Tracking Volume");
    n->SetOpacity(0.1);
    n->SetColor(0.4, 0.4, 1.0);
    this->GetDefaultDataStorage()->Add(n);
    this->GetActiveStdMultiWidget()->SetWidgetPlanesVisibility(false);
    mitk::RenderingManager::GetInstance()->InitializeViews(this->GetDefaultDataStorage()->ComputeBoundingGeometry3D(this->GetDefaultDataStorage()->GetAll()));  // reset render window geometry to include all objects
  }
  try
  {
    this->SetupIGTPipeline(tracker, fileName);
    //m_Source->Connect();
    m_Source->StartTracking();
    m_RecordingTimer->start(50);
  }
  catch (std::exception& e)
  {
    mitk::StatusBar::GetInstance()->DisplayText(QString("Error during navigation pipeline setup: %1").arg(e.what()).toLatin1(), 4000);
    m_RecordingTimer->stop();
  }
}


void QmitkIGTRecorderView::OnStopRecording()
{
  if (m_RecordingTimer != NULL)
    m_RecordingTimer->stop();
  if (m_Recorder.IsNotNull())
    m_Recorder->StopRecording();
  if (m_Source.IsNotNull())
  {
    m_Source->StopTracking();
    //m_Source->Disconnect();
  }  
  m_MessageFilter = NULL;
  m_PointSetFilter = NULL;
  m_Recorder = NULL;
  m_Visualizer = NULL;
  //m_Source = NULL;
}


void QmitkIGTRecorderView::OnPauseRecording( bool pause )
{
  if (pause == true)
    m_RecordingTimer->stop();
  else
    m_RecordingTimer->start(50);
}


void QmitkIGTRecorderView::RecordFrame()
{
 
  try
  {
    bool updated = false;
    if ((m_Controls->m_UpdateTrajectory->checkState() == Qt::Checked) && (m_PointSetFilter.IsNotNull()))
      m_PointSetFilter->Update(); // update pipeline
    else
      updated |= false;
    if ((m_Controls->m_UpdateRecorder->checkState() == Qt::Checked) && (m_Recorder.IsNotNull()))
      m_Recorder->Update(); // record data to file
    else
      updated |= false;
    if ((updated == false) && (m_Recorder.IsNotNull()))
      m_Visualizer->Update();

    mitk::RenderingManager::GetInstance()->RequestUpdate(this->GetActiveStdMultiWidget()->GetRenderWindow4()->GetRenderWindow()); // render 3D scene
  }
  catch (...)
  {
    mitk::StatusBar::GetInstance()->DisplayText("Error during navigation pipeline update", 1000);
  }
}


void QmitkIGTRecorderView::OnStartReplaying()
{

}


void QmitkIGTRecorderView::OnStopReplaying()
{

}


void QmitkIGTRecorderView::SetupIGTPipeline(mitk::TrackingDevice::Pointer tracker, QString fileName)
{
  mitk::DataStorage* ds = this->GetDefaultDataStorage();
  if (ds == NULL)
    throw std::invalid_argument("DataStorage not available");
  if (tracker.IsNull())
    throw std::invalid_argument("invalid tracking device provided.");

  
  if (m_ConfigWidget->GetToolsByToolType("Instrument").isEmpty())
    throw std::invalid_argument("insufficient tool assignments for instrument");

  /* build pipeline filters */
  if (m_Source.IsNull())
  {
    m_Source = mitk::TrackingDeviceSource::New();
    m_Source->SetTrackingDevice(tracker);
  }
  m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
  m_MessageFilter = mitk::NavigationDataToMessageFilter::New();
  m_PointSetFilter = mitk::NavigationDataToPointSetFilter::New();
  m_PointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode3D);
  m_Recorder = mitk::NavigationDataRecorder::New();
  m_Recorder->SetFileName(fileName.toLatin1());

  /* connect inputs and outputs of filters */
  for (unsigned int i = 0; i < m_Source->GetNumberOfOutputs(); ++i)   // source --> visualizer --> messager --> pointset
  {
    /* visualization filter */
    m_Visualizer->SetInput(i, m_Source->GetOutput(i));
    /* message filter */
    m_MessageFilter->SetInput(i, m_Visualizer->GetOutput(i));  
    /* landmark transform filter */
    m_PointSetFilter->SetInput(i, m_MessageFilter->GetOutput(i));
    m_Recorder->AddNavigationData(m_MessageFilter->GetOutput(i));
  }
  m_Recorder->StartRecording();
  /* set filter parameters */
  /* instrument visualization */
  const QList<unsigned int> instruments = m_ConfigWidget->GetToolsByToolType("Instrument");
  foreach (const unsigned int& index, instruments)
  {
    mitk::DataTreeNode::Pointer node = this->CreateInstrumentVisualization(m_Source->GetOutput(index)->GetName());
    ds->Add(node);      
    m_Visualizer->SetRepresentationObject(index, node->GetData());
  }
  for (unsigned int i = 0; i < m_PointSetFilter->GetNumberOfOutputs(); i++)
  {

    mitk::PointSet* p = m_PointSetFilter->GetOutput(i);
    assert(p);

    mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
    pointSetNode->SetData(p);
    pointSetNode->SetName(QString("Trajectory of Tool #%1 (%2)").arg(i).arg(QTime::currentTime().toString()).toLatin1());
    mitk::Color color;
    color.Set(0.2, 0.3 * i ,0.9 - 0.2 * i);
    pointSetNode->SetColor(color); //change color of points
    pointSetNode->SetProperty("contourcolor", mitk::ColorProperty::New(color)); // change color of trajectory line
    pointSetNode->SetProperty("pointsize", mitk::FloatProperty::New(1.0)); // enlarge visualization of points
    pointSetNode->SetProperty("contoursize", mitk::FloatProperty::New(0.5)); // enlarge visualization of trajectory line
    pointSetNode->SetBoolProperty("show contour", true);
    pointSetNode->SetBoolProperty("updateDataOnRender", false); // do not call Update() on the pointset during render (this would cause a execution of the pipeline that is still connected to the pointset)
    this->GetDefaultDataStorage()->Add(pointSetNode); //add it to the DataStorage
  }
}

mitk::DataTreeNode::Pointer QmitkIGTRecorderView::CreateInstrumentVisualization(const char* label)
{
  vtkAxes* axes = vtkAxes::New();
  axes->SymmetricOn();
  axes->SetScaleFactor(10.0); 
  vtkTubeFilter* tuber = vtkTubeFilter::New();
  tuber->SetRadius(0.02);
  tuber->SetNumberOfSides(6);
  tuber->SetInputConnection(axes->GetOutputPort());
  vtkTextSource* tss = vtkTextSource::New();
  tss->SetText(label);
  tss->BackingOff();
  vtkConeSource* cone = vtkConeSource::New();
  cone->SetDirection(0.0, 0.0, -1.0); // direction: along z-axis, facing towards -z direction
  cone->SetCenter(0.0, 0.0, 100.0);
  cone->SetHeight(200.0);              // center at 100, height 200 should set the tip of the cone to the origin
  cone->SetRadius(10.0);
  vtkAppendPolyData* ap = vtkAppendPolyData::New();
  //ap->AddInput(tss->GetOutput());
  ap->AddInput(tuber->GetOutput());
  //ap->AddInput(cone->GetOutput());
  ap->GetOutput()->Update();
  mitk::Surface::Pointer dummy = mitk::Surface::New();
  dummy->SetVtkPolyData(ap->GetOutput());
  ap->Delete(); 
  cone->Delete();
  tss->Delete();
  tuber->Delete();
  axes->Delete();

  mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
  node->SetData(dummy);
  node->SetName(label);
  node->SetColor(0.2, 0.9, 0.2);
  //node->SetBoolProperty("helperObject", true);
  return node;
}


void QmitkIGTRecorderView::OnConnect()
{
  m_Controls->m_StartRecording->setEnabled(true);
  m_Controls->m_StopRecording->setEnabled(true);
  m_Controls->m_PauseRecording->setEnabled(true);
  //m_Controls->m_StartReplay->setEnabled(true);
  //m_Controls->m_StopReplay->setEnabled(true);

}


void QmitkIGTRecorderView::OnDisconnect()
{
  this->OnStopRecording();
  m_Controls->m_StartRecording->setEnabled(false);
  m_Controls->m_StopRecording->setEnabled(false);
  m_Controls->m_PauseRecording->setEnabled(false);
}
