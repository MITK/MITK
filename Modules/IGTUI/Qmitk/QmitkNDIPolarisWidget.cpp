/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkNDIPolarisWidget.h"

#include "mitkNDITrackingDevice.h"
#include "mitkNDIPolarisTypeInformation.h"

#include <QScrollBar>
#include <QSettings>

const std::string QmitkNDIPolarisWidget::VIEW_ID = "org.mitk.views.NDIPolarisWidget";

QmitkNDIPolarisWidget::QmitkNDIPolarisWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkNDIAbstractDeviceWidget(parent, f)
  , m_Controls(nullptr)
{
}

void QmitkNDIPolarisWidget::Initialize()
{
  InitializeNDIWidget();
  CreateQtPartControl(this);
  CreateConnections();
}

QmitkNDIPolarisWidget::~QmitkNDIPolarisWidget()
{
  delete m_Controls;
}

void QmitkNDIPolarisWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkNDIPolarisWidget;
    m_Controls->setupUi(parent);
  }
}

void QmitkNDIPolarisWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect((QObject*)(m_Controls->m_testConnectionPolaris), SIGNAL(clicked()), this, SLOT(TestConnection()));
    connect((QObject*)(m_Controls->m_AutoScanPolaris), SIGNAL(clicked()), this, SLOT(AutoScanPorts()));

    //set a few UI components depending on Windows / Linux
#ifdef WIN32
    m_Controls->portTypeLabelPolaris->setVisible(false);
    m_Controls->portTypePolaris->setVisible(false);

#else
    m_Controls->m_comPortLabelPolaris->setText("Port Nr:");
    m_Controls->m_portSpinBoxPolaris->setPrefix("");
#endif
  }
  }

void QmitkNDIPolarisWidget::ResetOutput()
{
  m_Controls->m_outputTextPolaris->setHtml("<body style=\" font-family:\'MS Shell Dlg 2\'; font-size:7pt; font-weight:400; font-style:normal;\" bgcolor=black><span style=\"color:#ffffff;\"><u>output:</u>");
}

void QmitkNDIPolarisWidget::AddOutput(std::string s)
{
  m_Controls->m_outputTextPolaris->setHtml(QString(s.c_str()));
  m_Controls->m_outputTextPolaris->verticalScrollBar()->setValue(m_Controls->m_outputTextPolaris->verticalScrollBar()->maximum());
}

mitk::TrackingDevice::Pointer QmitkNDIPolarisWidget::GetTrackingDevice()
{
  mitk::NDITrackingDevice::Pointer tempTrackingDevice = mitk::NDITrackingDevice::New();

  //get port
  int port = 0;
  port = m_Controls->m_portSpinBoxPolaris->value();

  //build prefix (depends on linux/win)
  QString prefix = "";
#ifdef WIN32
  prefix = "COM";
  tempTrackingDevice->SetPortNumber(static_cast<mitk::SerialCommunication::PortNumber>(port)); //also set the com port for compatibility
  tempTrackingDevice->SetIlluminationActivationRate(GetPolarisFrameRate());
#else
  prefix = m_Controls->portTypePolaris->currentText();
  tempTrackingDevice->SetIlluminationActivationRate(GetPolarisFrameRate());
#endif

  //build port name string
  QString portName = prefix + QString::number(port);

  tempTrackingDevice->SetDeviceName(portName.toStdString()); //set the port name
  tempTrackingDevice->SetBaudRate(mitk::SerialCommunication::BaudRate115200);//set baud rate
  tempTrackingDevice->SetType(mitk::NDIPolarisTypeInformation::GetTrackingDeviceName());
  return static_cast<mitk::TrackingDevice::Pointer>(tempTrackingDevice);
}

void QmitkNDIPolarisWidget::StoreUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  if (this->GetPersistenceService()) // now save the settings using the persistence service
  {
    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
    propList->Set("PolarisPortWin", m_Controls->m_portSpinBoxPolaris->value());
    propList->Set("PortTypePolaris", m_Controls->portTypePolaris->currentIndex());
    propList->Set("PolarisFrameRate", GetPolarisFrameRate());
  }
  else // QSettings as a fallback if the persistence service is not available
  {
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));
    settings.setValue("portSpinBoxPolaris", QVariant(m_Controls->m_portSpinBoxPolaris->value()));
    settings.setValue("portTypePolaris", QVariant(m_Controls->portTypePolaris->currentIndex()));
    settings.setValue("PolarisFrameRate", QVariant(GetPolarisFrameRate()));
    settings.endGroup();
  }
}

void QmitkNDIPolarisWidget::LoadUISettings()
{
  std::string id = "org.mitk.modules.igt.ui.trackingdeviceconfigurationwidget";
  if (this->GetPersistenceService())
  {
    int port = 0;
    int portType = 0;
    int polarisFrameRate = 0;

    mitk::PropertyList::Pointer propList = this->GetPersistenceService()->GetPropertyList(id);
    if (propList.IsNull())
    {
      MITK_ERROR << "Property list for this UI (" << id << ") is not available, could not load UI settings!"; return;
    }

    propList->Get("PolarisPortWin", port);
    propList->Get("PortTypePolaris", portType);
    propList->Get("PolarisFrameRate", polarisFrameRate);
    this->SetPortTypeToGUI(portType);
    this->SetPortValueToGUI(port);
    m_Controls->m_frameRateComboBoxPolaris->setCurrentIndex((int)(polarisFrameRate / 30));
  }
  else
  {
    // QSettings as a fallback if the persistence service is not available
    QSettings settings;
    settings.beginGroup(QString::fromStdString(id));

    m_Controls->m_portSpinBoxPolaris->setValue(settings.value("portSpinBoxPolaris", 0).toInt());
    m_Controls->portTypePolaris->setCurrentIndex(settings.value("portTypePolaris", 0).toInt());
    //framerates 20,30,60 --> divided by 30 = 0,1,2 --> index of combobox
    m_Controls->m_frameRateComboBoxPolaris->setCurrentIndex((int)(settings.value("PolarisFrameRate", 0).toInt() / 30));

    settings.endGroup();
  }
}

mitk::IlluminationActivationRate QmitkNDIPolarisWidget::GetPolarisFrameRate()
{
  mitk::IlluminationActivationRate frameRate = mitk::Hz20;
  QString comboBox = m_Controls->m_frameRateComboBoxPolaris->currentText();
  if (comboBox == "20 Hz") frameRate = mitk::Hz20;
  else if (comboBox == "30 Hz") frameRate = mitk::Hz30;
  else if (comboBox == "60 Hz") frameRate = mitk::Hz60;
  return frameRate;
}

void QmitkNDIPolarisWidget::SetPortValueToGUI(int portValue){
  m_Controls->m_portSpinBoxPolaris->setValue(portValue);
}
void QmitkNDIPolarisWidget::SetPortTypeToGUI(int portType){
  m_Controls->portTypePolaris->setCurrentIndex(portType);
}

QmitkNDIPolarisWidget* QmitkNDIPolarisWidget::Clone(QWidget* parent) const
{
  QmitkNDIPolarisWidget* clonedWidget = new QmitkNDIPolarisWidget(parent);
  clonedWidget->Initialize();

  clonedWidget->SetPortTypeToGUI(m_Controls->portTypePolaris->currentIndex());
  clonedWidget->SetPortValueToGUI(m_Controls->m_portSpinBoxPolaris->value());
  clonedWidget->m_Controls->m_frameRateComboBoxPolaris->setCurrentIndex(m_Controls->m_frameRateComboBoxPolaris->currentIndex());
  return clonedWidget;
}
