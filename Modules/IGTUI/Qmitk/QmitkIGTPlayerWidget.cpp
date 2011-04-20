/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:14:45 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkIGTPlayerWidget.h"

//mitk headers
#include "mitkTrackingTypes.h"
#include <mitkSTLFileReader.h>
#include <mitkSurface.h>
#include <mitkNavigationToolReader.h>
#include <mitkNavigationToolWriter.h>
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkNavigationToolStorageSerializer.h>




//qt headers
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtimer.h>


QmitkIGTPlayerWidget::QmitkIGTPlayerWidget(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f), m_Player(NULL), m_StartTime(-1.0)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  this->ResetLCDNumbers();
}


QmitkIGTPlayerWidget::~QmitkIGTPlayerWidget()
{ 
  m_PlayingTimer->stop();
  m_Player = NULL;  
  m_PlayingTimer = NULL;
}

void QmitkIGTPlayerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTPlayerWidgetControls;                         
    m_Controls->setupUi(parent);

    m_PlayingTimer = new QTimer(this); // initialize update timer

   }

}

void QmitkIGTPlayerWidget::CreateConnections()
{
  if ( m_Controls )
  {     
    connect( (QObject*)(m_Controls->selectPushButton), SIGNAL(clicked()), this, SLOT(OnSelectPressed()) ); // open file dialog
    //connect( (QObject*) (m_Controls->m_cbPointSetMode), SIGNAL(clicked(bool)), this, SLOT(OnChangeWidgetView(bool)) ); // widget view switcher

    connect( (QObject*)(m_Controls->playPushButton), SIGNAL(clicked(bool)), this, SLOT(OnPlayButtonClicked(bool)) ); // play button
    connect( (QObject*)(m_PlayingTimer), SIGNAL(timeout()), this, SLOT(OnPlaying()) ); // update timer

    connect( (QObject*) (m_Controls->beginPushButton), SIGNAL(clicked()), this, SLOT(OnGoToBegin()) ); // reset player and go to begin
    connect( (QObject*) (m_Controls->stopPushButton), SIGNAL(clicked()), this, SLOT(OnGoToEnd()) ); // reset player

    // passing signal from ui component
    connect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SLOT(SignalCurrentTrajectoryChanged(int)) );
  }
}



bool QmitkIGTPlayerWidget::CheckInputFileValid()
{
  QFile file(m_CmpFilename);

  if(!file.exists())
  {
    QMessageBox::warning(NULL, "IGTPlayer: Error", "No valid input file was loaded. Please load input file first!");
    return false;
  }

  return true;
}


unsigned int QmitkIGTPlayerWidget::GetNumberOfTools()
{
  unsigned int result = 0;
  
  if(m_Player.IsNotNull())
    result = m_Player->GetNumberOfOutputs();

  return result;
}


void QmitkIGTPlayerWidget::SetUpdateRate(unsigned int msecs)
{
  m_PlayingTimer->setInterval((int) msecs);
}

void QmitkIGTPlayerWidget::OnPlayButtonClicked(bool checked)
{
  if(CheckInputFileValid())  // no playing possible without valid input file
  {
    if(checked) // play
    {
      if(m_Player.IsNull())  // start play
      {
        m_Player = mitk::NavigationDataPlayer::New();
        m_Player->SetFileName(m_CmpFilename.toStdString());

        m_Player->StartPlaying();
        m_PlayingTimer->start(100);

        emit PlayingStarted();
      }
      else // resume play
      {
        m_Player->Resume();  
        m_PlayingTimer->start(100);
        emit PlayingResumed();
      }
    }

    else // pause
    {
      m_Player->Pause();
      m_PlayingTimer->stop();
      emit PlayingPaused();
    }
  }

  else
    m_Controls->playPushButton->setChecked(false); // uncheck play button if file unvalid
}

QTimer*  QmitkIGTPlayerWidget::GetPlayingTimer()
{
  return m_PlayingTimer;
}

void QmitkIGTPlayerWidget::OnStopPlaying()
{
  this->StopPlaying();
}


void QmitkIGTPlayerWidget::StopPlaying()
{
  m_PlayingTimer->stop();
  emit PlayingStopped();
  if(m_Player.IsNotNull())
    m_Player->StopPlaying();
  m_Player = NULL;
  m_StartTime = -1;  // set starttime back
  this->ResetLCDNumbers();
  m_Controls->playPushButton->setChecked(false); // set play button unchecked


}

void QmitkIGTPlayerWidget::OnPlaying()
{
  if(m_Player.IsNull())
    return;

  if(m_StartTime < 0)
    m_StartTime = m_Player->GetOutput()->GetTimeStamp(); // get playback start time

  if(!m_Player->IsAtEnd())
  {
    m_Player->Update(); // update player

    int msc = (int) (m_Player->GetOutput()->GetTimeStamp() - m_StartTime);

    // calculation for playing time display
    int ms = msc % 1000;
    msc = (msc - ms) / 1000;
    int s = msc % 60;
    int min = (msc-s) / 60;

    // set lcd numbers
    m_Controls->msecLCDNumber->display(ms);
    m_Controls->secLCDNumber->display(s);
    m_Controls->minLCDNumber->display(min);    

    emit PlayerUpdated(); // player successfully updated
  }
  else
    this->StopPlaying(); // if player is at EOF

}

const std::vector<mitk::NavigationData::Pointer> QmitkIGTPlayerWidget::GetNavigationDatas()
{
  std::vector<mitk::NavigationData::Pointer> navDatas;

  if(m_Player.IsNotNull())
  {
    for(unsigned int i=0; i < m_Player->GetNumberOfOutputs(); ++i)
    {
      navDatas.push_back(m_Player->GetOutput(i));
    }
  } 
 
  return navDatas;    
}

const mitk::PointSet::Pointer QmitkIGTPlayerWidget::GetNavigationDatasPointSet()
{
  mitk::PointSet::Pointer result = mitk::PointSet::New();
  
  mitk::PointSet::PointType pointType;

  if(m_Player.IsNotNull())
  {
    for(unsigned int i=0; i < m_Player->GetNumberOfOutputs(); ++i)
    {
      mitk::NavigationData::PositionType position = m_Player->GetOutput(i)->GetPosition();
      
      pointType[0] = position[0];
      pointType[1] = position[1];
      pointType[2] = position[2];

      result->InsertPoint(i,pointType);
    }
  }

  return result;
}

const mitk::PointSet::PointType QmitkIGTPlayerWidget::GetNavigationDataPoint(unsigned int index)
{
  if( index > this->GetNumberOfTools() || index < 0 )
    throw std::out_of_range("Tool Index out of range!");

  mitk::PointSet::PointType result;

  if(m_Player.IsNotNull())
  {
    mitk::NavigationData::PositionType position = m_Player->GetOutput(index)->GetPosition();

    result[0] = position[0];
    result[1] = position[1];
    result[2] = position[2];
  }

  return result;  
}


void QmitkIGTPlayerWidget::SetInputFileName(const QString& inputFileName)
{
  this->OnGoToEnd(); /// stops playing and resets lcd numbers

  QString oldName = m_CmpFilename;
  m_CmpFilename.clear();

  m_CmpFilename = inputFileName;

  QFile file(m_CmpFilename);
  if(m_CmpFilename.isEmpty() || !file.exists())  
  {    
    QMessageBox::warning(NULL, "Warning", QString("Please enter valid path! Using previous path again."));
    m_CmpFilename=oldName;
    m_Controls->inputFileLineEdit->setText(m_CmpFilename);
  }
}

void QmitkIGTPlayerWidget::SetPlayer( mitk::NavigationDataPlayer::Pointer player )
{
  if(player.IsNotNull())
    m_Player = player;
}

void QmitkIGTPlayerWidget::OnSelectPressed()
{
  

  QString oldName = m_CmpFilename;
  m_CmpFilename.clear();
  m_CmpFilename = QFileDialog::getOpenFileName(this, "Load tracking data", QDir::currentPath(),"XML files (*.xml)");

  if (m_CmpFilename.isEmpty())//if something went wrong or user pressed cancel in the save dialog
    m_CmpFilename=oldName;
  else
  {
    this->OnGoToEnd(); /// stops playing and resets lcd numbers
    emit InputFileChanged();
  }

  m_Controls->inputFileLineEdit->setText(m_CmpFilename);
}


void QmitkIGTPlayerWidget::OnGoToEnd()
{
  this->StopPlaying(); 
  
  // reset lcd numbers 
  this->ResetLCDNumbers();


}


void QmitkIGTPlayerWidget::OnGoToBegin()
{
  // stop player manual so no PlayingStopped()
  m_PlayingTimer->stop();
  if(m_Player.IsNotNull())
    m_Player->StopPlaying();
  m_Player = NULL;  // set player to NULL so it can be initialized again if playback is called afterwards
  m_StartTime = -1;  // set starttime back

  //reset view elements
  m_Controls->playPushButton->setChecked(false);
  this->ResetLCDNumbers();
 
}

void QmitkIGTPlayerWidget::ResetLCDNumbers()
{
    m_Controls->minLCDNumber->display(QString("00"));
    m_Controls->secLCDNumber->display(QString("00"));
    m_Controls->msecLCDNumber->display(QString("000")); 
}



void QmitkIGTPlayerWidget::SetTrajectoryNames(const QStringList toolNames)
{ 
  QComboBox* cBox = m_Controls->trajectorySelectComboBox;
 
  if(cBox->count() > 0)
    this->ClearTrajectorySelectCombobox();
    
  disconnect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );

  if(!toolNames.isEmpty())
    m_Controls->trajectorySelectComboBox->insertItems(0, toolNames);

  connect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );

}


int QmitkIGTPlayerWidget::GetResolution()
{
   return m_Controls->resolutionSpinBox->value();
}

void QmitkIGTPlayerWidget::ClearTrajectorySelectCombobox()
{
  disconnect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );
  
  m_Controls->trajectorySelectComboBox->clear();
  
  connect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );
}


