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
#include <QmitkUSNewVideoDeviceWidget.h>

//QT headers


//mitk headers


//itk headers


const std::string QmitkUSNewVideoDeviceWidget::VIEW_ID = "org.mitk.views.QmitkUSNewVideoDeviceWidget";

QmitkUSNewVideoDeviceWidget::QmitkUSNewVideoDeviceWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkUSNewVideoDeviceWidget::~QmitkUSNewVideoDeviceWidget()
{
}

//////////////////// INITIALIZATION /////////////////////

void QmitkUSNewVideoDeviceWidget::Initialize (mitk::USDeviceService::Pointer deviceService){
  m_DeviceService = deviceService;
}


void QmitkUSNewVideoDeviceWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkUSNewVideoDeviceWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkUSNewVideoDeviceWidget::CreateConnections()
{
  if ( m_Controls )
  {
    //connect( m_Controls->m_BtnActivate,   SIGNAL(clicked()), this, SLOT(OnClickedActivateDevice()) );
   // connect( m_Controls->m_BtnDisconnect, SIGNAL(clicked()), this, SLOT(OnClickedDisconnectDevice()) );
  }
}


///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSNewVideoDeviceWidget::OnClickedActivateDevice(){
  MITK_INFO << "Activated Device";
}

void QmitkUSNewVideoDeviceWidget::OnClickedDisconnectDevice(){
  MITK_INFO << "Disconnected Device";
}



///////////////// Methods & Slots Handling Logic //////////////////////////




/////////////////////// HOUSEHOLDING CODE /////////////////////////////////

QListWidgetItem* QmitkUSNewVideoDeviceWidget::ConstructItemFromDevice(mitk::USDevice::Pointer device){
  QListWidgetItem *result = new QListWidgetItem;
  std::string text = device->GetDeviceManufacturer() + "|" + device->GetDeviceModel();
  result->setText(text.c_str());
  return result;
}