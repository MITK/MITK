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
#include <list>

//QT headers


//mitk headers


//itk headers

//microservices
#include <usModuleRegistry.h>
#include <usModule.h>


const std::string QmitkUSDeviceManagerWidget::VIEW_ID = "org.mitk.views.QmitkUSDeviceManagerWidget";

QmitkUSDeviceManagerWidget::QmitkUSDeviceManagerWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);

  // get ModuleContext
  mitk::Module* mitkUS = mitk::ModuleRegistry::GetModule("MitkUS");  
  m_MitkUSContext = mitkUS->GetModuleContext();

  // Register this Widget as a listener for Registry changes.
  // If devices are registered, unregistered or changed, notifications will go there
  m_MitkUSContext->AddServiceListener(this, &QmitkUSDeviceManagerWidget::OnServiceEvent ,"");
}

QmitkUSDeviceManagerWidget::~QmitkUSDeviceManagerWidget()
{
}

//////////////////// INITIALIZATION /////////////////////

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
  GetAllRegisteredDevices();
}

void QmitkUSDeviceManagerWidget::OnClickedDisconnectDevice(){
  MITK_INFO << "Disconnected Device";
}



///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkUSDeviceManagerWidget::OnServiceEvent(const mitk::ServiceEvent event){
  // Empty ListWidget
  m_Controls->m_ConnectedDevices->clear();

  
  // get Active Devices
  std::vector<mitk::USDevice::Pointer> devices = this->GetAllRegisteredDevices();
  // Transfer them to the List
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


//mitk::ServiceTracker<mitk::USDevice, mitk::USDevice::Pointer> QmitkUSDeviceManagerWidget::ConstructServiceTracker(){
//return 0;
//}

std::vector <mitk::USDevice::Pointer> QmitkUSDeviceManagerWidget::GetAllRegisteredDevices(){
  
  //Get Service References
  std::list<mitk::ServiceReference> serviceRefs = m_MitkUSContext->GetServiceReferences<mitk::USDevice>();
  
  // Convert Service References to US Devices
  std::vector<mitk::USDevice::Pointer>* result = new std::vector<mitk::USDevice::Pointer>;
  std::list<mitk::ServiceReference>::const_iterator iterator;
  for (iterator = serviceRefs.begin(); iterator != serviceRefs.end(); ++iterator)
  {
    mitk::USDevice::Pointer device = m_MitkUSContext->GetService<mitk::USDevice>(*iterator);
    if (device) result->push_back(device);
  }

  return *result;
}