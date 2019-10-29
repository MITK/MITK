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

#include "QmitkPatientInfoWidget.h"

// semantic relations module
#include <mitkDICOMHelper.h>

// mitk core
#include <mitkPropertyNameHelper.h>

QmitkPatientInfoWidget::QmitkPatientInfoWidget(QWidget* parent)
  : QWidget(parent)
{
  Init();
}

QmitkPatientInfoWidget::~QmitkPatientInfoWidget()
{
  // nothing here
}

void QmitkPatientInfoWidget::Init()
{
  // Create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(this);
}

void QmitkPatientInfoWidget::SetPatientInfo(const mitk::DataNode* dataNode)
{
  if (nullptr == dataNode)
  {
    MITK_INFO << "Not a valid data node.";
    return;
  }

  mitk::BaseData* baseData = dataNode->GetData();
  if (nullptr == baseData)
  {
    MITK_INFO << "No valid base data.";
    return;
  }

  mitk::BaseProperty* acquisitionDate = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0022).c_str());
  std::string acquisitionDateAsString = "";
  if (nullptr != acquisitionDate)
  {
    acquisitionDateAsString = acquisitionDate->GetValueAsString();
    mitk::DICOMHelper::ReformatDICOMTag(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0022).c_str(), acquisitionDateAsString);
  }

  mitk::BaseProperty* acquisitionTime = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0032).c_str());
  std::string acquisitionTimeAsString = "";
  if (nullptr != acquisitionTime)
  {
    acquisitionTimeAsString = acquisitionTime->GetValueAsString();
    mitk::DICOMHelper::ReformatDICOMTag(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0032).c_str(), acquisitionTimeAsString);
  }

  std::string patiensBirthDateAsString = "";
  mitk::BaseProperty* patientBirthDate = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0030).c_str());
  if (nullptr != patientBirthDate)
  {
    patiensBirthDateAsString = patientBirthDate->GetValueAsString();
    mitk::DICOMHelper::ReformatDICOMTag(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0030).c_str(), patiensBirthDateAsString);
  }

  mitk::BaseProperty* modality = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str());
  std::string modalityAsString = "";
  if (nullptr != modality)
  {
    modalityAsString = modality->GetValueAsString();
    mitk::DICOMHelper::ReformatDICOMTag(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(), modalityAsString);
  }

  mitk::BaseProperty* gender = baseData->GetProperty(mitk::GeneratePropertyNameForDICOMTag(0x0010, 0x0040).c_str());
  std::string genderAsString = "";
  if (nullptr != gender)
  {
    genderAsString = gender->GetValueAsString();
    mitk::DICOMHelper::ReformatDICOMTag(mitk::GeneratePropertyNameForDICOMTag(0x0008, 0x0060).c_str(), genderAsString);
  }

  m_Controls.dataIDLineEdit->setText(QString::fromStdString(mitk::DICOMHelper::GetIDFromDataNode(dataNode)));
  m_Controls.nameLineEdit->setText(QString::fromStdString(mitk::DICOMHelper::GetCaseIDFromDataNode(dataNode)));
  m_Controls.dateLineEdit->setText(QString::fromStdString(acquisitionDateAsString));
  m_Controls.timeLineEdit->setText(QString::fromStdString(acquisitionTimeAsString));
  m_Controls.scannerLineEdit->setText("");
  m_Controls.modalityLineEdit->setText(QString::fromStdString(modalityAsString));
  m_Controls.birthdateLineEdit->setText(QString::fromStdString(patiensBirthDateAsString));
  m_Controls.genderLineEdit->setText(QString::fromStdString(genderAsString));
}
