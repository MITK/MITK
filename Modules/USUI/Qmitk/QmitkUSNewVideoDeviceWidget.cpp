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
    connect( m_Controls->m_BtnDone,   SIGNAL(clicked()), this, SLOT(OnClickedDone()) );
    connect( m_Controls->m_BtnCancel, SIGNAL(clicked()), this, SLOT(OnClickedCancel()) );
    connect( m_Controls->m_RadioDeviceSource, SIGNAL(clicked()), this, SLOT(OnDeviceTypeSelection()) );
    connect( m_Controls->m_RadioFileSource,   SIGNAL(clicked()), this, SLOT(OnDeviceTypeSelection()) );

  }
  // Hide & show stuff
  m_Controls->m_FilePathSelector->setVisible(false);
}


///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSNewVideoDeviceWidget::OnClickedDone(){
  m_Active = false;

  // Assemble Metadata
  mitk::USImageMetadata::Pointer metadata = mitk::USImageMetadata::New();
  metadata->SetDeviceComment(m_Controls->m_Comment->text().toStdString());
  metadata->SetDeviceModel(m_Controls->m_Model->text().toStdString());
  metadata->SetDeviceManufacturer(m_Controls->m_Manufacturer->text().toStdString());
  metadata->SetProbeName(m_Controls->m_Probe->text().toStdString());
  metadata->SetZoom(m_Controls->m_Zoom->text().toStdString());


  // Create Device
  mitk::USVideoDevice::Pointer newDevice;
  if (m_Controls->m_RadioDeviceSource->isChecked()){
    int deviceID = m_Controls->m_DeviceSelector->value();
    newDevice = mitk::USVideoDevice::New(deviceID, metadata);
  } else {
    std::string filepath = m_Controls->m_FilePathSelector->text().toStdString();
    newDevice = mitk::USVideoDevice::New(filepath, metadata);
  }

  // Set Video Options
   newDevice->GetSource()->SetColorOutput(! m_Controls->m_CheckGreyscale->isChecked());

  // If Resolution override is activated, apply it
  if (m_Controls->m_CheckResolutionOverride->isChecked())
  {
    int width  = m_Controls->m_ResolutionWidth->value();
    int height = m_Controls->m_ResolutionHeight->value();
    newDevice->GetSource()->OverrideResolution(width, height);
    newDevice->GetSource()->SetResolutionOverride(true);
  }

  newDevice->Connect();

  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnClickedCancel(){
  m_TargetDevice = 0;
  m_Active = false;
  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnDeviceTypeSelection(){
  m_Controls->m_FilePathSelector->setVisible(m_Controls->m_RadioFileSource->isChecked());
  m_Controls->m_DeviceSelector->setVisible(m_Controls->m_RadioDeviceSource->isChecked());
}



///////////////// Methods & Slots Handling Logic //////////////////////////


void QmitkUSNewVideoDeviceWidget::EditDevice(mitk::USDevice::Pointer device)
{
  // If no VideoDevice is given, throw an exception
  if (device->GetDeviceClass().compare("org.mitk.modules.us.USVideoDevice") != 0){
    // TODO Alert if bad path
    mitkThrow() << "NewVideoDevcieWidget recieved an incompatible Device Type to edit. Devicetype was: " << device->GetDeviceClass();
  }
  m_TargetDevice = static_cast<mitk::USVideoDevice*> (device.GetPointer());
  m_Active = true;
}


void QmitkUSNewVideoDeviceWidget::CreateNewDevice()
{
  m_TargetDevice = 0;
  InitFields(mitk::USImageMetadata::New());
  m_Active = true;
}


/////////////////////// HOUSEHOLDING CODE ///////////////////////////////

QListWidgetItem* QmitkUSNewVideoDeviceWidget::ConstructItemFromDevice(mitk::USDevice::Pointer device){
  QListWidgetItem *result = new QListWidgetItem;
  std::string text = device->GetDeviceManufacturer() + "|" + device->GetDeviceModel();
  result->setText(text.c_str());
  return result;
}

void QmitkUSNewVideoDeviceWidget::InitFields(mitk::USImageMetadata::Pointer metadata){
  this->m_Controls->m_Manufacturer->setText (metadata->GetDeviceManufacturer().c_str());
  this->m_Controls->m_Model->setText (metadata->GetDeviceModel().c_str());
  this->m_Controls->m_Comment->setText (metadata->GetDeviceComment().c_str());
  this->m_Controls->m_Probe->setText (metadata->GetProbeName().c_str());
  this->m_Controls->m_Zoom->setText (metadata->GetZoom().c_str());
}
