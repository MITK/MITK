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

#include "QmitkNDIAbstractDeviceWidget.h"

#include <QThread>

const std::string QmitkNDIAbstractDeviceWidget::VIEW_ID = "org.mitk.views.NDIAbstractDeviceWidget";

QmitkNDIAbstractDeviceWidget::QmitkNDIAbstractDeviceWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkAbstractTrackingDeviceWidget(parent, f)
  , m_ScanPortsWorker(new QmitkTrackingDeviceConfigurationWidgetScanPortsWorker())
  , m_ScanPortsWorkerThread(new QThread())
{
}

void QmitkNDIAbstractDeviceWidget::InitializeNDIWidget()
{
  InitializeSuperclassWidget();
  CreateConnections();
}

QmitkNDIAbstractDeviceWidget::~QmitkNDIAbstractDeviceWidget()
{
  if (m_ScanPortsWorker) delete m_ScanPortsWorker;
  if (m_ScanPortsWorkerThread) delete m_ScanPortsWorkerThread;
}

void QmitkNDIAbstractDeviceWidget::CreateConnections()
{
  //slots for the worker thread
  connect(m_ScanPortsWorker, SIGNAL(PortsScanned(int, QString, int)), this, SLOT(AutoScanPortsFinished(int, QString, int)));
  connect(m_ScanPortsWorkerThread, SIGNAL(started()), m_ScanPortsWorker, SLOT(ScanPortsThreadFunc()));
}

void QmitkNDIAbstractDeviceWidget::AutoScanPorts()
{
  this->setEnabled(false);
  AddOutput("<br>Scanning...");
  m_ScanPortsWorkerThread->start();
}

void QmitkNDIAbstractDeviceWidget::AutoScanPortsFinished(int Port, QString result, int PortType)
{
  m_ScanPortsWorkerThread->quit();
#ifdef WIN32
  if (PortType != -1) { MITK_WARN << "Port type is specified although this should not be the case for Windows. Ignoring port type."; }
#else //linux systems
  SetPortTypeToGUI(PortType);
#endif
  SetPortValueToGUI(Port);
  AddOutput(result.toStdString());
  this->setEnabled(true);
}
