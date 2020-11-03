/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNDIAuroraWidget.h"

#include "mitkNDITrackingDevice.h"
#include "mitkNDIAuroraTypeInformation.h"

#include <QScrollBar>
#include <QSettings>

const std::string QmitkNDIAuroraWidget::VIEW_ID = "org.mitk.views.NDIAuroraWidget";

QmitkNDIAuroraWidget::QmitkNDIAuroraWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkNDIAbstractDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}

void QmitkNDIAuroraWidget::Initialize()
{
  InitializeNDIWidget();
  CreateQtPartControl(this);
  CreateConnections();
}

QmitkNDIAuroraWidget::~QmitkNDIAuroraWidget()
{
  delete m_Controls;
}

void QmitkNDIAuroraWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNDIAuroraWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkNDIAuroraWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_testConnectionAurora), SIGNAL(clicked()), this, SLOT(TestConnection()));
    connect((QObject*)(m_Controls->m_AutoScanAurora), SIGNAL(clicked()), this, SLOT(AutoScanPorts()));
    //set a few UI components depending on Windows / Linux
#ifdef WIN32
    m_Controls->portTypeLabelAurora->setVisible(false);
    m_Controls->portTypeAurora->setVisible(false);
#else
    m_Controls->comPortLabelAurora->setText("Port Nr:");
    m_Controls->m_portSpinBoxAurora->setPrefix("");
#endif
  }
}

void QmitkNDIAuroraWidget::ResetOutput()
{
  m_Controls->m_outputTextAurora->setHtml("<body style=\" font-family:\'MS Shell Dlg 2\'; font-size:7pt; font-weight:400; font-style:normal;\" bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u>");
}

void QmitkNDIAuroraWidget::AddOutput(std::string s)
{
  m_Controls->m_outputTextAurora->setHtml(QString(s.c_str()));
  m_Controls->m_outputTextAurora->verticalScrollBar()->setValue(m_Controls->m_outputTextAurora->verticalScrollBar()->maximum());
}

mitk::TrackingDevice::Pointer QmitkNDIAuroraWidget::GetTrackingDevice()
{
  mitk::NDITrackingDevice::Pointer tempTrackingDevice = mitk::NDITrackingDevice::New();

  //get port
  int port = 0;
  port = m_Controls->m_portSpinBoxAurora->value();

  //build prefix (depends on linux/win)
  QString prefix = "";
#ifdef WIN32
  prefix = "COM";
  tempTrackingDevice->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(port)); //also set the com port for compatibility
#else
  prefix = m_Controls->portTypeAurora->currentText();
#endif

  //build port name string
  QString portName = prefix + QString::number(port);

  tempTrackingDevice->SetDeviceName(portName.toStdString()); //set the port name
  tempTrackingDevice->SetBaudRate(mitk::SerialCommunication::BaudRate115200);//set baud rate
  tempTrackingDevice->SetType(mitk::NDIAuroraTypeInformation::GetTrackingDeviceName());
  return static_cast<mitk::TrackingDevice::Pointer>(tempTrackingDevice);
}

void QmitkNDIAuroraWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  if (this->GetPersistenceService()) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
    propList->Set("AuroraPortWin", m_Controls->m_portSpinBoxAurora->value());
    propList->Set("PortTypeAurora", m_Controls->portTypeAurora->currentIndex());
  }
  else // QSettings as a fallback if the persistence service is not available
  {
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    settings.setValue("portSpinBoxAurora", QVariant(m_Controls->m_portSpinBoxAurora->value()));
    settings.setValue("portTypeAurora", QVariant(m_Controls->portTypeAurora->currentIndex()));
    settings.endGroup();
  }
}

void QmitkNDIAuroraWidget::LoadUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  if (this->GetPersistenceService())
  {
    int port = 0;
    int portType = 0;
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
    if (propList.IsNull())
    {
      MITK_ERROR << "Property list for this UI (" << id << ") is not available, could not load UI settings!"; return;
    }

    propList->Get("AuroraPortWin", port);
    propList->Get("PortTypeAurora", portType);
    this->SetPortTypeToGUI(portType);
    this->SetPortValueToGUI(port);
  }
  else
  {
    // QSettings as a fallback if the persistence service is not available
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));

    m_Controls->m_portSpinBoxAurora->setValue(settings.value("portSpinBoxAurora", 0).toInt());
    m_Controls->portTypeAurora->setCurrentIndex(settings.value("portTypeAurora", 0).toInt());

    settings.endGroup();
  }
}

void QmitkNDIAuroraWidget::SetPortValueToGUI(int portValue){
  m_Controls->m_portSpinBoxAurora->setValue(portValue);
}
void QmitkNDIAuroraWidget::SetPortTypeToGUI(int portType){
  m_Controls->portTypeAurora->setCurrentIndex(portType);
}

QmitkNDIAuroraWidget* QmitkNDIAuroraWidget::Clone(QWidget* parent) const
{
  QmitkNDIAuroraWidget* clonedWidget = new QmitkNDIAuroraWidget(parent);
  clonedWidget->Initialize();

  clonedWidget->SetPortTypeToGUI(m_Controls->portTypeAurora->currentIndex());
  clonedWidget->SetPortValueToGUI(m_Controls->m_portSpinBoxAurora->value());
  return clonedWidget;
}
