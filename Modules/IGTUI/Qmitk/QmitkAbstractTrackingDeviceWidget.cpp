/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAbstractTrackingDeviceWidget.h"

#include <QThread>

QmitkAbstractTrackingDeviceWidget::QmitkAbstractTrackingDeviceWidget(QWidget* parent, Qt::WindowFlags f)
  : QWidget(parent, f)
  , m_TestConnectionWorker(nullptr)
  , m_TestConnectionWorkerThread(nullptr)
  , m_ErrorMessage("")
  , isInitialized(false)
{
}

void QmitkAbstractTrackingDeviceWidget::InitializeSuperclassWidget()
{
  m_TestConnectionWorkerThread = new QThread();
  m_TestConnectionWorker = new QmitkTrackingDeviceConfigurationWidgetConnectionWorker();
  CreateConnections();
  m_ErrorMessage = "";
  isInitialized = true;
}

QmitkAbstractTrackingDeviceWidget::~QmitkAbstractTrackingDeviceWidget(){
  if (m_TestConnectionWorker) delete m_TestConnectionWorker;
  if (m_TestConnectionWorkerThread) delete m_TestConnectionWorkerThread;
}

void QmitkAbstractTrackingDeviceWidget::TestConnectionFinished(bool connected, QString output)
{
  m_TestConnectionWorkerThread->quit();
  AddOutput(output.toStdString());
  MITK_INFO << "Test connection: " << connected;
  this->setEnabled(true);
}

void QmitkAbstractTrackingDeviceWidget::TestConnection()
{
  this->setEnabled(false);
  //construct a tracking device:
  mitk::TrackingDevice::Pointer testTrackingDevice = GetTrackingDevice();
  m_TestConnectionWorker->SetTrackingDevice(testTrackingDevice);
  m_TestConnectionWorkerThread->start();
}

void QmitkAbstractTrackingDeviceWidget::CreateConnections() {
  connect(m_TestConnectionWorker, SIGNAL(ConnectionTested(bool, QString)), this, SLOT(TestConnectionFinished(bool, QString)));
  connect(m_TestConnectionWorkerThread, SIGNAL(started()), m_TestConnectionWorker, SLOT(TestConnectionThreadFunc()));

  //move the worker to the thread
  m_TestConnectionWorker->moveToThread(m_TestConnectionWorkerThread);
}

QmitkAbstractTrackingDeviceWidget* QmitkAbstractTrackingDeviceWidget::CloneForQt(QWidget* parent) const
{
  QmitkAbstractTrackingDeviceWidget* clonedWidget = this->Clone(parent);
  if (!clonedWidget->IsInitialized())
    MITK_ERROR << "Your cloned widget is not initialized!";
  clonedWidget->create();

  return clonedWidget;
}
