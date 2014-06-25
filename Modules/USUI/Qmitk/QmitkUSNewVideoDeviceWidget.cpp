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
#include <QFileDialog>

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
    connect( m_Controls->m_OpenFileButton, SIGNAL(clicked()), this, SLOT(OnOpenFileButtonClicked()) );
  }
}

///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSNewVideoDeviceWidget::OnClickedDone()
{
  m_Active = false;

  // Create Device
  mitk::USVideoDevice::Pointer newDevice;
  if (m_Controls->m_RadioDeviceSource->isChecked())
  {
    newDevice = mitk::USVideoDevice::New(
      m_Controls->m_DeviceSelector->value(),
      m_Controls->m_Manufacturer->text().toStdString(),
      m_Controls->m_Model->text().toStdString());
    newDevice->SetComment(m_Controls->m_Comment->text().toStdString());
  }
  else
  {
    newDevice = mitk::USVideoDevice::New(
      m_Controls->m_FilePathSelector->text().toStdString(),
      m_Controls->m_Manufacturer->text().toStdString(),
      m_Controls->m_Model->text().toStdString());
    newDevice->SetComment(m_Controls->m_Comment->text().toStdString());
  }

  // get USImageVideoSource from new device
  mitk::USImageVideoSource::Pointer imageSource =
    dynamic_cast<mitk::USImageVideoSource*>(newDevice->GetUSImageSource().GetPointer());
  if ( ! imageSource )
  {
    MITK_ERROR << "There is no USImageVideoSource at the current device.";
    mitkThrow() << "There is no USImageVideoSource at the current device.";
  }

  // Set Video Options
  imageSource->SetColorOutput(! m_Controls->m_CheckGreyscale->isChecked());

  // If Resolution override is activated, apply it
  if (m_Controls->m_CheckResolutionOverride->isChecked())
  {
    int width  = m_Controls->m_ResolutionWidth->value();
    int height = m_Controls->m_ResolutionHeight->value();
    imageSource->OverrideResolution(width, height);
    imageSource->SetResolutionOverride(true);
  }

  newDevice->Initialize();

  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnClickedCancel(){
  m_TargetDevice = 0;
  m_Active = false;
  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnDeviceTypeSelection(){
  m_Controls->m_FilePathSelector->setEnabled(m_Controls->m_RadioFileSource->isChecked());
  m_Controls->m_DeviceSelector->setEnabled(m_Controls->m_RadioDeviceSource->isChecked());
}

void QmitkUSNewVideoDeviceWidget::OnOpenFileButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(NULL, "Open Video File");
  if ( fileName.isNull() ) { return; } // user pressed cancel

  m_Controls->m_FilePathSelector->setText(fileName);

  m_Controls->m_RadioFileSource->setChecked(true);
  this->OnDeviceTypeSelection();
}

///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkUSNewVideoDeviceWidget::EditDevice(mitk::USDevice::Pointer device)
{
  // If no VideoDevice is given, throw an exception
  if (device->GetDeviceClass().compare("org.mitk.modules.us.USVideoDevice") != 0){
    // TODO Alert if bad path
    mitkThrow() << "NewVideoDeviceWidget recieved an incompatible device type to edit. Type was: " << device->GetDeviceClass();
  }
  m_TargetDevice = static_cast<mitk::USVideoDevice*> (device.GetPointer());
  m_Active = true;
}

void QmitkUSNewVideoDeviceWidget::CreateNewDevice()
{
  m_TargetDevice = 0;
  m_Active = true;
}

/////////////////////// HOUSEHOLDING CODE ///////////////////////////////

QListWidgetItem* QmitkUSNewVideoDeviceWidget::ConstructItemFromDevice(mitk::USDevice::Pointer device){
  QListWidgetItem *result = new QListWidgetItem;
  std::string text = device->GetDeviceManufacturer() + "|" + device->GetDeviceModel();
  result->setText(text.c_str());
  return result;
}
