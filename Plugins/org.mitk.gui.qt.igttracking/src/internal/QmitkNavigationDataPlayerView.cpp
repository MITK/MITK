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
#include "QmitkNavigationDataPlayerView.h"
#include "QmitkStdMultiWidget.h"

// QT
#include <QFileDialog>

//mitk
#include <mitkNavigationDataSet.h>
#include <mitkNavigationDataReaderXML.h>
#include <mitkNavigationDataSequentialPlayer.h>
#include <mitkNavigationDataPlayer.h>

// VTK
#include <vtkSphereSource.h>

const std::string QmitkNavigationDataPlayerView::VIEW_ID = "org.mitk.views.navigationdataplayer";

QmitkNavigationDataPlayerView::QmitkNavigationDataPlayerView()
  : QmitkFunctionality()
  , m_Controls( 0 )
  , m_MultiWidget( NULL )
{
}

QmitkNavigationDataPlayerView::~QmitkNavigationDataPlayerView()
{
}

void QmitkNavigationDataPlayerView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkNavigationDataPlayerViewControls;
    m_Controls->setupUi( parent );

    this->CreateBundleWidgets( parent );
    this->CreateConnections();

    // make deselected Player invisible
    m_Controls->m_TimedWidget->setVisible(false);
  }
}

void QmitkNavigationDataPlayerView::CreateBundleWidgets(QWidget* parent)
{
  //m_PlayerWidget = new QmitkIGTPlayerWidget( parent );   // this bundle's ND player widget
}

void QmitkNavigationDataPlayerView::CreateConnections()
{
  connect( m_Controls->m_RdbSequential, SIGNAL(released()), this, SLOT(OnSelectPlayer()) );
  connect( m_Controls->m_RdbTimeBased, SIGNAL(released()), this, SLOT(OnSelectPlayer()) );
  connect( m_Controls->m_BtnOpenFile, SIGNAL(released()), this, SLOT(OnOpenFile()) );
  connect( m_Controls->m_ChkDisplay, SIGNAL(released()), this, SLOT(OnSetDisplay()) );
  connect( m_Controls->m_chkRepeat, SIGNAL(released()), this, SLOT(OnSetRepeat()) );
  connect( m_Controls->m_ChkMicroservice, SIGNAL(released()), this, SLOT(OnSetMicroservice()) );

  connect( m_Controls->m_SequentialWidget, SIGNAL(SignalUpdate()), this, SLOT(OnUpdate()) );
  connect( m_Controls->m_TimedWidget, SIGNAL(SignalUpdate()), this, SLOT(OnUpdate()) );
}

void QmitkNavigationDataPlayerView::OnPlayingStarted()
{
}

void QmitkNavigationDataPlayerView::OnOpenFile(){
  mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();

  // FIXME Filter for correct Files and use correct Reader
  QString fileName = QFileDialog::getOpenFileName(NULL, "Open Navigation Data Set", "", "");
  m_Data = reader->Read(fileName.toStdString());

  // Update Labels
  m_Controls->m_LblFilePath->setText(fileName);
  m_Controls->m_LblFrames->setText(QString::number(m_Data->Size()));
  m_Controls->m_LblTools->setText(QString::number(m_Data->GetNumberOfTools()));

  // Initialize Widgets and create Player
  OnSelectPlayer();
}

void QmitkNavigationDataPlayerView::OnSelectPlayer(){
  if (m_Controls->m_RdbSequential->isChecked())
  {
    m_Controls->m_SequentialWidget->setVisible(true);
    m_Controls->m_TimedWidget->setVisible(false);
    mitk::NavigationDataSequentialPlayer::Pointer seqPlayer = mitk::NavigationDataSequentialPlayer::New();
    seqPlayer->SetNavigationDataSet(m_Data);
    m_Controls->m_SequentialWidget->SetPlayer(seqPlayer);
    m_Player = seqPlayer;
  } else {
    m_Controls->m_SequentialWidget->setVisible(false);
    m_Controls->m_TimedWidget->setVisible(true);
    mitk::NavigationDataPlayer::Pointer timedPlayer = mitk::NavigationDataPlayer::New();
    timedPlayer->SetNavigationDataSet(m_Data);
    m_Controls->m_TimedWidget->SetPlayer(timedPlayer);
    m_Player = timedPlayer;
  }

  // SetupRenderingPipeline
  OnSetDisplay();
}

void ConfigurePlayer(){
  // FIXME: Why is repeat not available in the base class?
  // TODO finish method
}

void QmitkNavigationDataPlayerView::OnSetRepeat(){
  MITK_WARN << "Repeat not yet supported";
}

void QmitkNavigationDataPlayerView::OnSetMicroservice(){
  MITK_WARN << "Register as Microservice not yet supported";
}

void QmitkNavigationDataPlayerView::OnUpdate(){
  m_VisFilter->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNavigationDataPlayerView::OnSetDisplay(){
  DestroyPipeline();
  if ( (m_Controls->m_ChkDisplay->isChecked()) && ( m_Player.IsNotNull() ))
  {
    CreatePipeline();
  }
}

void QmitkNavigationDataPlayerView::CreatePipeline(){
  m_VisFilter = mitk::NavigationDataObjectVisualizationFilter::New();
  m_VisFilter->ConnectTo(m_Player);

  for (unsigned int i = 0 ; i < m_Player->GetNumberOfIndexedOutputs(); i++ ) {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    QString name = "Recorded Tool " + QString::number(i);
    node->SetName(name.toStdString());

    //create small sphere and use it as surface
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkSphereSource *vtkData = vtkSphereSource::New();
    vtkData->SetRadius(5.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    vtkData->Delete();
    node->SetData(mySphere);
    m_VisFilter->SetRepresentationObject(i, mySphere);

    // Add Node to DataStorageand to local list of Nodes
    GetDataStorage()->Add(node);
    m_RenderingNodes.push_back(node);
  }
}

void QmitkNavigationDataPlayerView::DestroyPipeline(){
  m_VisFilter = NULL;
  for (unsigned int i = 0; i < m_RenderingNodes.size(); i++){
    this->GetDataStorage()->Remove(m_RenderingNodes[i]);
  }
  m_RenderingNodes.clear();
}