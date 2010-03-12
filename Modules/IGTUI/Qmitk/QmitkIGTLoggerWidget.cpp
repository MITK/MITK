/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: QmitkTrackingDeviceWidget.cpp $
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

  //update filename label
  std::string tmpDir = itksys::SystemTools::GetCurrentWorkingDirectory();
  m_Dir.append(QString(tmpDir.c_str()));
  m_Dir.append("/");
  this->UpdateFilename();

  //update milli seconds and samples
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
    connect( (QObject*)(m_Controls->m_pbLoadDir), SIGNAL(clicked()), this, SLOT(OnLoadDir()) );
    connect( (QObject*)(m_Controls->m_pbStartRecording), SIGNAL(clicked()), this, SLOT(OnStartRecording()) );
    connect( m_RecordingTimer, SIGNAL(timeout()), this, SLOT(OnRecording()) );
    connect( (QObject*)(m_Controls->m_leFileName), SIGNAL(editingFinished()), this, SLOT(UpdateFilename()) );
    connect( (QObject*)(m_Controls->m_leRecordingValue), SIGNAL(editingFinished()), this, SLOT(UpdateRecordingTime()) );
    connect( (QObject*)(m_Controls->m_cbRecordingType), SIGNAL(activated(int)), this, SLOT(UpdateRecordingTime()) );
  }
}

void QmitkIGTLoggerWidget::SetDataStorage(mitk::DataStorage* dataStorage)
{
  m_DataStorage = dataStorage;
}

void QmitkIGTLoggerWidget::OnStartRecording()
{
  
  if (m_Recorder.IsNull())
  {
    QMessageBox::warning(NULL, "Warning", QString("Please start tracking before recording!"));
    return;
  }
  if (m_CmpFilename.isEmpty())
  {
    QMessageBox::warning(NULL, "Warning", QString("Please specify filename without extension!"));
    return;
  }

   if (!m_RecordingActivated)
  {   
    m_Recorder->SetFileName(m_CmpFilename.toStdString());  

    try
    { /*start the recording mechanism */     
      m_Recorder->StartRecording();  
      m_RecordingTimer->start(50);    //now every update of the recorder stores one line into the file for each added NavigationData      
      mitk::StatusBar::GetInstance()->DisplayText("Recording tracking data now"); // Display recording message for 75ms in status bar
    }
    catch (std::exception& e)
    {
      QMessageBox::warning(NULL, "IGT-Tracking Logger: Error", QString("Error while recording tracking data: %1").arg(e.what()));
      mitk::StatusBar::GetInstance()->DisplayText(""); // Display recording message for 75ms in status bar
    }
    m_Controls->m_pbStartRecording->setText("Stop recording");
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

void QmitkIGTLoggerWidget::StopRecording()
{
  m_RecordingTimer->stop();
  m_Recorder->StopRecording();
  mitk::StatusBar::GetInstance()->DisplayText("Recording STOPPED", 2000); // Display  message for 2s in status bar
  m_Controls->m_pbStartRecording->setText("Start recording"); 
  m_RecordingActivated = false;
}

void QmitkIGTLoggerWidget::OnRecording()
{
  static unsigned int sampleCounter = 0;
  unsigned int int_samples = m_Samples.toInt();
  if(sampleCounter >= int_samples)
  {
    this->StopRecording();
    sampleCounter=0;
  }
  m_Recorder->Update();
  
  if (m_Controls->m_cbRecordingType->currentIndex()==1)
    sampleCounter++;
}

void QmitkIGTLoggerWidget::OnLoadDir()
{
  m_Dir.clear();
  m_CmpFilename.clear();

  std::string tmpDir = itksys::SystemTools::GetCurrentWorkingDirectory();
  m_Dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
    QString(tmpDir.c_str()),
    QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
  if (m_Controls->m_leFileName->text().isEmpty())
  {
    QMessageBox::warning(NULL, "Warning", QString("Please specify filename without extension!"));
    return;
  }
  
  m_Dir.append("/");
  QString filename (m_Controls->m_leFileName->text());
  
  m_CmpFilename.append(m_Dir);
  m_CmpFilename.append(filename);    
  m_Controls->m_tlFullFileName->setText(m_CmpFilename+".xml");
}

void QmitkIGTLoggerWidget::SetRecorder( mitk::NavigationDataRecorder::Pointer recorder )
{
  m_Recorder = recorder;
}

void QmitkIGTLoggerWidget::UpdateFilename()
{
  m_CmpFilename.clear();
  m_CmpFilename.append(m_Dir);
  m_CmpFilename.append(m_Controls->m_leFileName->text());
 
  m_Controls->m_tlFullFileName->setText(m_CmpFilename+".xml");
}

void QmitkIGTLoggerWidget::UpdateRecordingTime()
{
  // milliseconds selected in the combobox
  if (m_Controls->m_cbRecordingType->currentIndex()==0)
  {
     m_MilliSeconds = m_Controls->m_leRecordingValue->text();
     
     bool success = false;

     if (!success)
     {
       QMessageBox::warning(NULL, "Warning", QString("Please enter a number!"));
       this->SetDefaultRecordingSettings();
       return;
     }
  }
  else // #samples selected in the combobox
  {
    m_Samples = m_Controls->m_leRecordingValue->text();

    bool success = false;

    if (!success)
    {
      QMessageBox::warning(NULL, "Warning", QString("Please enter a number!"));
      this->SetDefaultRecordingSettings();
      return;
    }

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

