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
#include <mitkIGTException.h>
#include "mitkNavigationDataReaderXML.h"

//qt headers
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtimer.h>

QmitkIGTPlayerWidget::QmitkIGTPlayerWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f),
  m_RealTimePlayer(mitk::NavigationDataPlayer::New()),
  m_SequentialPlayer(mitk::NavigationDataSequentialPlayer::New()),
  m_StartTime(-1.0),
  m_CurrentSequentialPointNumber(0),
  m_Controls(new Ui::QmitkIGTPlayerWidgetControls)
{
  m_Controls->setupUi(this);
  m_PlayingTimer = new QTimer(this); // initialize update timer

  CreateConnections();

  m_Controls->samplePositionHorizontalSlider->setVisible(false);

  this->ResetLCDNumbers(); // reset lcd numbers at start
}

QmitkIGTPlayerWidget::~QmitkIGTPlayerWidget()
{
  m_PlayingTimer->stop();

  delete m_Controls;
}

void QmitkIGTPlayerWidget::CreateConnections()
{
  connect( (QObject*)(m_Controls->playPushButton), SIGNAL(clicked(bool)), this, SLOT(OnPlayButtonClicked(bool)) ); // play button
  connect( (QObject*)(m_PlayingTimer), SIGNAL(timeout()), this, SLOT(OnPlaying()) ); // update timer
  connect( (QObject*) (m_Controls->beginPushButton), SIGNAL(clicked()), this, SLOT(OnGoToBegin()) ); // reset player and go to begin
  connect( (QObject*) (m_Controls->stopPushButton), SIGNAL(clicked()), this, SLOT(OnGoToEnd()) ); // reset player
  // pass this widgets protected combobox signal to public signal
  connect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );
  // pass this widgets protected checkbox signal to public signal
  connect( m_Controls->splineModeCheckBox, SIGNAL(toggled(bool)), this, SIGNAL(SignalSplineModeToggled(bool)) );
  //connect( m_Controls->sequencialModeCheckBox, SIGNAL(toggled(bool)), this, SLOT(OnSequencialModeToggled(bool)) );

  connect( m_Controls->samplePositionHorizontalSlider, SIGNAL(sliderPressed()), this, SLOT(OnSliderPressed()) );
  connect( m_Controls->samplePositionHorizontalSlider, SIGNAL(sliderReleased()), this, SLOT(OnSliderReleased()) );

  connect( m_Controls->m_OpenFileButton, SIGNAL(clicked()), this, SLOT(OnOpenFileButtonPressed()) );
}

bool QmitkIGTPlayerWidget::IsTrajectoryInSplineMode()
{
  return m_Controls->splineModeCheckBox->isChecked();
}

bool QmitkIGTPlayerWidget::CheckInputFileValid()
{
  QFile file(m_CmpFilename);

  // check if file exists
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

  if(this->GetCurrentPlaybackMode() == RealTimeMode)
  {
    if(m_RealTimePlayer.IsNotNull())
      result = m_RealTimePlayer->GetNumberOfOutputs();
  }

  else if(this->GetCurrentPlaybackMode() == SequentialMode)
  {
    if(m_SequentialPlayer.IsNotNull())
      result = m_SequentialPlayer->GetNumberOfOutputs();
  }

  // at the moment this works only if player is initialized
  return result;
}

void QmitkIGTPlayerWidget::SetUpdateRate(unsigned int msecs)
{
  m_PlayingTimer->setInterval((int) msecs); // set update timer update rate
}

void QmitkIGTPlayerWidget::OnPlayButtonClicked(bool checked)
{
  if ( ! checked )
  {
    if ( this->GetCurrentPlaybackMode() == RealTimeMode )
    {
      m_RealTimePlayer->StopPlaying();
    }
    else if ( this->GetCurrentPlaybackMode() == SequentialMode )
    {
      //      m_SequentialPlayer->
    }
  }

  if(CheckInputFileValid())  // no playing possible without valid input file
  {
    switch ( this->GetCurrentPlaybackMode() )
    {
    case RealTimeMode:
      {
        break;
      }
    case SequentialMode:
      {
        break;
      }
    }

    PlaybackMode currentMode = this->GetCurrentPlaybackMode();
    bool isRealTimeMode = currentMode == RealTimeMode;
    bool isSequentialMode = currentMode == SequentialMode;

    if(checked) // play
    {
      if( (isRealTimeMode && m_RealTimePlayer.IsNull()) || (isSequentialMode && m_SequentialPlayer.IsNull()))  // start play
      {
        mitk::NavigationDataSet::Pointer navigationDataSet;
        try
        {
          mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
          navigationDataSet = reader->Read(m_CmpFilename.toStdString());
        }
        catch(mitk::IGTException)
        {
          std::string errormessage = "Error during start playing. Invalid or wrong file?";
          QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
          m_Controls->playPushButton->setChecked(false);
          m_RealTimePlayer = NULL;
          return;
        }

        if(isRealTimeMode)
        {
          m_RealTimePlayer = mitk::NavigationDataPlayer::New();
          m_RealTimePlayer->SetNavigationDataSet(navigationDataSet);
          try
          {
            m_RealTimePlayer->StartPlaying();
          }
          catch(mitk::IGTException)
          {
            std::string errormessage = "Error during start playing. Invalid or wrong file?";
            QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
            m_Controls->playPushButton->setChecked(false);
            m_RealTimePlayer = NULL;
            return;
          }
        }
        else if(isSequentialMode)
        {
          m_SequentialPlayer = mitk::NavigationDataSequentialPlayer::New();
          try
          {
            m_SequentialPlayer->SetNavigationDataSet(navigationDataSet);
          }
          catch(mitk::IGTException)
          {
            std::string errormessage = "Error during start playing. Invalid or wrong file type?";
            QMessageBox::warning(NULL, "IGTPlayer: Error", errormessage.c_str());
            m_Controls->playPushButton->setChecked(false);
            m_RealTimePlayer = NULL;
            return;
          }

          m_Controls->samplePositionHorizontalSlider->setMinimum(0);

          m_Controls->samplePositionHorizontalSlider->setMaximum(m_SequentialPlayer->GetNumberOfSnapshots());

          m_Controls->samplePositionHorizontalSlider->setEnabled(true);
        }

        m_PlayingTimer->start(100);

        emit SignalPlayingStarted();
      }
      else // resume play
      {
        if(isRealTimeMode)
          m_RealTimePlayer->Resume();

        m_PlayingTimer->start(100);
        emit SignalPlayingResumed();
      }
    }

    else // pause
    {
      if(isRealTimeMode)
        m_RealTimePlayer->Pause();

      m_PlayingTimer->stop();
      emit SignalPlayingPaused();
    }
  }

  else
  {
    m_Controls->playPushButton->setChecked(false); // uncheck play button if file unvalid
  }
}

QmitkIGTPlayerWidget::PlaybackMode QmitkIGTPlayerWidget::GetCurrentPlaybackMode()
{
  /*if(m_Controls->sequencialModeCheckBox->isChecked())
  return SequentialMode;
  else*/
  return RealTimeMode;
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
  emit SignalPlayingStopped();

  if(m_RealTimePlayer.IsNotNull())
    m_RealTimePlayer->StopPlaying();

  m_StartTime = -1;  // set starttime back
  m_CurrentSequentialPointNumber = 0;
  m_Controls->samplePositionHorizontalSlider->setSliderPosition(m_CurrentSequentialPointNumber);
  m_Controls->sampleLCDNumber->display(static_cast<int>(m_CurrentSequentialPointNumber));

  this->ResetLCDNumbers();
  m_Controls->playPushButton->setChecked(false); // set play button unchecked
}

void QmitkIGTPlayerWidget::OnPlaying()
{
  switch ( this->GetCurrentPlaybackMode() )
  {
  case RealTimeMode:
    {
      if ( m_RealTimePlayer.IsNull() ) { return; }

      if ( m_StartTime < 0 )
      {
        // get playback start time
        m_StartTime = m_RealTimePlayer->GetOutput()->GetTimeStamp();
      }

      if( ! m_RealTimePlayer->IsAtEnd() )
      {
        m_RealTimePlayer->Update(); // update player

        int msc = (int) (m_RealTimePlayer->GetOutput()->GetTimeStamp() - m_StartTime);

        // calculation for playing time display
        int ms = msc % 1000;
        msc = (msc - ms) / 1000;
        int s = msc % 60;
        int min = (msc-s) / 60;

        // set lcd numbers
        m_Controls->msecLCDNumber->display(ms);
        m_Controls->secLCDNumber->display(s);
        m_Controls->minLCDNumber->display(min);

        emit SignalPlayerUpdated(); // player successfully updated
      }
      else
      {
        this->StopPlaying(); // if player is at EOF
      }

      break;
    }
  case SequentialMode:
    {
      if ( m_SequentialPlayer.IsNull() ) { return; }

      if ( m_CurrentSequentialPointNumber < m_SequentialPlayer->GetNumberOfSnapshots() )
      {
        m_SequentialPlayer->Update(); // update sequential player

        m_Controls->samplePositionHorizontalSlider->setSliderPosition(m_CurrentSequentialPointNumber++); // refresh slider position
        m_Controls->sampleLCDNumber->display(static_cast<int>(m_CurrentSequentialPointNumber));

        //for debugging purposes
        //std::cout << "Sample: " << m_CurrentSequentialPointNumber << " X: " << m_SequentialPlayer->GetOutput()->GetPosition()[0] << " Y: " << m_SequentialPlayer->GetOutput()->GetPosition()[1] << " Y: " << m_SequentialPlayer->GetOutput()->GetPosition()[2] << std::endl;

        emit SignalPlayerUpdated(); // player successfully updated
      }
      else
      {
        this->StopPlaying(); // if player is at EOF
      }

      break;
    }
  }
}

const std::vector<mitk::NavigationData::Pointer> QmitkIGTPlayerWidget::GetNavigationDatas()
{
  std::vector<mitk::NavigationData::Pointer> navDatas;

  if(this->GetCurrentPlaybackMode() == RealTimeMode && m_RealTimePlayer.IsNotNull())
  {
    for(unsigned int i=0; i < m_RealTimePlayer->GetNumberOfOutputs(); ++i)
    {
      navDatas.push_back(m_RealTimePlayer->GetOutput(i)); // push back current navigation data for each tool
    }
  }

  else if(this->GetCurrentPlaybackMode() == SequentialMode && m_SequentialPlayer.IsNotNull())
  {
    for(unsigned int i=0; i < m_SequentialPlayer->GetNumberOfOutputs(); ++i)
    {
      navDatas.push_back(m_SequentialPlayer->GetOutput(i)); // push back current navigation data for each tool
    }
  }

  return navDatas;
}

const mitk::PointSet::Pointer QmitkIGTPlayerWidget::GetNavigationDatasPointSet()
{
  mitk::PointSet::Pointer result = mitk::PointSet::New();

  mitk::PointSet::PointType pointType;

  PlaybackMode currentMode = this->GetCurrentPlaybackMode();
  bool isRealTimeMode = currentMode == RealTimeMode;
  bool isSequentialMode = currentMode == SequentialMode;

  if( (isRealTimeMode && m_RealTimePlayer.IsNotNull()) || (isSequentialMode && m_SequentialPlayer.IsNotNull()))
  {
    int numberOfOutputs = 0;

    if(isRealTimeMode)
      numberOfOutputs = m_RealTimePlayer->GetNumberOfOutputs();
    else if(isSequentialMode)
      numberOfOutputs = m_SequentialPlayer->GetNumberOfOutputs();

    for(unsigned int i=0; i < m_RealTimePlayer->GetNumberOfOutputs(); ++i)
    {
      mitk::NavigationData::PositionType position;

      if(isRealTimeMode)
        position = m_RealTimePlayer->GetOutput(i)->GetPosition();
      else if(isSequentialMode)
        position = m_SequentialPlayer->GetOutput(i)->GetPosition();

      pointType[0] = position[0];
      pointType[1] = position[1];
      pointType[2] = position[2];

      result->InsertPoint(i,pointType);  // insert current ND as Pointtype in PointSet for return
    }
  }

  return result;
}

const mitk::PointSet::PointType QmitkIGTPlayerWidget::GetNavigationDataPoint(unsigned int index)
{
  if( index > this->GetNumberOfTools() || index < 0 )
    throw std::out_of_range("Tool Index out of range!");

  PlaybackMode currentMode = this->GetCurrentPlaybackMode();
  bool isRealTimeMode = currentMode == RealTimeMode;
  bool isSequentialMode = currentMode == SequentialMode;

  // create return PointType from current ND for tool index
  mitk::PointSet::PointType result;

  if( (isRealTimeMode && m_RealTimePlayer.IsNotNull()) || (isSequentialMode && m_SequentialPlayer.IsNotNull()))
  {
    mitk::NavigationData::PositionType position;

    if(isRealTimeMode)
      position = m_RealTimePlayer->GetOutput(index)->GetPosition();
    else if(isSequentialMode)
      position = m_SequentialPlayer->GetOutput(index)->GetPosition();

    result[0] = position[0];
    result[1] = position[1];
    result[2] = position[2];
  }

  return result;
}

/*void QmitkIGTPlayerWidget::SetRealTimePlayer( mitk::NavigationDataPlayer::Pointer player )
{
if(player.IsNotNull())
m_RealTimePlayer = player;
}

void QmitkIGTPlayerWidget::SetSequentialPlayer( mitk::NavigationDataSequentialPlayer::Pointer player )
{
if(player.IsNotNull())
m_SequentialPlayer = player;
}*/

void QmitkIGTPlayerWidget::OnOpenFileButtonPressed()
{
  QString filename = QFileDialog::getOpenFileName(this, "Load tracking data", QDir::currentPath(),"XML files (*.xml)");
  QFile file(filename);

  // if something went wrong or user pressed cancel in the save dialog
  if ( filename.isEmpty()  || ! file.exists() )
  {
    QMessageBox::warning(NULL, "Warning", QString("Please enter valid path. Using previous path again."));
    return;
  }

  m_CmpFilename = filename;

  this->OnGoToEnd(); /// stops playing and resets lcd numbers

  m_Controls->m_ActiveFileLabel->setText(m_CmpFilename);

  emit SignalInputFileChanged();

  mitk::NavigationDataReaderInterface::Pointer navigationDataReader = mitk::NavigationDataReaderXML::New().GetPointer();
  mitk::NavigationDataSet::Pointer navigationDataSet = navigationDataReader->Read(m_CmpFilename.toStdString());
  m_RealTimePlayer->SetNavigationDataSet(navigationDataSet);
  m_SequentialPlayer->SetNavigationDataSet(navigationDataSet);

  m_Controls->m_PlayerControlsGroupBox->setEnabled(true);
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

  if(this->GetCurrentPlaybackMode() == RealTimeMode && m_RealTimePlayer.IsNotNull())
  {
    m_RealTimePlayer->StopPlaying();
    m_RealTimePlayer = NULL;  // set player to NULL so it can be initialized again if playback is called afterwards
  }

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

  // before making changed to QComboBox it is recommended to disconnet it's SIGNALS and SLOTS
  disconnect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );

  if(!toolNames.isEmpty())
    m_Controls->trajectorySelectComboBox->insertItems(0, toolNames); // adding current tool names to combobox

  // reconnect after performed changes
  connect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );
}

int QmitkIGTPlayerWidget::GetResolution()
{
  return m_Controls->resolutionSpinBox->value();  // return currently selected trajectory resolution
}

void QmitkIGTPlayerWidget::ClearTrajectorySelectCombobox()
{
  // before making changed to QComboBox it is recommended to disconnet it's SIGNALS and SLOTS
  disconnect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );

  m_Controls->trajectorySelectComboBox->clear();

  // reconnect after performed changes
  connect( (QObject*) (m_Controls->trajectorySelectComboBox), SIGNAL(currentIndexChanged(int)), this, SIGNAL(SignalCurrentTrajectoryChanged(int)) );
}

void QmitkIGTPlayerWidget::OnSequencialModeToggled(bool toggled)
{
  this->StopPlaying(); // stop playing when mode is changed

  if(toggled)
  {
    m_Controls->samplePositionHorizontalSlider->setEnabled(true); // enable slider if sequential mode
  }
  else if(!toggled)
  {
    m_Controls->samplePositionHorizontalSlider->setSliderPosition(0); // set back and disable slider
    m_Controls->samplePositionHorizontalSlider->setDisabled(true);
  }
}

void QmitkIGTPlayerWidget::OnSliderReleased()
{
  int currentSliderValue = m_Controls->samplePositionHorizontalSlider->value(); // current slider value selected through user movement

  if(currentSliderValue > m_CurrentSequentialPointNumber) // at the moment only forward scrolling is possible
  {
    unsigned int snapshotNumber = currentSliderValue;
    m_SequentialPlayer->GoToSnapshot(snapshotNumber); // move player to selected snapshot
    m_CurrentSequentialPointNumber = currentSliderValue;
    m_Controls->sampleLCDNumber->display(currentSliderValue); // update lcdnumber in widget
  }
  else
    m_Controls->samplePositionHorizontalSlider->setValue(m_CurrentSequentialPointNumber);
}

void QmitkIGTPlayerWidget::OnSliderPressed()
{
  if(m_Controls->playPushButton->isChecked())  // check if widget is playing
    m_Controls->playPushButton->click(); // perform click to pause the play
}