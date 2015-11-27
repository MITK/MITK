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

#include "QmitkNDIPolarisWidget.h"
#include "QmitkTrackingDeviceConfigurationWidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <mitkNDITrackingDevice.h>

const std::string QmitkNDIPolarisWidget::VIEW_ID = "org.mitk.views.NDIPolarisWidget";

QmitkNDIPolarisWidget::QmitkNDIPolarisWidget(QWidget* parent, Qt::WindowFlags f)
  : QmitkNDIAbstractDeviceWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
  CreateConnections();
  m_ErrorMessage = "";
}

QmitkNDIPolarisWidget::~QmitkNDIPolarisWidget()
{
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

    //disable unused UI component
    m_Controls->m_polarisTrackingModeBox->setVisible(false); //don't delete this component, because it is used in the MBI part of MITK
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

mitk::TrackingDevice::Pointer QmitkNDIPolarisWidget::ConstructTrackingDevice()
{
  if (m_Controls->m_radioPolaris5D->isChecked()) //5D Tracking
  {
    //not yet in the open source part so we'll only get NULL here.
    MITK_WARN << "5D Tracking not implemented yet! Returning nullptr.";
    return nullptr;
  }
  else //6D Tracking
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
    tempTrackingDevice->SetType(mitk::TRACKING_DEVICE_IDENTIFIER_POLARIS);
    return static_cast<mitk::TrackingDevice::Pointer>(tempTrackingDevice);
  }
}

void QmitkNDIPolarisWidget::StoreUISettings()
{
}

void QmitkNDIPolarisWidget::LoadUISettings()
{
}

mitk::IlluminationActivationRate QmitkNDIPolarisWidget::GetPolarisFrameRate()
{
  mitk::IlluminationActivationRate frameRate;
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
