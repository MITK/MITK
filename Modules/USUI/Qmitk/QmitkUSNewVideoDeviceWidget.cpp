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

// QT headers
#include <QFileDialog>

// mitk headers

// itk headers

const std::string QmitkUSNewVideoDeviceWidget::VIEW_ID =
"org.mitk.views.QmitkUSNewVideoDeviceWidget";

QmitkUSNewVideoDeviceWidget::QmitkUSNewVideoDeviceWidget(QWidget* parent,
  Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkUSNewVideoDeviceWidget::~QmitkUSNewVideoDeviceWidget() {}

//////////////////// INITIALIZATION /////////////////////

void QmitkUSNewVideoDeviceWidget::CreateQtPartControl(QWidget* parent)
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
  if (m_Controls)
  {
    // connect(m_Controls->m_BtnDone, SIGNAL(clicked()), this,
    //  SLOT(OnClickedDone()));
    connect(m_Controls->m_BtnCancel, SIGNAL(clicked()), this,
      SLOT(OnClickedCancel()));
    connect(m_Controls->m_RadioDeviceSource, SIGNAL(clicked()), this,
      SLOT(OnDeviceTypeSelection()));
    connect(m_Controls->m_RadioFileSource, SIGNAL(clicked()), this,
      SLOT(OnDeviceTypeSelection()));
    connect(m_Controls->m_RadioOIGTLClientSource, SIGNAL(clicked()), this,
      SLOT(OnDeviceTypeSelection()));
    connect(m_Controls->m_RadioOIGTLServerSource, SIGNAL(clicked()), this,
      SLOT(OnDeviceTypeSelection()));
    connect(m_Controls->m_OpenFileButton, SIGNAL(clicked()), this,
      SLOT(OnOpenFileButtonClicked()));

    //Connect buttons and functions for editing of probes
    connect(m_Controls->m_BtnRemoveProbe, SIGNAL(clicked()), this, SLOT(OnClickedRemoveProbe()));
    connect(m_Controls->m_BtnRemoveDepth, SIGNAL(clicked()), this, SLOT(OnClickedRemoveDepth()));
    connect(m_Controls->m_BtnAddDepths, SIGNAL(clicked()), this, SLOT(OnClickedAddDepths()));
    connect(m_Controls->m_Probes, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnProbeChanged(const QString &)));
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
  else if (m_Controls->m_RadioFileSource->isChecked())
  {
    newDevice = mitk::USVideoDevice::New(
      m_Controls->m_FilePathSelector->text().toStdString(),
      m_Controls->m_Manufacturer->text().toStdString(),
      m_Controls->m_Model->text().toStdString());
    newDevice->SetComment(m_Controls->m_Comment->text().toStdString());
  }
  else if (m_Controls->m_RadioOIGTLClientSource->isChecked())
  {
    std::string host = m_Controls->m_OIGTLClientHost->text().toStdString();
    int port = m_Controls->m_OIGTLClientPort->value();

    // Create a new USIGTLDevice. The last parameter tells the device that it should be a client.
    mitk::USIGTLDevice::Pointer device =
      mitk::USIGTLDevice::New(m_Controls->m_Manufacturer->text().toStdString(),
      m_Controls->m_Model->text().toStdString(), host, port, false);
    device->Initialize();
    emit Finished();
    // The rest of this method does stuff that is specific to USVideoDevices,
    // which we don't need. So we return directly.
    return;
  }
  else
  {
    std::string host = m_Controls->m_OIGTLServerHost->text().toStdString();
    int port = m_Controls->m_OIGTLServerPort->value();

    // Create a new USIGTLDevice. The last parameter tells the device that it should be a server.
    mitk::USIGTLDevice::Pointer device =
      mitk::USIGTLDevice::New(m_Controls->m_Manufacturer->text().toStdString(),
      m_Controls->m_Model->text().toStdString(), host, port, true);
    device->Initialize();
    emit Finished();
    // The rest of this method does stuff that is specific to USVideoDevices,
    // which we don't need. So we return directly.
    return;
  }

  // get USImageVideoSource from new device
  mitk::USImageVideoSource::Pointer imageSource =
    dynamic_cast<mitk::USImageVideoSource*>(
    newDevice->GetUSImageSource().GetPointer());
  if (!imageSource)
  {
    MITK_ERROR << "There is no USImageVideoSource at the current device.";
    mitkThrow() << "There is no USImageVideoSource at the current device.";
  }

  // Set Video Options
  imageSource->SetColorOutput(!m_Controls->m_CheckGreyscale->isChecked());

  // If Resolution override is activated, apply it
  if (m_Controls->m_CheckResolutionOverride->isChecked())
  {
    int width = m_Controls->m_ResolutionWidth->value();
    int height = m_Controls->m_ResolutionHeight->value();
    imageSource->OverrideResolution(width, height);
    imageSource->SetResolutionOverride(true);
  }
  if (!m_Controls->m_ProbesInformation->text().isEmpty()) //there are informations about the probes of the device, so create the probes
  {
    AddProbesToDevice(newDevice);
  }
  else //no information about the probes of the device, so set default value
  {
    mitk::USProbe::Pointer probe = mitk::USProbe::New("default");
    probe->SetDepth(0);
    newDevice->AddNewProbe(probe);
  }
  newDevice->Initialize();
  CleanUpAfterCreatingNewDevice();

  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnClickedFinishedEditing()
{
  m_Active = false;
  m_TargetDevice->SetManufacturer(m_Controls->m_Manufacturer->text().toStdString());
  m_TargetDevice->SetName(m_Controls->m_Model->text().toStdString());
  m_TargetDevice->SetComment(m_Controls->m_Comment->text().toStdString());

  if (!m_Controls->m_ProbesInformation->text().isEmpty()){ //there is information about probes to add, so add them
    AddProbesToDevice(m_TargetDevice);
  }
  mitk::USImageVideoSource::Pointer imageSource =
    dynamic_cast<mitk::USImageVideoSource*>(
    m_TargetDevice->GetUSImageSource().GetPointer());
  if (!imageSource)
  {
    MITK_ERROR << "There is no USImageVideoSource at the current device.";
    mitkThrow() << "There is no USImageVideoSource at the current device.";
  }

  // Set Video Options
  imageSource->SetColorOutput(!m_Controls->m_CheckGreyscale->isChecked());

  // If Resolution override is activated, apply it
  if (m_Controls->m_CheckResolutionOverride->isChecked())
  {
    int width = m_Controls->m_ResolutionWidth->value();
    int height = m_Controls->m_ResolutionHeight->value();
    imageSource->OverrideResolution(width, height);
    imageSource->SetResolutionOverride(true);
  }
  CleanUpAfterEditingOfDevice();
  MITK_INFO << "Finished Editing";
  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnClickedCancel()
{
  m_TargetDevice = 0;
  m_Active = false;
  CleanUpAfterCreatingNewDevice();
  CleanUpAfterEditingOfDevice();
  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnDeviceTypeSelection()
{
  m_Controls->m_FilePathSelector->setEnabled(
    m_Controls->m_RadioFileSource->isChecked());
  m_Controls->m_DeviceSelector->setEnabled(
    m_Controls->m_RadioDeviceSource->isChecked());
  m_Controls->m_OIGTLClientHost->setEnabled(
    m_Controls->m_RadioOIGTLClientSource->isChecked());
  m_Controls->m_OIGTLClientPort->setEnabled(
    m_Controls->m_RadioOIGTLClientSource->isChecked());
  m_Controls->m_OIGTLServerHost->setEnabled(
    m_Controls->m_RadioOIGTLServerSource->isChecked());
  m_Controls->m_OIGTLServerPort->setEnabled(
    m_Controls->m_RadioOIGTLServerSource->isChecked());
}

void QmitkUSNewVideoDeviceWidget::OnOpenFileButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(NULL, "Open Video File");
  if (fileName.isNull())
  {
    return;
  }  // user pressed cancel

  m_Controls->m_FilePathSelector->setText(fileName);

  m_Controls->m_RadioFileSource->setChecked(true);
  this->OnDeviceTypeSelection();
}

///////////////// Methods & Slots Handling Logic //////////////////////////

void QmitkUSNewVideoDeviceWidget::EditDevice(mitk::USDevice::Pointer device)
{
  // If no VideoDevice is given, throw an exception
  if (device->GetDeviceClass().compare("org.mitk.modules.us.USVideoDevice") !=
    0)
  {
    // TODO Alert if bad path
    mitkThrow() << "NewVideoDeviceWidget recieved an incompatible device type "
      "to edit. Type was: " << device->GetDeviceClass();
  }
  m_TargetDevice = static_cast<mitk::USVideoDevice*>(device.GetPointer());
  m_Active = true;

  ChangeUIEditingUSVideoDevice();
}

void QmitkUSNewVideoDeviceWidget::CreateNewDevice()
{
  //When new device is created there are no probes to edit, therefore disable the Groupbox
  m_Controls->m_GroupBoxEditProbes->setEnabled(false);

  //Toggle functionality of Btn_Done
  connect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedDone()));
  m_Controls->m_BtnDone->setText("Add Video Device");

  //Fill Metadata with default information
  m_Controls->m_Manufacturer->setText("Unknown Manufacturer");
  m_Controls->m_Model->setText("Unknown Model");
  m_Controls->m_Comment->setText("None");

  m_TargetDevice = 0;
  m_Active = true;
}

/////////////////////// HOUSEHOLDING CODE ///////////////////////////////

QListWidgetItem* QmitkUSNewVideoDeviceWidget::ConstructItemFromDevice(
  mitk::USDevice::Pointer device)
{
  QListWidgetItem* result = new QListWidgetItem;
  std::string text =
    device->GetDeviceManufacturer() + "|" + device->GetDeviceModel();
  result->setText(text.c_str());
  return result;
}

void QmitkUSNewVideoDeviceWidget::ChangeUIEditingUSVideoDevice()
{
  //deactivate the group box containing Videosource options because they should not be changed
  m_Controls->m_GroupBoxVideoSource->setEnabled(false);

  //activate the groupbox contaning the options to edit the probes of the device and fill it with information
  m_Controls->m_GroupBoxEditProbes->setEnabled(true);
  std::vector<mitk::USProbe::Pointer> probes = m_TargetDevice->GetAllProbes();
  for (std::vector<mitk::USProbe::Pointer>::iterator it = probes.begin(); it != probes.end(); it++)
  {
    std::string probeName = (*it)->GetName();
    m_Controls->m_Probes->addItem(QString::fromUtf8(probeName.data(), probeName.size()));
  }
  OnProbeChanged(m_Controls->m_Probes->currentText());

  //Toggle functionality of Btn_Done
  m_Controls->m_BtnDone->setText("Save Changes");
  connect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedFinishedEditing()));

  //Fill Metadata with Information provided by the Device selected to edit
  m_Controls->m_Manufacturer->setText(m_TargetDevice->GetManufacturer().c_str());
  m_Controls->m_Model->setText(m_TargetDevice->GetName().c_str());
  m_Controls->m_Comment->setText(m_TargetDevice->GetComment().c_str());
}

void QmitkUSNewVideoDeviceWidget::OnClickedAddDepths()
{
  if (!m_Controls->m_Probes->currentText().isEmpty())
  {
    std::string probename = m_Controls->m_Probes->currentText().toStdString();
    mitk::USProbe::Pointer currentProbe = m_TargetDevice->GetProbeByName(probename);
    QString depths = m_Controls->m_AddDepths->text();
    QStringList singleDepths = depths.split(',');
    for (int i = 0; i < singleDepths.size(); i++)
    {
      currentProbe->SetDepth(singleDepths.at(i).toInt());
    }
    m_Controls->m_AddDepths->clear();
    OnProbeChanged(m_Controls->m_Probes->currentText());
  }
}

void QmitkUSNewVideoDeviceWidget::OnClickedRemoveDepth()
{
  if (!m_Controls->m_Probes->currentText().isEmpty() && !m_Controls->m_Depths->currentText().isEmpty())
  {
    std::string probename = m_Controls->m_Probes->currentText().toStdString();
    int indexOfDepthToRemove = m_Controls->m_Depths->currentIndex();
    mitk::USProbe::Pointer currentProbe = m_TargetDevice->GetProbeByName(probename);
    currentProbe->RemoveDepth(m_Controls->m_Depths->currentText().toInt());
    m_Controls->m_Depths->removeItem(indexOfDepthToRemove);
  }
}

void QmitkUSNewVideoDeviceWidget::OnClickedRemoveProbe()
{
  if (!m_Controls->m_Probes->currentText().isEmpty())
  {
    std::string probename = m_Controls->m_Probes->currentText().toStdString();
    int indexOfProbeToRemove = m_Controls->m_Probes->currentIndex();
    m_TargetDevice->RemoveProbeByName(probename);
    m_Controls->m_Probes->removeItem(indexOfProbeToRemove);
  }
}

void QmitkUSNewVideoDeviceWidget::OnProbeChanged(const QString & probename)
{
  if (!probename.isEmpty())
  {
    std::string name = probename.toStdString();
    mitk::USProbe::Pointer probe = m_TargetDevice->GetProbeByName(name);
    std::map<int, mitk::Vector3D> depths = probe->GetDepthsAndSpacing();
    m_Controls->m_Depths->clear();
    for (std::map<int, mitk::Vector3D>::iterator it = depths.begin(); it != depths.end(); it++)
    {
      m_Controls->m_Depths->addItem(QString::number(it->first));
    }
  }
}

void QmitkUSNewVideoDeviceWidget::CleanUpAfterCreatingNewDevice()
{
  disconnect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedDone()));
  m_Controls->m_ProbesInformation->clear();
}

void QmitkUSNewVideoDeviceWidget::CleanUpAfterEditingOfDevice()
{
  disconnect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedFinishedEditing()));
  m_Controls->m_Probes->clear();
  m_Controls->m_Depths->clear();
  m_Controls->m_AddDepths->clear();
  m_Controls->m_ProbesInformation->clear();
}

void QmitkUSNewVideoDeviceWidget::AddProbesToDevice(mitk::USVideoDevice::Pointer device)
{
  QString probesInformation = m_Controls->m_ProbesInformation->text();
  QStringList probes = probesInformation.split(';'); //split the different probes
  for (int i = 0; i < probes.size(); i++)
  {
    QStringList depths = probes.at(i).split(','); //now for every probe split the probe name and the different depths
    mitk::USProbe::Pointer probe = mitk::USProbe::New();
    probe->SetName(depths.at(0).toStdString()); //first element is the probe name
    for (int i = 1; i < depths.size(); i++) //all the other elements are the depths for the specific probe so add them to the probe
    {
      probe->SetDepth(depths.at(i).toInt());
    }
    device->AddNewProbe(probe);
  }
}
