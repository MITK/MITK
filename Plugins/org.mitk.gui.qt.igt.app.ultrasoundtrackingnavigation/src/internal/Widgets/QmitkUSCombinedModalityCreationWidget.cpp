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
#include "QmitkUSCombinedModalityCreationWidget.h"
#include "ui_QmitkUSCombinedModalityCreationWidget.h"

#include "mitkTrackedUltrasound.h"

QmitkUSCombinedModalityCreationWidget::QmitkUSCombinedModalityCreationWidget(QWidget *parent) :
QWidget(parent),
ui(new Ui::QmitkUSCombinedModalityCreationWidget)
{
  ui->setupUi(this);

  std::string filterExcludeCombinedModalities = "(&(" + us::ServiceConstants::OBJECTCLASS() + "=" + "org.mitk.services.UltrasoundDevice)(!(" + mitk::USDevice::GetPropertyKeys().US_PROPKEY_CLASS + "=" + mitk::USCombinedModality::DeviceClassIdentifier + ")))";

  connect(ui->usDevicesServiceList, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnSelectedUltrasoundOrTrackingDevice()));
  connect(ui->trackingDevicesServiceList, SIGNAL(ServiceSelectionChanged(us::ServiceReferenceU)), this, SLOT(OnSelectedUltrasoundOrTrackingDevice()));

  ui->usDevicesServiceList->SetAutomaticallySelectFirstEntry(true);
  ui->trackingDevicesServiceList->SetAutomaticallySelectFirstEntry(true);

  ui->usDevicesServiceList->Initialize<mitk::USDevice>(mitk::USDevice::GetPropertyKeys().US_PROPKEY_LABEL, filterExcludeCombinedModalities);
  ui->trackingDevicesServiceList->Initialize<mitk::NavigationDataSource>(mitk::NavigationDataSource::US_PROPKEY_DEVICENAME);

  connect(ui->createButton, SIGNAL(clicked()), this, SLOT(OnCreation()));
  connect(ui->cancelButton, SIGNAL(clicked()), this, SIGNAL(SignalAborted()));
}

QmitkUSCombinedModalityCreationWidget::~QmitkUSCombinedModalityCreationWidget()
{
  delete ui;
}

void QmitkUSCombinedModalityCreationWidget::OnCreation()
{
  mitk::USDevice::Pointer usDevice = ui->usDevicesServiceList->GetSelectedService<mitk::USDevice>();
  if (usDevice.IsNull())
  {
    MITK_WARN << "No Ultrasound Device selected for creation of Combined Modality.";
    return;
  }

  mitk::NavigationDataSource::Pointer trackingDevice = ui->trackingDevicesServiceList->GetSelectedService<mitk::NavigationDataSource>();
  if (trackingDevice.IsNull())
  {
    MITK_WARN << "No Traccking Device selected for creation of Combined Modality.";
    return;
  }

  QString vendor = ui->vendorLineEdit->text();
  QString name = ui->nameLineEdit->text();
  if (name.isEmpty()) { name = "Ultrasound Tracker Device"; }

  if( ui->activateTrackedUSCheckbox->isChecked() )
  {
    m_CombinedModality = mitk::TrackedUltrasound::New(usDevice, trackingDevice, true);
    MITK_INFO << "Created TrackedUltrasound device";
  }
  else
  {
    m_CombinedModality = mitk::USCombinedModality::New(usDevice, trackingDevice, false);
    MITK_INFO << "Created CombinedModality device";
  }

  m_CombinedModality->GetUltrasoundDevice()->Initialize();
  m_CombinedModality->RegisterAsMicroservice(); // register as micro service

  emit SignalCreated();
  //emit SignalCreated(m_CombinedModality);
}

void QmitkUSCombinedModalityCreationWidget::OnSelectedUltrasoundOrTrackingDevice()
{
  // create button is enabled only if an ultrasound
  // and a tracking device is selected
  ui->createButton->setEnabled(
    ui->usDevicesServiceList->GetIsServiceSelected()
    && ui->trackingDevicesServiceList->GetIsServiceSelected());
}
