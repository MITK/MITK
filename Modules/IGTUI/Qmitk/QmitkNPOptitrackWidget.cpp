/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNPOptitrackWidget.h"

#include "mitkOptitrackTrackingDevice.h"
#include "mitkNPOptitrackTrackingTypeInformation.h"
#include "QmitkIGTCommonHelper.h"

#include <QFileDialog>
#include <QScrollBar>
#include <Poco/Path.h>

const std::string QmitkNPOptitrackWidget::VIEW_ID = "org.mitk.views.NPOptitrackWidget";

QmitkNPOptitrackWidget::QmitkNPOptitrackWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}

void QmitkNPOptitrackWidget::Initialize()
{
  InitializeSuperclassWidget();
  CreateQtPartControl(this);
  CreateConnections();
}

QmitkNPOptitrackWidget::~QmitkNPOptitrackWidget()
{
  delete m_Controls;
}

void QmitkNPOptitrackWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNPOptitrackWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkNPOptitrackWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_testConnectionOptitrack), SIGNAL(clicked()), this, SLOT(TestConnection()));
    connect((QObject*)(m_Controls->m_SetOptitrackCalibrationFile), SIGNAL(clicked()), this, SLOT(SetOptitrackCalibrationFileClicked()));
  }
}

void QmitkNPOptitrackWidget::ResetOutput()
{
  m_Controls->m_outputTextOptitrack->setHtml("<body style=\" font-family:\'MS Shell Dlg 2\'; font-size:7pt; font-weight:400; font-style:normal;\" bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u>");
}

void QmitkNPOptitrackWidget::AddOutput(std::string s)
{
  m_Controls->m_outputTextOptitrack->setHtml(QString(s.c_str()));
  m_Controls->m_outputTextOptitrack->verticalScrollBar()->setValue(m_Controls->m_outputTextOptitrack->verticalScrollBar()->maximum());
}

mitk::TrackingDevice::Pointer QmitkNPOptitrackWidget::GetTrackingDevice()
{
  // Create the Tracking Device
  mitk::OptitrackTrackingDevice::Pointer tempTrackingDevice = mitk::OptitrackTrackingDevice::New();
  // Set the calibration File
  tempTrackingDevice->SetCalibrationPath(m_OptitrackCalibrationFile);

  //Set the camera parameters
  tempTrackingDevice->SetExp(m_Controls->m_OptitrackExp->value());
  tempTrackingDevice->SetLed(m_Controls->m_OptitrackLed->value());
  tempTrackingDevice->SetThr(m_Controls->m_OptitrackThr->value());

  tempTrackingDevice->SetType(mitk::NPOptitrackTrackingTypeInformation::GetTrackingDeviceName());
  return static_cast<mitk::TrackingDevice::Pointer>(tempTrackingDevice);
}

bool QmitkNPOptitrackWidget::IsDeviceInstalled()
{
  return mitk::OptitrackTrackingDevice::New()->IsDeviceInstalled();
}

void QmitkNPOptitrackWidget::SetOptitrackCalibrationFileClicked()
{
  std::string filename = QFileDialog::getOpenFileName(nullptr, tr("Open Calibration File"), QmitkIGTCommonHelper::GetLastFileLoadPath(), "*.*").toLatin1().data();
  if (filename == "") { return; }
  else
  {
    QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));
    m_OptitrackCalibrationFile = filename;
    Poco::Path myPath = Poco::Path(m_OptitrackCalibrationFile.c_str());
    m_Controls->m_OptitrackCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
  }
}

QmitkNPOptitrackWidget* QmitkNPOptitrackWidget::Clone(QWidget* parent) const
{
  QmitkNPOptitrackWidget* clonedWidget = new QmitkNPOptitrackWidget(parent);
  clonedWidget->Initialize();
  clonedWidget->m_OptitrackCalibrationFile = this->m_OptitrackCalibrationFile;
  clonedWidget->m_Controls->m_OptitrackCalibrationFile->setText(m_Controls->m_OptitrackCalibrationFile->text());

  clonedWidget->m_Controls->m_OptitrackExp->setValue(m_Controls->m_OptitrackExp->value());
  clonedWidget->m_Controls->m_OptitrackLed->setValue(m_Controls->m_OptitrackLed->value());
  clonedWidget->m_Controls->m_OptitrackThr->setValue(m_Controls->m_OptitrackThr->value());
  return clonedWidget;
}
