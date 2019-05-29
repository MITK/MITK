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

// Qmitk
#include "QmitkNavigationDataPlayerView.h"

// QT
#include <QFileDialog>
#include <QMessageBox>

//mitk
#include <mitkNavigationDataSet.h>
#include <mitkNavigationDataReaderInterface.h>
#include <mitkNavigationDataSequentialPlayer.h>
#include <mitkNavigationDataPlayer.h>
#include <mitkVirtualTrackingTool.h>
#include <mitkIOUtil.h>

// VTK
#include <vtkSphereSource.h>

const std::string QmitkNavigationDataPlayerView::VIEW_ID = "org.mitk.views.navigationdataplayer";

QmitkNavigationDataPlayerView::QmitkNavigationDataPlayerView()
  : m_Controls( 0 )
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

    this->CreateConnections();

    // make deselected Player invisible
    m_Controls->m_TimedWidget->setVisible(false);
  }
}

void QmitkNavigationDataPlayerView::SetFocus()
{
  if ( m_Controls )
  {
    m_Controls->m_grpbxControls->setFocus();
  }
}

void QmitkNavigationDataPlayerView::CreateConnections()
{
  connect( m_Controls->m_RdbSequential, SIGNAL(released()), this, SLOT(OnSelectPlayer()) );
  connect( m_Controls->m_RdbTimeBased, SIGNAL(released()), this, SLOT(OnSelectPlayer()) );
  connect( m_Controls->m_BtnOpenFile, SIGNAL(released()), this, SLOT(OnOpenFile()) );
  connect( m_Controls->m_ChkDisplay, SIGNAL(released()), this, SLOT(OnSetDisplay()) );
  connect( m_Controls->m_chkRepeat, SIGNAL(stateChanged(int)), this, SLOT(OnSetRepeat(int)) );
  connect( m_Controls->m_ChkMicroservice, SIGNAL(released()), this, SLOT(OnSetMicroservice()) );

  connect( m_Controls->m_SequentialWidget, SIGNAL(SignalUpdate()), this, SLOT(OnUpdate()) );
  connect( m_Controls->m_TimedWidget, SIGNAL(SignalUpdate()), this, SLOT(OnUpdate()) );

  this->SetInteractionComponentsEnabledState(false);
}

void QmitkNavigationDataPlayerView::OnOpenFile()
{
  mitk::NavigationDataReaderInterface::Pointer reader = nullptr;

  QString filter = tr("NavigationData File (*.csv *.xml)");

  QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open NavigationData Set"), "", filter);

  if ( fileName.isNull() ) { return; } // user pressed cancel

  try
  {
    m_Data = dynamic_cast<mitk::NavigationDataSet*> (mitk::IOUtil::Load(fileName.toStdString())[0].GetPointer());
  }
  catch ( const mitk::Exception &e )
  {
    MITK_WARN("NavigationDataPlayerView") << "could not open file " << fileName.toStdString();
    QMessageBox::critical(0, "Error Reading File", "The file '" + fileName
                          +"' could not be read.\n" + e.GetDescription() );
    return;
  }

  if (m_Controls->m_ChkConvertToPointSet->isChecked())
    m_Data->ConvertNavigationDataToPointSet();

  // Update Labels
  m_Controls->m_LblFilePath->setText(fileName);
  m_Controls->m_LblFrames->setText(QString::number(m_Data->Size()));
  m_Controls->m_LblTools->setText(QString::number(m_Data->GetNumberOfTools()));

  // Initialize Widgets and create Player
  this->OnSelectPlayer();
  this->SetInteractionComponentsEnabledState(true);
}

void QmitkNavigationDataPlayerView::OnSelectPlayer()
{
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

  this->ConfigurePlayer();

  // SetupRenderingPipeline
  this->OnSetDisplay();
}

void QmitkNavigationDataPlayerView::ConfigurePlayer()
{
  // set repeat mode according to the checkbox
  m_Player->SetRepeat( m_Controls->m_chkRepeat->isChecked() );
}

void QmitkNavigationDataPlayerView::OnSetRepeat(int checkState)
{
  m_Player->SetRepeat(checkState != 0);
}

void QmitkNavigationDataPlayerView::OnSetMicroservice(){
  if(m_Controls->m_ChkMicroservice->isChecked())
  {
    m_ToolStorage = mitk::NavigationToolStorage::New();
    for (itk::ProcessObject::DataObjectPointerArraySizeType i = 0;
         i < m_Player->GetNumberOfIndexedOutputs(); i++)
    {
      mitk::NavigationTool::Pointer currentDummyTool = mitk::NavigationTool::New();
      mitk::VirtualTrackingTool::Pointer dummyTool = mitk::VirtualTrackingTool::New();
      std::stringstream name;
      name << "Virtual Tool " << i;
      dummyTool->SetToolName(name.str());
      currentDummyTool->SetDataNode(m_RenderingNodes.at(i));
      currentDummyTool->SetIdentifier(name.str());
      m_ToolStorage->AddTool(currentDummyTool);
    }
    m_ToolStorage->SetName("NavigationDataPlayer Tool Storage");
    m_Player->SetToolMetaDataCollection(m_ToolStorage);
    m_Player->RegisterAsMicroservice();
    m_ToolStorage->SetSourceID(m_Player->GetMicroserviceID()); //DEPRECATED / not needed anymore because NavigationDataSource now holds a member of its tool storage. Only left for backward compatibility.
    m_ToolStorage->RegisterAsMicroservice();
  } else {
    if (m_ToolStorage.IsNotNull()) m_ToolStorage->UnRegisterMicroservice();
    m_ToolStorage = nullptr;
    m_Player->UnRegisterMicroservice();
  }
}

void QmitkNavigationDataPlayerView::OnUpdate(){
  if (m_VisFilter.IsNotNull())
  {
    m_VisFilter->Update();
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
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
    QString name = "Recorded Tool " + QString::number(i + 1);
    node->SetName(name.toStdString());

    //create small sphere and use it as surface
    mitk::Surface::Pointer mySphere = mitk::Surface::New();
    vtkSmartPointer<vtkSphereSource> vtkData = vtkSmartPointer<vtkSphereSource>::New();
    vtkData->SetRadius(5.0f);
    vtkData->SetCenter(0.0, 0.0, 0.0);
    vtkData->Update();
    mySphere->SetVtkPolyData(vtkData->GetOutput());
    node->SetData(mySphere);
    m_VisFilter->SetRepresentationObject(i, mySphere);

    // Add Node to DataStorageand to local list of Nodes
    GetDataStorage()->Add(node);
    m_RenderingNodes.push_back(node);
  }
  m_VisFilter->Update();

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(GetDataStorage());
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkNavigationDataPlayerView::DestroyPipeline(){
  m_VisFilter = nullptr;
  for (unsigned int i = 0; i < m_RenderingNodes.size(); i++){
    this->GetDataStorage()->Remove(m_RenderingNodes[i]);
  }
  m_RenderingNodes.clear();
}

void QmitkNavigationDataPlayerView::SetInteractionComponentsEnabledState(bool isActive){
  m_Controls->m_grpbxSettings->setEnabled(isActive);
  m_Controls->m_grpbxControls->setEnabled(isActive);
}
