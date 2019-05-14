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
#include "mitkUSVideoDevice.h"
#include <mitkUSIGTLDevice.h>

const std::string QmitkUSDeviceManagerWidget::VIEW_ID =
"org.mitk.views.QmitkUSDeviceManagerWidget";

QmitkUSDeviceManagerWidget::QmitkUSDeviceManagerWidget(QWidget* parent,
  Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = nullptr;
  CreateQtPartControl(this);
}

QmitkUSDeviceManagerWidget::~QmitkUSDeviceManagerWidget() {}

//////////////////// INITIALIZATION /////////////////////

void QmitkUSDeviceManagerWidget::CreateQtPartControl(QWidget* parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkUSDeviceManagerWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    m_Controls->m_ConnectedDevices->SetAutomaticallySelectFirstEntry(true);
  }

  // Initializations
  std::string empty = "";
  m_Controls->m_ConnectedDevices->Initialize<mitk::USDevice>(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_LABEL, empty);
}

void QmitkUSDeviceManagerWidget::CreateConnections()
{
  if (m_Controls)
  {
    connect(m_Controls->m_BtnActivate, SIGNAL(clicked()), this,
      SLOT(OnClickedActivateDevice()));
    // connect( m_Controls->m_BtnDisconnect, SIGNAL( clicked() ), this,
    // SLOT(OnClickedDisconnectDevice()) );
    connect(m_Controls->m_BtnRemove, SIGNAL(clicked()), this,
      SLOT(OnClickedRemoveDevice()));
    connect(m_Controls->m_BtnNewDevice, SIGNAL(clicked()), this,
      SLOT(OnClickedNewDevice()));
    connect(m_Controls->m_ConnectedDevices,
      SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this,
      SLOT(OnDeviceSelectionChanged(us::ServiceReferenceU)));
    connect(m_Controls->m_BtnEdit, SIGNAL(clicked()), this,
      SLOT(OnClickedEditDevice()));
  }
}

///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSDeviceManagerWidget::OnClickedActivateDevice()
{
  mitk::USDevice::Pointer device =
    m_Controls->m_ConnectedDevices->GetSelectedService<mitk::USDevice>();
  if (device.IsNull())
  {
    return;
  }

  if (device->GetIsActive())
  {
    device->Deactivate();
    device->Disconnect();
  }
  else
  {
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (device->GetDeviceState() < mitk::USDevice::State_Connected)
    {
      device->Connect();
    }
    if (device->GetIsConnected())
    {
      device->Activate();
    }
    QApplication::restoreOverrideCursor();

    if (!device->GetIsActive())
    {
      QMessageBox::warning(
        this, "Activation failed",
        "Could not activate device. Check logging for details.");
    }
    else
    {
      emit DeviceActivated();
    }
  }

  // Manually reevaluate Button logic
  OnDeviceSelectionChanged(
    m_Controls->m_ConnectedDevices->GetSelectedServiceReference());
}

void QmitkUSDeviceManagerWidget::OnClickedDisconnectDevice()
{
  mitk::USDevice::Pointer device =
    m_Controls->m_ConnectedDevices->GetSelectedService<mitk::USDevice>();
  if (device.IsNull())
  {
    return;
  }

  if (device->GetIsConnected())
  {
    device->Disconnect();
  }
  else
  {
    if (!device->Connect())
    {
      QMessageBox::warning(
        this, "Connecting failed",
        "Could not connect to device. Check logging for details.");
    }
  }
}

void QmitkUSDeviceManagerWidget::OnClickedRemoveDevice()
{
  mitk::USDevice::Pointer device =
    m_Controls->m_ConnectedDevices->GetSelectedService<mitk::USDevice>();

  if (device.IsNull())
  {
    return;
  }

  if (device->GetDeviceClass() == "org.mitk.modules.us.USVideoDevice")
  {
    if (device->GetIsActive())
    {
      device->Deactivate();
    }
    if (device->GetIsConnected())
    {
      device->Disconnect();
    }

    dynamic_cast<mitk::USVideoDevice*>(device.GetPointer())
      ->UnregisterOnService();
  }
  else if (device->GetDeviceClass() == "IGTL Client")
  {
    mitk::USIGTLDevice::Pointer ultrasoundIGTLDevice = dynamic_cast<mitk::USIGTLDevice*>(device.GetPointer());
    if (ultrasoundIGTLDevice->GetIsActive())
    {
      ultrasoundIGTLDevice->Deactivate();
    }
    if (ultrasoundIGTLDevice->GetIsConnected())
    {
      ultrasoundIGTLDevice->Disconnect();
    }
    ultrasoundIGTLDevice->UnregisterOnService();
  }
}

void QmitkUSDeviceManagerWidget::OnClickedNewDevice()
{
  emit NewDeviceButtonClicked();
}

void QmitkUSDeviceManagerWidget::OnClickedEditDevice()
{
  mitk::USDevice::Pointer device =
    m_Controls->m_ConnectedDevices->GetSelectedService<mitk::USDevice>();
  emit EditDeviceButtonClicked(device);
}

void QmitkUSDeviceManagerWidget::OnDeviceSelectionChanged(
  us::ServiceReferenceU reference)
{
  if (!reference)
  {
    m_Controls->m_BtnActivate->setEnabled(false);
    m_Controls->m_BtnRemove->setEnabled(false);
    m_Controls->m_BtnEdit->setEnabled(false);
    return;
  }
  std::string isConnected =
    reference.GetProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_ISCONNECTED)
    .ToString();
  std::string isActive =
    reference.GetProperty(
    mitk::USDevice::GetPropertyKeys().US_PROPKEY_ISACTIVE)
    .ToString();

  if (isActive.compare("false") == 0)
  {
    m_Controls->m_BtnActivate->setEnabled(true);
    m_Controls->m_BtnActivate->setText("Activate");
  }
  else
  {
    m_Controls->m_BtnActivate->setEnabled(true);
    m_Controls->m_BtnActivate->setText("Deactivate");
  }

  std::string deviceClass =
    reference.GetProperty(mitk::USDevice::GetPropertyKeys().US_PROPKEY_CLASS)
    .ToString();
  m_Controls->m_BtnRemove->setEnabled(deviceClass ==
    "org.mitk.modules.us.USVideoDevice" || deviceClass == "IGTL Client");
  m_Controls->m_BtnEdit->setEnabled(((deviceClass == "org.mitk.modules.us.USVideoDevice") && (isActive.compare("false") == 0)) ||
                                    ((deviceClass == "IGTL Client") && (isActive.compare("false") == 0)));
}

void QmitkUSDeviceManagerWidget::DisconnectAllDevices()
{
  // at the moment disconnects ALL devices. Maybe we only want to disconnect the
  // devices handled by this widget?
  us::ModuleContext* thisContext = us::GetModuleContext();
  std::vector<us::ServiceReference<mitk::USDevice> > services =
    thisContext->GetServiceReferences<mitk::USDevice>();
  for (std::vector<us::ServiceReference<mitk::USDevice> >::iterator it =
    services.begin();
    it != services.end(); ++it)
  {
    mitk::USDevice* currentDevice = thisContext->GetService(*it);
    currentDevice->Disconnect();
  }
  MITK_INFO << "Disconnected ALL US devises!";
}
