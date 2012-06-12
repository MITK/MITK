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
#include <QmitkUSDeviceListWidget.h>
#include <list>

//QT headers
#include <QColor>

//mitk headers


//itk headers

//microservices
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usServiceTracker.h>


const std::string QmitkUSDeviceListWidget::VIEW_ID = "org.mitk.views.QmitkUSDeviceListWidget";

QmitkUSDeviceListWidget::QmitkUSDeviceListWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f) 
{
  m_Controls = NULL;
  CreateQtPartControl(this);

  // get ModuleContext
  mitk::Module* mitkUS = mitk::ModuleRegistry::GetModule("MitkUS");  
  m_MitkUSContext = mitkUS->GetModuleContext();

 

  
}

QmitkUSDeviceListWidget::~QmitkUSDeviceListWidget()
{

}

//////////////////// INITIALIZATION /////////////////////

void QmitkUSDeviceListWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkUSDeviceListWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}

void QmitkUSDeviceListWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_DeviceList, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *)), this, SLOT(OnDeviceSelectionChanged()) );
  }
}

void QmitkUSDeviceListWidget::Initialize(std::string filter)
{
  m_Filter = filter;
  m_MitkUSContext->AddServiceListener(this, &QmitkUSDeviceListWidget::OnServiceEvent, m_Filter);
}

///////////////////////// Getter & Setter /////////////////////////////////

mitk::USDevice::Pointer QmitkUSDeviceListWidget::GetSelectedDevice()
{
  return this->GetDeviceForListItem(this->m_Controls->m_DeviceList->currentItem());
}

///////////// Methods & Slots Handling Direct Interaction /////////////////


void QmitkUSDeviceListWidget::OnDeviceSelectionChanged(){
  mitk::USDevice::Pointer device = this->GetDeviceForListItem(this->m_Controls->m_DeviceList->currentItem());
  if (device.IsNull()) return;
  emit (DeviceSelected(device));
}


///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkUSDeviceListWidget::OnServiceEvent(const mitk::ServiceEvent event){
  // Empty ListWidget
  this->m_ListContent.clear();
  m_Controls->m_DeviceList->clear();
  

  
  // get Active Devices
  std::vector<mitk::USDevice::Pointer> devices = this->GetAllRegisteredDevices();
  // Transfer them to the List
  for(std::vector<mitk::USDevice::Pointer>::iterator it = devices.begin(); it != devices.end(); ++it)
  {
    QListWidgetItem *newItem = ConstructItemFromDevice(it->GetPointer());
    //Add new item to QListWidget
    m_Controls->m_DeviceList->addItem(newItem);
    // Construct link and add to internal List for reference
    QmitkUSDeviceListWidget::DeviceListLink link;
    link.device = it->GetPointer();
    link.item = newItem;
    m_ListContent.push_back(link);
  }
}


/////////////////////// HOUSEHOLDING CODE /////////////////////////////////

QListWidgetItem* QmitkUSDeviceListWidget::ConstructItemFromDevice(mitk::USDevice::Pointer device){
  QListWidgetItem *result = new QListWidgetItem;
  std::string text = device->GetDeviceManufacturer() + "|" + device->GetDeviceModel();

  if (device->GetIsActive())
  {
    result->foreground().setColor(Qt::blue);
    text += "|(ON)";
  } else text += "|(OFF)";

  result->setText(text.c_str());

  return result;
}


mitk::USDevice::Pointer QmitkUSDeviceListWidget::GetDeviceForListItem(QListWidgetItem* item)
{
  for(std::vector<QmitkUSDeviceListWidget::DeviceListLink>::iterator it = m_ListContent.begin(); it != m_ListContent.end(); ++it)
  {
    if (item == it->item) return it->device;
  }
  return 0;
}


//mitk::ServiceTracker<mitk::USDevice, mitk::USDevice::Pointer> QmitkUSDeviceListWidget::ConstructServiceTracker(){
//return 0;
//}

std::vector <mitk::USDevice::Pointer> QmitkUSDeviceListWidget::GetAllRegisteredDevices(){
  
  //Get Service References
  std::list<mitk::ServiceReference> serviceRefs = m_MitkUSContext->GetServiceReferences<mitk::USDevice>(m_Filter);
  
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