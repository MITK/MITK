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
#include <mitkDataNode.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>


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
  connect( m_Controls.crop_left, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  connect( m_Controls.crop_right, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  connect( m_Controls.crop_top, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  connect( m_Controls.crop_bot, SIGNAL(valueChanged(int)), this, SLOT(OnCropAreaChanged()) );
  //connect (m_Controls.m_ActiveVideoDevices, SIGNAL())

  // Initializations
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  std::string filter = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(" + mitk::USDevice::US_PROPKEY_ISACTIVE + "=true))";
  m_Controls.m_ActiveVideoDevices->Initialize<mitk::USDevice>(mitk::USDevice::US_PROPKEY_LABEL ,filter);

  //UI initializations
  m_Controls.crop_left->setEnabled(false);
  m_Controls.crop_right->setEnabled(false);
  m_Controls.crop_bot->setEnabled(false);
  m_Controls.crop_top->setEnabled(false);


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
  m_Device->UpdateOutputData(0);
  m_Node->SetData(m_Device->GetOutput());
  this->RequestRenderWindowUpdate();

  m_FrameCounter ++;
  if (m_FrameCounter == 10)
  {
    int nMilliseconds = m_Clock.restart();
    int fps = 10000.0f / (nMilliseconds );
    m_Controls.m_FramerateLabel->setText("Current Framerate: "+ QString::number(fps) +" FPS");
    m_FrameCounter = 0;
  }
}

void UltrasoundSupport::OnCropAreaChanged()
{
if (m_Device->GetDeviceClass()=="org.mitk.modules.us.USVideoDevice")
      {
      mitk::USVideoDevice::Pointer currentVideoDevice = dynamic_cast<mitk::USVideoDevice*>(m_Device.GetPointer());

      mitk::USDevice::USImageCropArea newArea;
      newArea.cropLeft = m_Controls.crop_left->value();
      newArea.cropTop = m_Controls.crop_top->value();
      newArea.cropRight = m_Controls.crop_right->value();
      newArea.cropBottom = m_Controls.crop_bot->value();

      //check enabled: if not we are in the initializing step and don't need to do anything
      //otherwise: update crop area
      if (m_Controls.crop_right->isEnabled())
        currentVideoDevice->SetCropArea(newArea);

      GlobalReinit();
      }
else
      {
      MITK_WARN << "No USVideoDevice: Cannot Crop!";
  }
}

void UltrasoundSupport::OnClickedViewDevice()
{
  m_FrameCounter = 0;
  // We use the activity state of the timer to determine whether we are currently viewing images
  if ( ! m_Timer->isActive() ) // Activate Imaging
  {
    //get device & set data node
    m_Device = m_Controls.m_ActiveVideoDevices->GetSelectedService<mitk::USDevice>();
    if (m_Device.IsNull()){
      m_Timer->stop();
      return;
    }
    m_Device->Update();
    m_Node->SetData(m_Device->GetOutput());

    //start timer
    int interval = (1000 / m_Controls.m_FrameRate->value());
    m_Timer->setInterval(interval);
    m_Timer->start();

    //reinit view
    GlobalReinit();

    //change UI elements
    m_Controls.m_BtnView->setText("Stop Viewing");
    m_Controls.m_FrameRate->setEnabled(false);
    m_Controls.crop_left->setValue(m_Device->GetCropArea().cropLeft);
    m_Controls.crop_right->setValue(m_Device->GetCropArea().cropRight);
    m_Controls.crop_bot->setValue(m_Device->GetCropArea().cropBottom);
    m_Controls.crop_top->setValue(m_Device->GetCropArea().cropTop);
    m_Controls.crop_left->setEnabled(true);
    m_Controls.crop_right->setEnabled(true);
    m_Controls.crop_bot->setEnabled(true);
    m_Controls.crop_top->setEnabled(true);
  }
  else //deactivate imaging
  {
    //stop timer & release data
    m_Timer->stop();
    m_Node->ReleaseData();
    this->RequestRenderWindowUpdate();

    //change UI elements
    m_Controls.m_BtnView->setText("Start Viewing");
    m_Controls.m_FrameRate->setEnabled(true);
    m_Controls.crop_left->setEnabled(false);
    m_Controls.crop_right->setEnabled(false);
    m_Controls.crop_bot->setEnabled(false);
    m_Controls.crop_top->setEnabled(false);
  }
}

void UltrasoundSupport::OnNewDeviceWidgetDone()
{
  m_Controls.m_NewVideoDeviceWidget->setVisible(false);
  m_Controls.m_DeviceManagerWidget->setVisible(true);
  m_Controls.m_AddDevice->setVisible(true);
  m_Controls.m_Headline->setText("Connected Devices:");
}

void UltrasoundSupport::GlobalReinit()
{
  // get all nodes that have not set "includeInBoundingBox" to false
  mitk::NodePredicateNot::Pointer pred = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox", mitk::BoolProperty::New(false)));
  mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);

  // calculate bounding geometry of these nodes
  mitk::TimeGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

  // initialize the views to the bounding geometry
  mitk::RenderingManager::GetInstance()->InitializeViews(bounds);
}

UltrasoundSupport::UltrasoundSupport()
{
  m_DevicePersistence = mitk::USDevicePersistence::New();
  m_DevicePersistence->RestoreLastDevices();
}

UltrasoundSupport::~UltrasoundSupport()
{
  m_DevicePersistence->StoreCurrentDevices();
  m_Controls.m_DeviceManagerWidget->DisconnectAllDevices();
}
