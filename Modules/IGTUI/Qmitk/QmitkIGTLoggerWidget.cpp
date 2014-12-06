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

#include "QmitkIGTLoggerWidget.h"

//mitk headers
#include "mitkTrackingTypes.h"
#include <mitkSTLFileReader.h>
#include <mitkSurface.h>
#include <mitkNavigationToolReader.h>
#include <mitkNavigationToolWriter.h>
#include <mitkNavigationToolStorage.h>
#include <mitkNavigationToolStorageDeserializer.h>
#include <mitkNavigationToolStorageSerializer.h>
#include <mitkStatusBar.h>
#include <mitkNavigationDataSetWriterXML.h>

//itk headers
#include <itksys/SystemTools.hxx>

//qt headers
#include <qfiledialog.h>
#include <qmessagebox.h>
#include <qtimer.h>


QmitkIGTLoggerWidget::QmitkIGTLoggerWidget(QWidget* parent, Qt::WindowFlags f)
: QWidget(parent, f), m_Recorder(NULL), m_RecordingActivated(false)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();

  //set output file
  this->SetOutputFileName();

  //update milliseconds and samples
  this->SetDefaultRecordingSettings();
}


QmitkIGTLoggerWidget::~QmitkIGTLoggerWidget()
{
  m_RecordingTimer->stop();
  m_Recorder = NULL;
  m_RecordingTimer = NULL;
}

void QmitkIGTLoggerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkIGTLoggerWidgetControls;
    m_Controls->setupUi(parent);

    m_RecordingTimer = new QTimer(this);
  }
}

void QmitkIGTLoggerWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_pbLoadDir), SIGNAL(clicked()), this, SLOT(OnChangePressed()) );
    connect( (QObject*)(m_Controls->m_pbStartRecording), SIGNAL(clicked(bool)), this, SLOT(OnStartRecording(bool)) );
    connect( m_RecordingTimer, SIGNAL(timeout()), this, SLOT(OnRecording()) );
    connect( (QObject*)(m_Controls->m_leRecordingValue), SIGNAL(editingFinished()), this, SLOT(UpdateRecordingTime()) );
    connect( (QObject*)(m_Controls->m_cbRecordingType), SIGNAL(activated(int)), this, SLOT(UpdateRecordingTime()) );
    connect( (QObject*)(m_Controls->m_leOutputFile), SIGNAL(editingFinished()), this, SLOT(UpdateOutputFileName()) );

  }
}

void QmitkIGTLoggerWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkIGTLoggerWidget::OnStartRecording(bool recording)
{

  if (m_Recorder.IsNull())
  {
    QMessageBox::warning(NULL, "Warning", QString("Please start tracking before recording!"));
    return;
  }
  if (m_CmpFilename.isEmpty())
  {
    QMessageBox::warning(NULL, "Warning", QString("Please specify filename!"));
    return;
  }

  if(recording)
  {

    if (!m_RecordingActivated)
    {
      //m_Recorder->SetFileName(m_CmpFilename.toStdString());

      try
      { /*start the recording mechanism */
        m_Recorder->StartRecording();
        m_RecordingTimer->start(50);    //now every update of the recorder stores one line into the file for each added NavigationData
        mitk::StatusBar::GetInstance()->DisplayText("Recording tracking data now"); // Display recording message for 75ms in status bar

        emit SignalRecordingStarted();
      }
      catch (std::exception& e)
      {
        QMessageBox::warning(NULL, "IGT-Tracking Logger: Error", QString("Error while recording tracking data: %1").arg(e.what()));
        mitk::StatusBar::GetInstance()->DisplayText(""); // Display recording message for 75ms in status bar
      }
      m_Controls->m_pbStartRecording->setText("Stop recording");
      m_Controls->m_leRecordingValue->setEnabled(false);
      m_Controls->m_cbRecordingType->setEnabled(false);

      m_RecordingActivated = true;

      if(m_Controls->m_cbRecordingType->currentIndex()==0)
      {
        bool success = false;
        QString str_ms = m_Controls->m_leRecordingValue->text();
        int int_ms = str_ms.toInt(&success);
        if (success)
          QTimer::singleShot(int_ms, this, SLOT(StopRecording()));
      }
    }
    else
    {
      this->StopRecording();
    }

  }
  else
  {
    this->StopRecording();
    m_Controls->m_pbStartRecording->setChecked(false);
  }

}

void QmitkIGTLoggerWidget::StopRecording()
{
  m_RecordingTimer->stop();
  m_Recorder->StopRecording();
  mitk::StatusBar::GetInstance()->DisplayText("Recording STOPPED", 2000); // Display  message for 2s in status bar
  m_Controls->m_pbStartRecording->setText("Start recording");
  m_Controls->m_pbStartRecording->setChecked(false);
  m_Controls->m_leRecordingValue->setEnabled(true);
  m_Controls->m_cbRecordingType->setEnabled(true);
  m_RecordingActivated = false;

  try
  {
    // write NavigationDataSet on StopRecording
    mitk::NavigationDataSetWriterXML().Write(m_CmpFilename.toStdString(), m_Recorder->GetNavigationDataSet());
  }
  catch(const std::exception &e)
  {
    // TODO: catch must be adapted when new file writer are merged to master
    QMessageBox::warning(NULL, "IGT-Tracking Logger: Error", QString("Error while writing tracking data: %1").arg(e.what()));
    MITK_WARN << "File could not be written.";
  }

  emit SignalRecordingStopped();
}

void QmitkIGTLoggerWidget::OnRecording()
{
  static unsigned int sampleCounter = 0;
  unsigned int int_samples = m_Samples.toInt();
  if(sampleCounter >= int_samples)
  {
    this->StopRecording();
    sampleCounter=0;
    return;
  }
  m_Recorder->Update();

  if (m_Controls->m_cbRecordingType->currentIndex()==1)
    sampleCounter++;
}

void QmitkIGTLoggerWidget::OnChangePressed()
{
  QString oldName = m_CmpFilename;
  m_CmpFilename.clear();
  m_CmpFilename = QFileDialog::getSaveFileName( QApplication::activeWindow()
    , "Save tracking data", "IGT_Tracking_Data.xml", "XML files (*.xml)" );

  if (m_CmpFilename.isEmpty())//if something went wrong or user pressed cancel in the save dialog
  {
    m_CmpFilename=oldName;
  }
  m_Controls->m_leOutputFile->setText(m_CmpFilename);
}

void QmitkIGTLoggerWidget::UpdateOutputFileName()
{
  QString oldName = m_CmpFilename;
  m_CmpFilename.clear();
  m_CmpFilename = m_Controls->m_leOutputFile->text();
  if (m_CmpFilename.isEmpty())
  {
    QMessageBox::warning(NULL, "Warning", QString("Please enter valid path! Using previous path again."));
     m_CmpFilename=oldName;
     m_Controls->m_leOutputFile->setText(m_CmpFilename);
  }
}

void QmitkIGTLoggerWidget::SetRecorder( mitk::NavigationDataRecorder::Pointer recorder )
{
  m_Recorder = recorder;
}


void QmitkIGTLoggerWidget::UpdateRecordingTime()
{
  // milliseconds selected in the combobox
  if (m_Controls->m_cbRecordingType->currentIndex()==0)
  {
    m_MilliSeconds = m_Controls->m_leRecordingValue->text();

    if(m_MilliSeconds.compare("infinite")==0)
    {
      this->SetDefaultRecordingSettings();
    }

    bool success = false;
    m_MilliSeconds.toInt(&success);
    if (!success)
    {
      QMessageBox::warning(NULL, "Warning", QString("Please enter a number!"));
      this->SetDefaultRecordingSettings();
      return;
    }
  }
  else if(m_Controls->m_cbRecordingType->currentIndex()==1) // #samples selected in the combobox
  {
    m_Samples = m_Controls->m_leRecordingValue->text();

    if(m_Samples.compare("infinite")==0)
    {
      this->SetDefaultRecordingSettings();
    }

    bool success = false;
    m_Samples.toInt(&success);
    if (!success)
    {
      QMessageBox::warning(NULL, "Warning", QString("Please enter a number!"));
      this->SetDefaultRecordingSettings();
      return;
    }
  }
  else if (m_Controls->m_cbRecordingType->currentIndex()==2)// infinite selected in the combobox
  {
   // U+221E unicode symbole for infinite
   QString infinite("infinite");
   m_Controls->m_leRecordingValue->setText(infinite);
  }
  // m_Controls->m_leSamples->setText(QString::number(samples));
}


void QmitkIGTLoggerWidget::SetDefaultRecordingSettings()
{
  m_Controls->m_leRecordingValue->setText("2000");
  m_Controls->m_cbRecordingType->setCurrentIndex(0);
  m_Samples="100";
  m_MilliSeconds="2000";
}

void QmitkIGTLoggerWidget::SetOutputFileName()
{
  std::string tmpDir = itksys::SystemTools::GetCurrentWorkingDirectory();
  QString dir = QString(tmpDir.c_str());
  QString filename = "IGT_Tracking_Data.xml";
  m_CmpFilename.append(dir);

  if(dir.isEmpty())
  {
    QMessageBox::warning(NULL, "Warning", QString("Could not load current working directory"));
    return;
  }
  if(dir.endsWith("/")||dir.endsWith("\\"))
  {
    m_CmpFilename.append(filename);
  }
  else
  {
    m_CmpFilename.append("/");
    m_CmpFilename.append(filename);
  }
  m_Controls->m_leOutputFile->setText(m_CmpFilename);
}
