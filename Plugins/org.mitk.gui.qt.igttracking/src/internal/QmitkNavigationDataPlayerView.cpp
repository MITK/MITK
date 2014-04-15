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
#include <vtkConeSource.h>

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
  mitk::NavigationDataSet::Pointer data;
  mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();

  // FIXME Filter for correct Files
  QString fileName = QFileDialog::getOpenFileName(NULL, "Open Navigation Data Set", "", "");
  data = reader->Read(fileName.toStdString());

  // Update Labels
  m_Controls->m_LblFilePath->setText(fileName);
  m_Controls->m_LblFrames->setText(QString::number(data->Size()));
  m_Controls->m_LblTools->setText(QString::number(data->GetNumberOfTools()));

  if (m_Controls->m_RdbSequential->isChecked())
  {
    mitk::NavigationDataSequentialPlayer::Pointer seqPlayer = mitk::NavigationDataSequentialPlayer::New();
    seqPlayer->SetNavigationDataSet(data);
    m_Controls->m_SequentialWidget->SetPlayer(seqPlayer);
    m_Player = seqPlayer;
  } else {
  }

  CreatePipeline(m_Player);
}

void QmitkNavigationDataPlayerView::OnSelectPlayer(){
  if (m_Controls->m_RdbSequential->isChecked())
  {
    m_Controls->m_SequentialWidget->setVisible(true);
    m_Controls->m_TimedWidget->setVisible(false);
  } else {
    m_Controls->m_SequentialWidget->setVisible(false);
    m_Controls->m_TimedWidget->setVisible(true);
  }
}

void ConfigurePlayer(mitk::NavigationDataPlayerBase::Pointer player){
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
  for (unsigned int i = 0; i < m_RenderingNodes.size(); i++){
    m_RenderingNodes[i]->Update();
  }
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNavigationDataPlayerView::OnSetDisplay(){
  DestroyPipeline();
  if ( (m_Controls->m_ChkDisplay->isChecked()) && ( m_Player.IsNotNull() ))
  {
    CreatePipeline(m_Player);
  }
}

void QmitkNavigationDataPlayerView::CreatePipeline(mitk::NavigationDataPlayerBase::Pointer player){
  m_VisFilter = mitk::NavigationDataObjectVisualizationFilter::New();

  for (unsigned int i = 0 ; i < player->GetNumberOfIndexedOutputs(); i++ ) {
    mitk::DataNode::Pointer node = mitk::DataNode::New();
    // TODO Give Nodes a proper name
    m_VisFilter->SetInput(i, player->GetOutput(i));

    //create small sphere and use it as surface
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkConeSource *vtkData = vtkConeSource::New();
    vtkData->SetAngle(5.0);
    vtkData->SetResolution(50);
    vtkData->SetHeight(6.0f);
    vtkData->SetRadius(2.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    vtkData->Delete();
    node->SetData(mySphere);
    m_VisFilter->SetRepresentationObject(i, mySphere);
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