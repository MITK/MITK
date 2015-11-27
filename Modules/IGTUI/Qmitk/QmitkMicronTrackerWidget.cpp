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

#include "QmitkMicronTrackerWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include <QFileDialog>
#include <QMessageBox>

#include <mitkClaronTrackingDevice.h>

#include <itksys/SystemTools.hxx>
#include <Poco/Path.h>

const std::string QmitkMicronTrackerWidget::VIEW_ID = "org.mitk.views.NDIMicronTrackerWidget";

QmitkMicronTrackerWidget::QmitkMicronTrackerWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_ErrorMessage = "";
  m_MTCalibrationFile = "";
}

QmitkMicronTrackerWidget::~QmitkMicronTrackerWidget()
{
}

void QmitkMicronTrackerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkMicronTrackerWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkMicronTrackerWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_testConnectionMicronTracker), SIGNAL(clicked()), this, SLOT(TestConnection()));
    connect((QObject*)(m_Controls->m_SetMTCalibrationFile), SIGNAL(clicked()), this, SLOT(SetMTCalibrationFileClicked()));
  }
}

void QmitkMicronTrackerWidget::ResetOutput()
{
  m_Controls->m_outputTextMicronTracker->setHtml("<body style=\" font-family:\'MS Shell Dlg 2\'; font-size:7pt; font-weight:400; font-style:normal;\" bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u>");
}

void QmitkMicronTrackerWidget::AddOutput(std::string s)
{
  m_Controls->m_outputTextMicronTracker->setHtml(QString(s.c_str()));
  m_Controls->m_outputTextMicronTracker->verticalScrollBar()->setValue(m_Controls->m_outputTextMicronTracker->verticalScrollBar()->maximum());
}

mitk::TrackingDevice::Pointer QmitkMicronTrackerWidget::ConstructTrackingDevice()
{
  mitk::ClaronTrackingDevice::Pointer newDevice = mitk::ClaronTrackingDevice::New();
  if (this->m_MTCalibrationFile.empty()) //if configuration file for MicronTracker is empty: load default
  {
    mitk::ClaronTrackingDevice::Pointer tempDevice = mitk::ClaronTrackingDevice::New();
    m_MTCalibrationFile = tempDevice->GetCalibrationDir();
    Poco::Path myPath = Poco::Path(m_MTCalibrationFile.c_str());
    m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
  }
  if (!this->m_MTCalibrationFile.empty())
  {
    //extract path from calibration file and set the calibration dir of the device
    std::string path = itksys::SystemTools::GetFilenamePath(m_MTCalibrationFile);
    newDevice->SetCalibrationDir(path);
  }
  else
    AddOutput("<br>Warning: Calibration file is not set!");
  return static_cast<mitk::TrackingDevice::Pointer>(newDevice);
}

void QmitkMicronTrackerWidget::StoreUISettings()
{
}

void QmitkMicronTrackerWidget::LoadUISettings()
{
}

bool QmitkMicronTrackerWidget::IsDeviceInstalled()
{
  return mitk::ClaronTrackingDevice::New()->IsDeviceInstalled();
}

void QmitkMicronTrackerWidget::SetMTCalibrationFileClicked()
{
  std::string filename = QFileDialog::getOpenFileName(NULL, tr("Open Calibration File"), "/", "*.*").toLatin1().data();
  if (filename == "") { return; }
  else
  {
    m_MTCalibrationFile = filename;
    Poco::Path myPath = Poco::Path(m_MTCalibrationFile.c_str());
    m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
  }
}
