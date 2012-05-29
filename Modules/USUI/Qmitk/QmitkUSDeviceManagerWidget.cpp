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

//#define _USE_MATH_DEFINES
#include <QmitkUSDeviceManagerWidget.h>

//QT headers


//mitk headers


//itk headers


const std::string QmitkUSDeviceManagerWidget::VIEW_ID = "org.mitk.views.QmitkUSDeviceManagerWidget";

QmitkUSDeviceManagerWidget::QmitkUSDeviceManagerWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkUSDeviceManagerWidget::~QmitkUSDeviceManagerWidget()
{
}

//////////////////// INITIALIZATION /////////////////////

void QmitkUSDeviceManagerWidget::Initialize (mitk::USDeviceService::Pointer deviceService){
  m_DeviceService = deviceService;
}


void QmitkUSDeviceManagerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkUSDeviceManagerWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkUSDeviceManagerWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_BtnActivate,   SIGNAL(clicked()), this, SLOT(OnClickedActivateDevice()) );
    connect( m_Controls->m_BtnDisconnect, SIGNAL(clicked()), this, SLOT(OnClickedDisconnectDevice()) );
  }
}


///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSDeviceManagerWidget::OnClickedActivateDevice(){
  MITK_INFO << "Activated Device";
}

void QmitkUSDeviceManagerWidget::OnClickedDisconnectDevice(){
  MITK_INFO << "Disconnected Device";
}



///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkUSDeviceManagerWidget::OnDeviceServiceUpdated(){
  // Empty ListWidget
  m_Controls->m_ConnectedDevices->clear();
  // get active Devices
  std::vector<mitk::USDevice::Pointer> devices = m_DeviceService->GetActiveDevices();


  for(std::vector<mitk::USDevice::Pointer>::iterator it = devices.begin(); it != devices.end(); ++it) {
    QListWidgetItem *newItem = ConstructItemFromDevice(it->GetPointer());
   m_Controls->m_ConnectedDevices->addItem(newItem);
  }
}


/////////////////////// HOUSEHOLDING CODE /////////////////////////////////

QListWidgetItem* QmitkUSDeviceManagerWidget::ConstructItemFromDevice(mitk::USDevice::Pointer device){
  QListWidgetItem *result = new QListWidgetItem;
  std::string text = device->GetDeviceManufacturer() + "|" + device->GetDeviceModel();
  result->setText(text.c_str());
  return result;
}