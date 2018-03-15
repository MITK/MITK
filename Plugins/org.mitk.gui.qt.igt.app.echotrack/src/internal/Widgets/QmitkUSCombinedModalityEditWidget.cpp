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
#include "QmitkUSCombinedModalityEditWidget.h"
#include "ui_QmitkUSCombinedModalityEditWidget.h"

#include "mitkUSControlInterfaceBMode.h"
#include "mitkUSControlInterfaceProbes.h"

#include "../QmitkUSNavigationCalibrationsDataModel.h"
#include "../QmitkUSNavigationCalibrationRemoveDelegate.h"
#include "../QmitkUSNavigationCalibrationUpdateDepthDelegate.h"

#include <QFileDialog>
#include <QFile>
#include <QTextStream>

QmitkUSCombinedModalityEditWidget::QmitkUSCombinedModalityEditWidget(QWidget *parent) :
  QWidget(parent),
  m_CalibrationsDataModel(new QmitkUSNavigationCalibrationsDataModel(this)),
  m_CalibrationUpdateDepthDelegate(new QmitkUSNavigationCalibrationUpdateDepthDelegate(this)),
  ui(new Ui::QmitkUSCombinedModalityEditWidget)
{
  ui->setupUi(this);
  ui->calibrationsTable->setModel(m_CalibrationsDataModel);
  ui->calibrationsTable->setItemDelegateForColumn(2, new QmitkUSNavigationCalibrationRemoveDelegate(this));
  ui->calibrationsTable->setItemDelegateForColumn(0, m_CalibrationUpdateDepthDelegate);
}

QmitkUSCombinedModalityEditWidget::~QmitkUSCombinedModalityEditWidget()
{
  delete ui;
}

void QmitkUSCombinedModalityEditWidget::SetCombinedModality(mitk::AbstractUltrasoundTrackerDevice::Pointer combinedModality)
{
  m_CalibrationsDataModel->SetCombinedModality(combinedModality);

  m_CombinedModality = combinedModality;
  if ( combinedModality.IsNull() ) { return; }

  m_LastCalibrations = m_CombinedModality->SerializeCalibration();

  ui->vendorLineEdit->setText(QString::fromStdString(combinedModality->GetUltrasoundDevice()->GetManufacturer()));
  ui->nameLineEdit->setText(QString::fromStdString(combinedModality->GetUltrasoundDevice()->GetName()));

  ui->ultrasoundDeviceLabel->setText(QString::fromStdString(combinedModality->GetUltrasoundDevice()->GetManufacturer() + " " +
    combinedModality->GetUltrasoundDevice()->GetName()));
  ui->trackingDeviceLabel->setText(QString::fromStdString(combinedModality->GetNavigationDataSource()->GetName()));

  mitk::USDevice::Pointer usDevice = combinedModality->GetUltrasoundDevice();
  if ( usDevice.IsNull() ) { return; }

  mitk::USControlInterfaceBMode::Pointer controlInterfaceBMode = usDevice->GetControlInterfaceBMode();
  if ( controlInterfaceBMode.IsNull() ) { return; }
  m_CalibrationUpdateDepthDelegate->SetControlInterfaceBMode(controlInterfaceBMode);
}

void QmitkUSCombinedModalityEditWidget::OnSaveButtonClicked()
{
  if ( m_CombinedModality.IsNotNull() )
  {
    m_CombinedModality->GetUltrasoundDevice()->SetManufacturer(ui->vendorLineEdit->text().toStdString());
    m_CombinedModality->GetUltrasoundDevice()->SetName(ui->nameLineEdit->text().toStdString());

    m_LastCalibrations = m_CombinedModality->SerializeCalibration();
  }

  emit SignalSaved();
}

void QmitkUSCombinedModalityEditWidget::OnCancelButtonClicked()
{
  if ( m_CombinedModality.IsNotNull() && ! m_LastCalibrations.empty() )
  {
    // restore previous calibrations if cancel button was clicked
    m_CombinedModality->DeserializeCalibration(m_LastCalibrations);
  }

  emit SignalAborted();
}

void QmitkUSCombinedModalityEditWidget::OnCalibrationsSaveButtonClicked()
{
  if ( m_CombinedModality.IsNull() )
  {
    MITK_WARN << "Cannot save calibrations as no combined modality is available.";
    return;
  }

  QString filename = QFileDialog::getSaveFileName( QApplication::activeWindow(),
      "Save Calibrations", "", "Calibration files *.cal" );

  QFile file(filename);
  if ( ! file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate) )
  {
    MITK_WARN << "Cannot open file '" << filename.toStdString() << "' for writing.";
    return;
  }

  std::string calibrationSerialization = m_CombinedModality->SerializeCalibration();

  QTextStream outStream(&file);
  outStream << QString::fromStdString(calibrationSerialization);
}

void QmitkUSCombinedModalityEditWidget::OnCalibrationsLoadButtonClicked()
{
  if ( m_CombinedModality.IsNull() )
  {
    MITK_WARN << "Cannot load calibrations as no combined modality is available.";
    return;
  }

  QString filename = QFileDialog::getOpenFileName( QApplication::activeWindow(),
    "Load Calibration", "", "Calibration files *.cal" );

  if ( filename.isNull() ) { return; }

  QFile file(filename);
  if ( ! file.open(QIODevice::ReadOnly | QIODevice::Text) )
  {
    MITK_WARN << "Cannot open file '" << filename.toStdString() << "' for reading.";
    return;
  }

  QTextStream inStream(&file);
  std::string calibration = inStream.readAll().toStdString();
  if ( calibration.empty() )
  {
    MITK_WARN << "Failed to load file. Unsupported format?";
    return;
  }

  try
  {
    m_CombinedModality->DeserializeCalibration(calibration, false);
  }
  catch ( const mitk::Exception& /*exception*/ )
  {
    MITK_WARN << "Failed to deserialize calibration. Unsuppoerted format?";
    return;
  }

  // make sure that the table model is up to date
  m_CalibrationsDataModel->SetCombinedModality(m_CombinedModality);
}
