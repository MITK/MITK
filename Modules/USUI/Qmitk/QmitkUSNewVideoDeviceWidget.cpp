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
#include <QMessageBox>

// mitk headers

// itk headers

#include <mitkUSDeviceReaderXML.h>
#include <mitkUSDeviceWriterXML.h>

const std::string QmitkUSNewVideoDeviceWidget::VIEW_ID =
"org.mitk.views.QmitkUSNewVideoDeviceWidget";

QmitkUSNewVideoDeviceWidget::QmitkUSNewVideoDeviceWidget(QWidget* parent,
  Qt::WindowFlags f)
  : QWidget(parent, f)
{
  m_Controls = nullptr;
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

    connect(m_Controls->m_BtnSave, SIGNAL(clicked()), this,
            SLOT(OnSaveButtonClicked()));
    connect(m_Controls->m_BtnLoadConfiguration, SIGNAL(clicked()), this,
      SLOT(OnLoadConfigurationButtonClicked()));

    //Connect buttons and functions for editing of probes
    connect(m_Controls->m_AddNewProbePushButton, SIGNAL(clicked()), this, SLOT(OnAddNewProbeClicked()));
    connect(m_Controls->m_BtnRemoveProbe, SIGNAL(clicked()), this, SLOT(OnClickedRemoveProbe()));
    connect(m_Controls->m_BtnRemoveDepth, SIGNAL(clicked()), this, SLOT(OnClickedRemoveDepth()));
    connect(m_Controls->m_BtnAddDepths, SIGNAL(clicked()), this, SLOT(OnClickedAddDepths()));
    connect(m_Controls->m_Probes, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnProbeChanged(const QString &)));
    connect(m_Controls->m_Depths, SIGNAL(currentTextChanged(const QString &)), this, SLOT(OnDepthChanged(const QString &)));

    connect(m_Controls->m_XSpacingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnXSpacingSpinBoxChanged(double)));
    connect(m_Controls->m_YSpacingSpinBox, SIGNAL(valueChanged(double)), this, SLOT(OnYSpacingSpinBoxChanged(double)));
    connect(m_Controls->m_CroppingTopSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnCroppingTopSpinBoxChanged(int)));
    connect(m_Controls->m_CroppingRightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnCroppingRightSpinBoxChanged(int)));
    connect(m_Controls->m_CroppingBottomSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnCroppingBottomSpinBoxChanged(int)));
    connect(m_Controls->m_CroppingLeftSpinBox, SIGNAL(valueChanged(int)), this, SLOT(OnCroppingLeftSpinBoxChanged(int)));
  }
}

///////////// Methods & Slots Handling Direct Interaction /////////////////

void QmitkUSNewVideoDeviceWidget::OnClickedDone()
{
  m_Active = false;

  // Create Device
  mitk::USDevice::Pointer newDevice;
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
    newDevice = mitk::USIGTLDevice::New(m_Controls->m_Manufacturer->text().toStdString(),
                m_Controls->m_Model->text().toStdString(), host, port, false);
    //New behavior at this position: do not return immediately as it was done in earlier MITK-versions
    // The IGTL Device can have different probe configurations,  as well.
  }
  else
  {
    std::string host = m_Controls->m_OIGTLServerHost->text().toStdString();
    int port = m_Controls->m_OIGTLServerPort->value();

    // Create a new USIGTLDevice. The last parameter tells the device that it should be a server.
    newDevice = mitk::USIGTLDevice::New(m_Controls->m_Manufacturer->text().toStdString(),
                m_Controls->m_Model->text().toStdString(), host, port, true);
    //New behavior at this position: do not return immediately as it was done in earlier MITK-versions
    // The IGTL Device can have different probe configurations,  as well.
  }

  //At first: only ckeck, whether it is a USImageVideoSource or not (--> if it a IGTL Client)
  // Later: perhaps it would be helpful, if the IGTLMessageToUSImageFilter have a region of interest, as well.
  mitk::USImageVideoSource::Pointer imageSource =
    dynamic_cast<mitk::USImageVideoSource*>(
      newDevice->GetUSImageSource().GetPointer());
  if (imageSource.IsNotNull())
  {

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
  }

  if (m_Controls->m_Probes->count() != 0 ) //there are informations about the probes of the device, so create the probes
  {
    this->AddProbesToDevice(newDevice);
  }
  else //no information about the probes of the device, so set default value
  {
    mitk::USProbe::Pointer probe = mitk::USProbe::New("default");
    probe->SetDepth(0);
    newDevice->DeleteAllProbes();
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

  if (m_Controls->m_Probes->count() != 0) //there are informations about the probes of the device, so create the probes
  {
    this->AddProbesToDevice(m_TargetDevice);
  }
  else //no information about the probes of the device, so set default value
  {
    mitk::USProbe::Pointer probe = mitk::USProbe::New("default");
    probe->SetDepth(0);
    m_TargetDevice->DeleteAllProbes();
    m_TargetDevice->AddNewProbe(probe);
  }

  //At first: only ckeck, whether it is a USImageVideoSource or not (--> if it a IGTL Client)
  // Later: perhaps it would be helpful, if the IGTLMessageToUSImageFilter have a region of interest, as well.
  mitk::USImageVideoSource::Pointer imageSource =
    dynamic_cast<mitk::USImageVideoSource*>(
    m_TargetDevice->GetUSImageSource().GetPointer());
  if (imageSource.IsNotNull())
  {
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
  }
  CleanUpAfterEditingOfDevice();
  MITK_INFO << "Finished Editing";
  emit Finished();
}

void QmitkUSNewVideoDeviceWidget::OnClickedCancel()
{
  m_TargetDevice = nullptr;
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
  QString fileName = QFileDialog::getOpenFileName(nullptr, "Open Video File");
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
  if (device->GetDeviceClass().compare("org.mitk.modules.us.USVideoDevice") != 0 &&
      device->GetDeviceClass().compare("IGTL Client") != 0)
  {
    // TODO Alert if bad path
    mitkThrow() << "NewVideoDeviceWidget recieved an incompatible device type "
      "to edit. Type was: " << device->GetDeviceClass();
  }

  m_TargetDevice = device;
  m_Active = true;
  m_ConfigProbes.clear();
  m_ConfigProbes = m_TargetDevice->GetAllProbes();
  ChangeUIEditingUSVideoDevice();
}

void QmitkUSNewVideoDeviceWidget::CreateNewDevice()
{

  //Prevent multiple calls of OnClickedDone()
  disconnect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedDone()));
  //Toggle functionality of Btn_Done
  connect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedDone()));
  m_Controls->m_BtnDone->setText("Add Video Device");

  //Fill Metadata with default information
  m_Controls->m_Manufacturer->setText("Unknown Manufacturer");
  m_Controls->m_Model->setText("Unknown Model");
  m_Controls->m_Comment->setText("None");

  m_TargetDevice = nullptr;
  m_ConfigProbes.clear();
  m_ConfigProbes.clear();
  m_Active = true;
}

/////////////////////// HOUSEHOLDING CODE ///////////////////////////////

QListWidgetItem* QmitkUSNewVideoDeviceWidget::ConstructItemFromDevice(
  mitk::USDevice::Pointer device)
{
  QListWidgetItem* result = new QListWidgetItem;
  std::string text =
    device->GetManufacturer() + "|" + device->GetName();
  result->setText(text.c_str());
  return result;
}

void QmitkUSNewVideoDeviceWidget::ChangeUIEditingUSVideoDevice()
{
  for (std::vector<mitk::USProbe::Pointer>::iterator it = m_ConfigProbes.begin(); it != m_ConfigProbes.end(); it++)
  {
    std::string probeName = (*it)->GetName();
    m_Controls->m_Probes->addItem(QString::fromUtf8(probeName.data(), probeName.size()));
  }
  OnProbeChanged(m_Controls->m_Probes->currentText());

  //Toggle functionality of Btn_Done
  m_Controls->m_BtnDone->setText("Apply Changes");
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
    QString depths = m_Controls->m_AddDepths->text();
    if( depths.isEmpty() )
      return;

    std::string probename = m_Controls->m_Probes->currentText().toStdString();
    mitk::USProbe::Pointer currentProbe = this->CheckIfProbeExistsAlready(probename);

    QStringList singleDepths = depths.split(',');
    for (int i = 0; i < singleDepths.size(); i++)
    {
      currentProbe->SetDepth(singleDepths.at(i).toInt());
    }
    m_Controls->m_AddDepths->clear();
    m_Controls->m_Depths->setEnabled(true);
    m_Controls->m_BtnRemoveDepth->setEnabled(true);
    OnProbeChanged(m_Controls->m_Probes->currentText());
  }
}

void QmitkUSNewVideoDeviceWidget::OnClickedRemoveDepth()
{
  if (!m_Controls->m_Probes->currentText().isEmpty() && !m_Controls->m_Depths->currentText().isEmpty())
  {
    std::string probename = m_Controls->m_Probes->currentText().toStdString();
    int indexOfDepthToRemove = m_Controls->m_Depths->currentIndex();
    mitk::USProbe::Pointer currentProbe = this->CheckIfProbeExistsAlready(probename);
    currentProbe->RemoveDepth(m_Controls->m_Depths->currentText().toInt());
    m_Controls->m_Depths->removeItem(indexOfDepthToRemove);

    if (m_Controls->m_Depths->count() == 0)
    {
      m_Controls->m_Depths->setEnabled(false);
      m_Controls->m_BtnRemoveDepth->setEnabled(false);

      this->EnableDisableSpacingAndCropping(false);
    }
  }
}

void QmitkUSNewVideoDeviceWidget::OnClickedRemoveProbe()
{
  if (!m_Controls->m_Probes->currentText().isEmpty())
  {
    std::string probename = m_Controls->m_Probes->currentText().toStdString();
    int indexOfProbeToRemove = m_Controls->m_Probes->currentIndex();
    m_ConfigProbes.erase(m_ConfigProbes.begin() + indexOfProbeToRemove);
    m_Controls->m_Probes->removeItem(indexOfProbeToRemove);
    if( m_Controls->m_Probes->count() == 0 )
    {
      m_Controls->m_Probes->setEnabled(false);
      m_Controls->m_BtnRemoveProbe->setEnabled(false);
      m_Controls->m_BtnAddDepths->setEnabled(false);
      m_Controls->m_AddDepths->setEnabled(false);
      m_Controls->m_Depths->setEnabled(false);
      m_Controls->m_BtnRemoveDepth->setEnabled(false);

      this->EnableDisableSpacingAndCropping(false);
    }
  }
}

void QmitkUSNewVideoDeviceWidget::OnProbeChanged(const QString & probename)
{
  if (!probename.isEmpty())
  {
    std::string name = probename.toStdString();
    mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(name);
    if( probe.IsNotNull() )
    {
      std::map<int, mitk::Vector3D> depths = probe->GetDepthsAndSpacing();
      m_Controls->m_Depths->clear();
      for (std::map<int, mitk::Vector3D>::iterator it = depths.begin(); it != depths.end(); it++)
      {
        m_Controls->m_Depths->addItem(QString::number(it->first));
      }

      this->OnDepthChanged(m_Controls->m_Depths->currentText().toInt(), probe);
    }
  }
  else
  {
    m_Controls->m_Depths->clear();
    m_Controls->m_Depths->setEnabled(false);
    m_Controls->m_BtnRemoveDepth->setEnabled(false);
  }
}

void QmitkUSNewVideoDeviceWidget::OnDepthChanged(int depth, mitk::USProbe::Pointer probe)
{
  if (m_Controls->m_Depths->count() == 0)
  {
    m_Controls->m_Depths->setEnabled(false);
    m_Controls->m_BtnRemoveDepth->setEnabled(false);

    this->EnableDisableSpacingAndCropping(false);
    return;
  }

  if (probe.IsNotNull())
  {
    mitk::Vector3D spacing = probe->GetSpacingForGivenDepth(depth);
    m_Controls->m_XSpacingSpinBox->setValue(spacing[0]);
    m_Controls->m_YSpacingSpinBox->setValue(spacing[1]);

    mitk::USProbe::USProbeCropping cropping = probe->GetProbeCropping();
    m_Controls->m_CroppingTopSpinBox->setValue(cropping.top);
    m_Controls->m_CroppingRightSpinBox->setValue(cropping.right);
    m_Controls->m_CroppingBottomSpinBox->setValue(cropping.bottom);
    m_Controls->m_CroppingLeftSpinBox->setValue(cropping.left);

    this->EnableDisableSpacingAndCropping(true);

    m_Controls->m_Depths->setEnabled(true);
    m_Controls->m_BtnRemoveDepth->setEnabled(true);
    m_Controls->m_AddDepths->setEnabled(true);
    m_Controls->m_BtnAddDepths->setEnabled(true);
    m_Controls->m_Probes->setEnabled(true);
    m_Controls->m_BtnRemoveProbe->setEnabled(true);
  }
}

void QmitkUSNewVideoDeviceWidget::OnDepthChanged(const QString &depth)
{
  MITK_INFO << "OnDepthChanged(int, mitk::USProbe)";
  if( depth.isEmpty() )
  {
    this->EnableDisableSpacingAndCropping(false);
    return;
  }
  QString probeName = m_Controls->m_Probes->currentText();

  this->OnDepthChanged(depth.toInt(), this->CheckIfProbeExistsAlready(probeName.toStdString()) );
}

void QmitkUSNewVideoDeviceWidget::OnSaveButtonClicked()
{
  QString fileName = QFileDialog::getSaveFileName(nullptr, "Save Configuration ...", "", "XML files (*.xml)");
  if( fileName.isNull() )
  {
    return;
  }  // user pressed cancel

  mitk::USDeviceWriterXML deviceWriter;
  deviceWriter.SetFilename(fileName.toStdString());

  mitk::USDeviceReaderXML::USDeviceConfigData config;
  this->CollectUltrasoundDeviceConfigInformation(config);

  if (!deviceWriter.WriteUltrasoundDeviceConfiguration(config))
  {
    QMessageBox msgBox;
    msgBox.setText("Error when writing the configuration to the selected file. Could not write device information.");
    msgBox.exec();
    return;
  }

}

void QmitkUSNewVideoDeviceWidget::OnLoadConfigurationButtonClicked()
{
  QString fileName = QFileDialog::getOpenFileName(this, "Open ultrasound device configuration ...");
  if (fileName.isNull())
  {
    return;
  }  // user pressed cancel

  mitk::USDeviceReaderXML deviceReader;
  deviceReader.SetFilename(fileName.toStdString());
  if (!deviceReader.ReadUltrasoundDeviceConfiguration())
  {
    QMessageBox msgBox;
    msgBox.setText("Error when parsing the selected file. Could not load stored device information.");
    msgBox.exec();
    return;
  }
  mitk::USDeviceReaderXML::USDeviceConfigData config = deviceReader.GetUSDeviceConfigData();

  if( config.fileversion == 1.0 )
  {
    if (config.deviceType.compare("video") == 0)
    {
      //Fill info in metadata groupbox:
      m_Controls->m_DeviceName->setText(QString::fromStdString(config.deviceName));
      m_Controls->m_Manufacturer->setText(QString::fromStdString(config.manufacturer));
      m_Controls->m_Model->setText(QString::fromStdString(config.model));
      m_Controls->m_Comment->setText(QString::fromStdString(config.comment));

      //Fill info about video source:
      m_Controls->m_DeviceSelector->setValue(config.sourceID);
      m_Controls->m_FilePathSelector->setText(QString::fromStdString(config.filepathVideoSource));

      //Fill video options:
      m_Controls->m_CheckGreyscale->setChecked(config.useGreyscale);

      //Fill override options:
      m_Controls->m_CheckResolutionOverride->setChecked(config.useResolutionOverride);
      m_Controls->m_ResolutionWidth->setValue(config.resolutionWidth);
      m_Controls->m_ResolutionHeight->setValue(config.resolutionHeight);

      //Fill information about probes:
      m_ConfigProbes.clear();
      m_ConfigProbes = config.probes;

      m_Controls->m_Probes->clear();
      m_Controls->m_ProbeNameLineEdit->clear();
      m_Controls->m_AddDepths->clear();
      m_Controls->m_Depths->clear();

      for( size_t index = 0; index < m_ConfigProbes.size(); ++index)
      {
        m_Controls->m_Probes->addItem(QString::fromStdString(config.probes.at(index)->GetName()));
      }
      this->OnProbeChanged(m_Controls->m_Probes->currentText());

    }
    else if (config.deviceType.compare("oigtl") == 0)
    {
      //Fill info in metadata groupbox:
      m_Controls->m_DeviceName->setText(QString::fromStdString(config.deviceName));
      m_Controls->m_Manufacturer->setText(QString::fromStdString(config.manufacturer));
      m_Controls->m_Model->setText(QString::fromStdString(config.model));
      m_Controls->m_Comment->setText(QString::fromStdString(config.comment));

      //Fill info about OpenIGTLink video source:
      if (config.server)
      {
        m_Controls->m_RadioOIGTLServerSource->setChecked(true);
        m_Controls->m_OIGTLServerHost->setText(QString::fromStdString(config.host));
        m_Controls->m_OIGTLServerPort->setValue(config.port);
      }
      else
      {
        m_Controls->m_RadioOIGTLClientSource->setChecked(true);
        m_Controls->m_OIGTLClientHost->setText(QString::fromStdString(config.host));
        m_Controls->m_OIGTLClientPort->setValue(config.port);
      }
      this->OnDeviceTypeSelection();

      //Fill video options:
      m_Controls->m_CheckGreyscale->setChecked(config.useGreyscale);

      //Fill override options:
      m_Controls->m_CheckResolutionOverride->setChecked(config.useResolutionOverride);
      m_Controls->m_ResolutionWidth->setValue(config.resolutionWidth);
      m_Controls->m_ResolutionHeight->setValue(config.resolutionHeight);

      //Fill information about probes:
      m_ConfigProbes.clear();
      m_ConfigProbes = config.probes;

      m_Controls->m_Probes->clear();
      m_Controls->m_ProbeNameLineEdit->clear();
      m_Controls->m_AddDepths->clear();
      m_Controls->m_Depths->clear();

      for (size_t index = 0; index < m_ConfigProbes.size(); ++index)
      {
        m_Controls->m_Probes->addItem(QString::fromStdString(config.probes.at(index)->GetName()));
      }
      this->OnProbeChanged(m_Controls->m_Probes->currentText());
    }
    else
    {
      MITK_WARN << "Unknown device type detected. The device type must be of type |video|";
    }
  }
  else
  {
    MITK_WARN << "Unknown fileversion. Only fileversion 1.0 is known to the system.";
  }
}

void QmitkUSNewVideoDeviceWidget::OnAddNewProbeClicked()
{
  QString probeName = m_Controls->m_ProbeNameLineEdit->text();
  probeName = probeName.trimmed();
  if (probeName.isEmpty())
  {
    m_Controls->m_ProbeNameLineEdit->clear();
    return;
  }

  if( this->CheckIfProbeExistsAlready(probeName.toStdString() ) != nullptr )
  {
    QMessageBox msgBox;
    msgBox.setText("Probe name already exists. Please choose another name for the probe.");
    msgBox.exec();
    m_Controls->m_ProbeNameLineEdit->clear();
  }
  else
  {
    mitk::USProbe::Pointer newProbe = mitk::USProbe::New(probeName.toStdString());
    m_ConfigProbes.push_back(newProbe);
    m_Controls->m_Probes->addItem(QString::fromStdString(probeName.toStdString()));

    m_Controls->m_Probes->setEnabled(true);
    m_Controls->m_BtnRemoveProbe->setEnabled(true);
    m_Controls->m_BtnAddDepths->setEnabled(true);
    m_Controls->m_AddDepths->setEnabled(true);
    m_Controls->m_ProbeNameLineEdit->clear();
  }
}

void QmitkUSNewVideoDeviceWidget::OnXSpacingSpinBoxChanged(double value)
{
  MITK_INFO << "Changing x-spacing to: " << value;
  QString probeName = m_Controls->m_Probes->currentText();
  int depth = m_Controls->m_Depths->currentText().toInt();

  mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(probeName.toStdString());
  if (probe.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("An error occurred when changing the spacing. \
      The specified probe does not exist. \
      Please restart the configuration process.");
    msgBox.exec();
    return;
  }

  mitk::Vector3D spacing = probe->GetSpacingForGivenDepth(depth);
  spacing[0] = value;
  probe->SetSpacingForGivenDepth(depth, spacing);
}

void QmitkUSNewVideoDeviceWidget::OnYSpacingSpinBoxChanged(double value)
{
  MITK_INFO << "Changing y-spacing to: " << value;
  QString probeName = m_Controls->m_Probes->currentText();
  int depth = m_Controls->m_Depths->currentText().toInt();

  mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(probeName.toStdString());
  if (probe.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("An error occurred when changing the spacing. \
      The specified probe does not exist. \
      Please restart the configuration process.");
    msgBox.exec();
    return;
  }

  mitk::Vector3D spacing = probe->GetSpacingForGivenDepth(depth);
  spacing[1] = value;
  probe->SetSpacingForGivenDepth(depth, spacing);
}

void QmitkUSNewVideoDeviceWidget::OnCroppingTopSpinBoxChanged(int value)
{
  MITK_INFO << "Changing cropping top to: " << value;
  QString probeName = m_Controls->m_Probes->currentText();

  mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(probeName.toStdString());
  if (probe.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("An error occurred when changing the probe cropping. \
      The specified probe does not exist. \
      Please restart the configuration process.");
    msgBox.exec();
    return;
  }

  mitk::USProbe::USProbeCropping cropping = probe->GetProbeCropping();
  probe->SetProbeCropping(value, cropping.bottom, cropping.left, cropping.right);
}

void QmitkUSNewVideoDeviceWidget::OnCroppingRightSpinBoxChanged(int value)
{
  MITK_INFO << "Changing cropping right to: " << value;
  QString probeName = m_Controls->m_Probes->currentText();

  mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(probeName.toStdString());
  if (probe.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("An error occurred when changing the probe cropping. \
      The specified probe does not exist. \
      Please restart the configuration process.");
    msgBox.exec();
    return;
  }

  mitk::USProbe::USProbeCropping cropping = probe->GetProbeCropping();
  probe->SetProbeCropping(cropping.top, cropping.bottom, cropping.left, value);
}

void QmitkUSNewVideoDeviceWidget::OnCroppingBottomSpinBoxChanged(int value)
{
  MITK_INFO << "Changing cropping bottom to: " << value;
  QString probeName = m_Controls->m_Probes->currentText();

  mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(probeName.toStdString());
  if (probe.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("An error occurred when changing the probe cropping. \
      The specified probe does not exist. \
      Please restart the configuration process.");
    msgBox.exec();
    return;
  }

  mitk::USProbe::USProbeCropping cropping = probe->GetProbeCropping();
  probe->SetProbeCropping(cropping.top, value, cropping.left, cropping.right);
}

void QmitkUSNewVideoDeviceWidget::OnCroppingLeftSpinBoxChanged(int value)
{
  MITK_INFO << "Changing cropping left to: " << value;
  QString probeName = m_Controls->m_Probes->currentText();

  mitk::USProbe::Pointer probe = this->CheckIfProbeExistsAlready(probeName.toStdString());
  if (probe.IsNull())
  {
    QMessageBox msgBox;
    msgBox.setText("An error occurred when changing the probe cropping. \
      The specified probe does not exist. \
      Please restart the configuration process.");
    msgBox.exec();
    return;
  }

  mitk::USProbe::USProbeCropping cropping = probe->GetProbeCropping();
  probe->SetProbeCropping(cropping.top, cropping.bottom, value, cropping.right);
}

void QmitkUSNewVideoDeviceWidget::CleanUpAfterCreatingNewDevice()
{
  disconnect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedDone()));
  m_Controls->m_Probes->clear();
  m_Controls->m_Depths->clear();
  m_Controls->m_AddDepths->clear();
  m_Controls->m_ProbeNameLineEdit->clear();
  m_ConfigProbes.clear();
}

void QmitkUSNewVideoDeviceWidget::CleanUpAfterEditingOfDevice()
{
  disconnect(m_Controls->m_BtnDone, SIGNAL(clicked()), this, SLOT(OnClickedFinishedEditing()));
  m_Controls->m_Probes->clear();
  m_Controls->m_Depths->clear();
  m_Controls->m_AddDepths->clear();
  m_ConfigProbes.clear();
}

void QmitkUSNewVideoDeviceWidget::AddProbesToDevice(mitk::USDevice::Pointer device)
{
  device->DeleteAllProbes();
  for( std::vector<mitk::USProbe::Pointer>::iterator it = m_ConfigProbes.begin();
    it != m_ConfigProbes.end(); it++)
  {
    if ((*it)->IsDepthAndSpacingEmpty())
    {
      (*it)->SetDepth(0);
    }
    device->AddNewProbe((*it));
  }
}

mitk::USProbe::Pointer QmitkUSNewVideoDeviceWidget::CheckIfProbeExistsAlready(const std::string &probeName)
{
  for( std::vector<mitk::USProbe::Pointer>::iterator it = m_ConfigProbes.begin();
       it != m_ConfigProbes.end(); it++ )
  {
    if( probeName.compare((*it)->GetName()) == 0)
      return (*it);
  }
  return nullptr; //no matching probe was found so nullptr is returned
}

void QmitkUSNewVideoDeviceWidget::CollectUltrasoundDeviceConfigInformation(mitk::USDeviceReaderXML::USDeviceConfigData &config)
{
  config.fileversion = 1.0;
  if (m_Controls->m_RadioDeviceSource->isChecked() || m_Controls->m_RadioFileSource->isChecked())
  {
    //Fill info about video source:
    config.deviceType = "video";
    config.sourceID = m_Controls->m_DeviceSelector->value();
    config.filepathVideoSource = m_Controls->m_FilePathSelector->text().toStdString();
  }
  else
  {
    config.deviceType = "oigtl";
    if (m_Controls->m_RadioOIGTLServerSource->isChecked())
    {
      config.server = true;
      config.host = m_Controls->m_OIGTLServerHost->text().toStdString();
      config.port = m_Controls->m_OIGTLServerPort->value();
    }
    else
    {
      config.server = false;
      config.host = m_Controls->m_OIGTLClientHost->text().toStdString();
      config.port = m_Controls->m_OIGTLClientPort->value();
    }
  }

  //Fill info in metadata groupbox:
  config.deviceName = m_Controls->m_DeviceName->text().toStdString();
  config.manufacturer = m_Controls->m_Manufacturer->text().toStdString();
  config.model = m_Controls->m_Model->text().toStdString();
  config.comment = m_Controls->m_Comment->text().toStdString();

  //Fill video options:
  config.useGreyscale = m_Controls->m_CheckGreyscale->isChecked();

  //Fill override options:
  config.useResolutionOverride = m_Controls->m_CheckResolutionOverride->isChecked();
  config.resolutionWidth = m_Controls->m_ResolutionWidth->value();
  config.resolutionHeight = m_Controls->m_ResolutionHeight->value();

  //Fill information about probes:
  config.probes = m_ConfigProbes;
}

void QmitkUSNewVideoDeviceWidget::EnableDisableSpacingAndCropping(bool enable)
{
  m_Controls->m_XSpacingSpinBox->setEnabled(enable);
  m_Controls->m_YSpacingSpinBox->setEnabled(enable);
  m_Controls->m_XSpacingLabel->setEnabled(enable);
  m_Controls->m_YSpacingLabel->setEnabled(enable);

  m_Controls->m_CroppingTopSpinBox->setEnabled(enable);
  m_Controls->m_CroppingRightSpinBox->setEnabled(enable);
  m_Controls->m_CroppingBottomSpinBox->setEnabled(enable);
  m_Controls->m_CroppingLeftSpinBox->setEnabled(enable);
  m_Controls->m_CroppingTopLabel->setEnabled(enable);
  m_Controls->m_CroppingBottomLabel->setEnabled(enable);
  m_Controls->m_CroppingLeftLabel->setEnabled(enable);
  m_Controls->m_CroppingRightLabel->setEnabled(enable);
}
