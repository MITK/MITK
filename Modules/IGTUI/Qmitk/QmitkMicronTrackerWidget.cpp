/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMicronTrackerWidget.h"

#include <QFileDialog>
#include <QScrollBar>

#include <mitkClaronTrackingDevice.h>

#include <itksys/SystemTools.hxx>
#include <Poco/Path.h>
#include <QSettings>

#include <QmitkIGTCommonHelper.h>

const std::string QmitkMicronTrackerWidget::VIEW_ID = "org.mitk.views.NDIMicronTrackerWidget";

QmitkMicronTrackerWidget::QmitkMicronTrackerWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}

void QmitkMicronTrackerWidget::Initialize()
{
  InitializeSuperclassWidget();
  CreateQtPartControl(this);
  CreateConnections();
  m_MTCalibrationFile = "";
}

QmitkMicronTrackerWidget::~QmitkMicronTrackerWidget()
{
  delete m_Controls;
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

mitk::TrackingDevice::Pointer QmitkMicronTrackerWidget::GetTrackingDevice()
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
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  if (this->GetPersistenceService()) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
    propList->Set("MTCalibrationFile", m_MTCalibrationFile);
  }
  else // QSettings as a fallback if the persistence service is not available
  {
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    settings.setValue("mTCalibrationFile", QVariant(QString::fromStdString(m_MTCalibrationFile)));
    settings.endGroup();
  }
}

void QmitkMicronTrackerWidget::LoadUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";

  if (this->GetPersistenceService())
  {
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
    if (propList.IsNull())
    {
      MITK_ERROR << "Property list for this UI (" << id << ") is not available, could not load UI settings!"; return;
    }

    propList->Get("MTCalibrationFile", m_MTCalibrationFile);
  }
  else
  {
    // QSettings as a fallback if the persistence service is not available
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    m_MTCalibrationFile = settings.value("mTCalibrationFile", "").toString().toStdString();

    settings.endGroup();
  }
  m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString::fromStdString(m_MTCalibrationFile));
}

bool QmitkMicronTrackerWidget::IsDeviceInstalled()
{
  return mitk::ClaronTrackingDevice::New()->IsDeviceInstalled();
}

void QmitkMicronTrackerWidget::SetMTCalibrationFileClicked()
{
  std::string filename = QFileDialog::getOpenFileName(nullptr, tr("Open Calibration File"), QmitkIGTCommonHelper::GetLastFileLoadPath(), "*.*").toLatin1().data();
  if (filename == "") { return; }
  else
  {
    QmitkIGTCommonHelper::SetLastFileLoadPathByFileName(QString::fromStdString(filename));
    m_MTCalibrationFile = filename;
    Poco::Path myPath = Poco::Path(m_MTCalibrationFile.c_str());
    m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString(myPath.getFileName().c_str()));
  }
}

QmitkMicronTrackerWidget* QmitkMicronTrackerWidget::Clone(QWidget* parent) const
{
  QmitkMicronTrackerWidget* clonedWidget = new QmitkMicronTrackerWidget(parent);
  clonedWidget->Initialize();
  clonedWidget->m_MTCalibrationFile = m_MTCalibrationFile;
  m_Controls->m_MTCalibrationFile->setText("Calibration File: " + QString::fromStdString(m_MTCalibrationFile));

  return clonedWidget;
}
