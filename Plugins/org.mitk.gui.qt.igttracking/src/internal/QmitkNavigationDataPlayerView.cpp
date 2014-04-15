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
}

void QmitkNavigationDataPlayerView::OnPlayingStarted()
{
}

void QmitkNavigationDataPlayerView::OnOpenFile(){
  mitk::NavigationDataSet::Pointer data;
  mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
  mitk::NavigationDataSequentialPlayer::Pointer seqPlayer = mitk::NavigationDataSequentialPlayer::New();

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
    m_Controls->m_SequentialWidget->SetPlayer(seqPlayer);
  } else {
  }
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

void QmitkNavigationDataPlayerView::OnSetRepeat(){
}

void QmitkNavigationDataPlayerView::OnSetMicroservice(){
}

void QmitkNavigationDataPlayerView::OnSetDisplay(){
}