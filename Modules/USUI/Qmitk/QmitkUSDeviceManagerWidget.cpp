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
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <QMessageBox>


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

void QmitkUSDeviceManagerWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkUSDeviceManagerWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }

  // Initializations
  std::string empty = "";
  m_Controls->m_ConnectedDevices->Initialize<mitk::USDevice>(mitk::USDevice::US_PROPKEY_LABEL, empty);
}

void QmitkUSDeviceManagerWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_BtnActivate,   SIGNAL( clicked() ), this, SLOT(OnClickedActivateDevice()) );
    connect( m_Controls->m_BtnDisconnect, SIGNAL( clicked() ), this, SLOT(OnClickedDisconnectDevice()) );
    connect( m_Controls->m_ConnectedDevices, SIGNAL( ServiceSelectionChanged(mitk::ServiceReference) ), this, SLOT(OnDeviceSelectionChanged(us::ServiceReferenceU)) );
  }
}


///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSDeviceManagerWidget::OnClickedActivateDevice()
{
  mitk::USDevice::Pointer device = m_Controls->m_ConnectedDevices->GetSelectedService<mitk::USDevice>();
  if (device.IsNull())
  {
    return;
  }
  if (device->GetIsActive())
  {
    device->Deactivate();
  }
  else
  {
    device->Activate();

    if ( ! device->GetIsActive() )
    {
      QMessageBox::warning(this, "Activation failed", "Could not activate device. Check logging for details.");
    }
  }

  // Manually reevaluate Button logic
  OnDeviceSelectionChanged(m_Controls->m_ConnectedDevices->GetSelectedServiceReference());
}

void QmitkUSDeviceManagerWidget::OnClickedDisconnectDevice(){
  mitk::USDevice::Pointer device = m_Controls->m_ConnectedDevices->GetSelectedService<mitk::USDevice>();
  if (device.IsNull()) { return; }
  device->Disconnect();
}

void QmitkUSDeviceManagerWidget::OnDeviceSelectionChanged(us::ServiceReferenceU reference){
  if (! reference)
  {
    m_Controls->m_BtnActivate->setEnabled(false);
    m_Controls->m_BtnDisconnect->setEnabled(false);
    return;
  }
  std::string isActive = reference.GetProperty( mitk::USDevice::US_PROPKEY_ISACTIVE ).ToString();
  if (isActive.compare("true") == 0)
  {
    m_Controls->m_BtnActivate->setEnabled(true);
    m_Controls->m_BtnDisconnect->setEnabled(false);
    m_Controls->m_BtnActivate->setText("Deactivate");
  }
  else
  {
    m_Controls->m_BtnActivate->setEnabled(true);
    m_Controls->m_BtnDisconnect->setEnabled(true);
    m_Controls->m_BtnActivate->setText("Activate");
  }
}

void QmitkUSDeviceManagerWidget::DisconnectAllDevices()
{
  //at the moment disconnects ALL devices. Maybe we only want to disconnect the devices handled by this widget?
  us::ModuleContext* thisContext = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::USDevice> > services = thisContext->GetServiceReferences<mitk::USDevice>();
  for(std::vector<us::ServiceReference<mitk::USDevice> >::iterator it = services.begin(); it != services.end(); ++it)
  {
    mitk::USDevice* currentDevice = thisContext->GetService(*it);
    currentDevice->Disconnect();
  }
  MITK_INFO << "Disconnected ALL US devises!";
}

