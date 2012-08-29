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
}

void QmitkUSDeviceManagerWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( m_Controls->m_BtnActivate,   SIGNAL(clicked()), this, SLOT(OnClickedActivateDevice()) );
    connect( m_Controls->m_BtnDisconnect, SIGNAL(clicked()), this, SLOT(OnClickedDisconnectDevice()) );
    connect( m_Controls->m_ConnectedDevices, SIGNAL(currentItemChanged( QListWidgetItem *, QListWidgetItem *)), this, SLOT(OnDeviceSelectionChanged()) );
  }

  // Initializations
  std::string empty = "";
  m_Controls->m_ConnectedDevices->Initialize<mitk::USDevice>(mitk::USImageMetadata::PROP_DEV_MODEL, empty);
}


///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSDeviceManagerWidget::OnClickedActivateDevice()
{
  mitk::USDevice::Pointer device = m_Controls->m_ConnectedDevices->GetSelectedServiceAsClass<mitk::USDevice>();
  if (device.IsNull()) return;
  if (device->GetIsActive()) device->Deactivate();
  else device->Activate();
}

void QmitkUSDeviceManagerWidget::OnClickedDisconnectDevice(){
  mitk::USDevice::Pointer device = m_Controls->m_ConnectedDevices->GetSelectedServiceAsClass<mitk::USDevice>();
  if (device.IsNull()) return;
  device->Disconnect();
}

void QmitkUSDeviceManagerWidget::OnDeviceSelectionChanged(){
  mitk::USDevice::Pointer device = m_Controls->m_ConnectedDevices->GetSelectedServiceAsClass<mitk::USDevice>();
  if (device.IsNull()) return;
  if (device->GetIsActive()) m_Controls->m_BtnActivate->setText("Deactivate");
    else m_Controls->m_BtnActivate->setText("Activate");
}

