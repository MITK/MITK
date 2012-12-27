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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

//Mitk
#include "mitkDataNode.h"


// Qmitk
#include "UltrasoundSupport.h"
#include <QTimer>

// Qt
#include <QMessageBox>

// Ultrasound
#include "mitkUSDevice.h"

const std::string UltrasoundSupport::VIEW_ID = "org.mitk.views.ultrasoundsupport";

void UltrasoundSupport::SetFocus()
{
  m_Controls.m_AddDevice->setFocus();
}

void UltrasoundSupport::CreateQtPartControl( QWidget *parent )
{
  m_Timer = new QTimer(this);

  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );
  connect( m_Controls.m_AddDevice, SIGNAL(clicked()), this, SLOT(OnClickedAddNewDevice()) ); // Change Widget Visibilities
  connect( m_Controls.m_AddDevice, SIGNAL(clicked()), this->m_Controls.m_NewVideoDeviceWidget, SLOT(CreateNewDevice()) ); // Init NewDeviceWidget
  connect( m_Controls.m_NewVideoDeviceWidget, SIGNAL(Finished()), this, SLOT(OnNewDeviceWidgetDone()) ); // After NewDeviceWidget finished editing
  connect( m_Controls.m_BtnView, SIGNAL(clicked()), this, SLOT(OnClickedViewDevice()) );
  connect( m_Timer, SIGNAL(timeout()), this, SLOT(DisplayImage()));
  //connect (m_Controls.m_ActiveVideoDevices, SIGNAL())

  // Initializations
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  std::string filter = "(&(" + mitk::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(IsActive=true))";
  m_Controls.m_ActiveVideoDevices->Initialize<mitk::USDevice>(mitk::USImageMetadata::PROP_DEV_MODEL ,filter);


  m_Node = mitk::DataNode::New();
  m_Node->SetName("US Image Stream");
  this->GetDataStorage()->Add(m_Node);
}

void UltrasoundSupport::OnClickedAddNewDevice()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(true);
  m_Controls.m_DeviceManagerWidget->setVisible(false);
  m_Controls.m_AddDevice->setVisible(false);
  m_Controls.m_Headline->setText("Add New Device:");
}

void UltrasoundSupport::DisplayImage()
{
 // m_Device->UpdateOutputData(0);
 // mitk::USImage::Pointer image = m_Device->GetOutput();

  m_Device->UpdateOutputData(0);
  m_Node->SetData(m_Device->GetOutput());
  this->RequestRenderWindowUpdate();

  m_FrameCounter ++;
  if (m_FrameCounter == 10)
  {
    int nMilliseconds = m_Clock.restart();
    float fps = 10000.0f / (nMilliseconds );
    m_Controls.m_FramerateLabel->setText("Current Framerate: "+ QString::number(fps) +" FPS");
    m_FrameCounter = 0;
  }
}

void UltrasoundSupport::OnClickedViewDevice()
{
  m_FrameCounter = 0;
  // We use the activity state of the timer to determine whether we are currently viewing images
  if ( ! m_Timer->isActive() ) // Activate Imaging
  {
    m_Device = m_Controls.m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
    if (m_Device.IsNull()){
      m_Timer->stop();
      return;
    }
    //m_Device->UpdateOutputData(0);
    m_Device->Update();
    m_Node->SetData(m_Device->GetOutput());
    int interval = (1000 / m_Controls.m_FrameRate->value());
    m_Timer->setInterval(interval);
    m_Timer->start();
    m_Controls.m_BtnView->setText("Stop Viewing");
  }
  else
  { //deactivate imaging
    m_Controls.m_BtnView->setText("Start Viewing");
    m_Timer->stop();
    m_Node->ReleaseData();
    this->RequestRenderWindowUpdate();
  }
}

void UltrasoundSupport::OnNewDeviceWidgetDone()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  m_Controls.m_DeviceManagerWidget->setVisible(true);
  m_Controls.m_AddDevice->setVisible(true);
  m_Controls.m_Headline->setText("Connected Devices:");
}