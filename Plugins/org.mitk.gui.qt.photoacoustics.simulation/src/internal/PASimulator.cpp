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

// Qmitk
#include "PASimulator.h"

// Qt
#include <QMessageBox>
#include <QCheckBox>
#include <QFileDialog>

// mitk
#include <mitkImage.h>
#include <mitkDataNode.h>
#include <mitkIOUtil.h>

#include <iostream>
#include <fstream>

const std::string PASimulator::VIEW_ID = "org.mitk.views.pasimulator";

void PASimulator::SetFocus()
{
  m_Controls.pushButtonShowRandomTissue->setFocus();
}

void PASimulator::CreateQtPartControl(QWidget *parent)
{
  m_Controls.setupUi(parent);
  connect(m_Controls.pushButtonShowRandomTissue, SIGNAL(clicked()), this, SLOT(DoImageProcessing()));
  connect(m_Controls.checkBoxGauss, SIGNAL(stateChanged(int)), this, SLOT(ClickedGaussBox()));
  connect(m_Controls.pushButtonOpenPath, SIGNAL(clicked()), this, SLOT(OpenFolder()));
  connect(m_Controls.pushButtonOpenBinary, SIGNAL(clicked()), this, SLOT(OpenBinary()));
  connect(m_Controls.checkBoxGenerateBatch, SIGNAL(clicked()), this, SLOT(UpdateVisibilityOfBatchCreation()));
  connect(m_Controls.pushButtonAjustWavelength, SIGNAL(clicked()), this, SLOT(UpdateParametersAccordingToWavelength()));
  connect(m_Controls.checkBoxRngSeed, SIGNAL(clicked()), this, SLOT(ClickedCheckboxFixedSeed()));
  connect(m_Controls.checkBoxRandomizeParameters, SIGNAL(clicked()), this, SLOT(ClickedRandomizePhysicalParameters()));

  m_Controls.spinboxSigma->setEnabled(false);
  m_Controls.labelSigma->setEnabled(false);

  std::string home_env = std::string(std::getenv("HOME"));
  if (home_env.empty())
  {
    home_env = std::string(std::getenv("HOMEPATH"));
  }
  if (home_env.empty())
  {
    home_env = "";
  }
  m_Controls.label_NrrdFilePath->setText(home_env.c_str());

  m_PhotoacousticPropertyCalculator = mitk::pa::PropertyCalculator::New();

  UpdateVisibilityOfBatchCreation();
  ClickedRandomizePhysicalParameters();
  ClickedCheckboxFixedSeed();
  ClickedGaussBox();
}

void PASimulator::ClickedRandomizePhysicalParameters()
{
  m_Controls.spinboxRandomizeParameters->setEnabled(m_Controls.checkBoxRandomizeParameters->isChecked());
}

void PASimulator::ClickedCheckboxFixedSeed()
{
  m_Controls.spinBoxRngSeed->setEnabled(m_Controls.checkBoxRngSeed->isChecked());
}

void PASimulator::UpdateParametersAccordingToWavelength()
{
  int wavelength = m_Controls.spinboxWavelength->value();
  double bloodOxygenation = m_Controls.spinboxBloodOxygenSaturation->value() / 100;

  auto result = m_PhotoacousticPropertyCalculator->CalculatePropertyForSpecificWavelength(
    mitk::pa::PropertyCalculator::TissueType::BLOOD, wavelength, bloodOxygenation);
  m_Controls.spinboxMaxAbsorption->setValue(result.mua);
  m_Controls.spinboxMinAbsorption->setValue(result.mua);
  m_Controls.spinboxBloodVesselScatteringMinimum->setValue(result.mus);
  m_Controls.spinboxBloodVesselScatteringMaximum->setValue(result.mus);
  m_Controls.spinboxBloodVesselAnisotropyMinimum->setValue(result.g);
  m_Controls.spinboxBloodVesselAnisotropyMaximum->setValue(result.g);

  result = m_PhotoacousticPropertyCalculator->CalculatePropertyForSpecificWavelength(
    mitk::pa::PropertyCalculator::TissueType::EPIDERMIS, wavelength, bloodOxygenation);
  m_Controls.spinboxSkinAbsorption->setValue(result.mua);
  m_Controls.spinboxSkinScattering->setValue(result.mus);
  m_Controls.spinboxSkinAnisotropy->setValue(result.g);

  result = m_PhotoacousticPropertyCalculator->CalculatePropertyForSpecificWavelength(
    mitk::pa::PropertyCalculator::TissueType::STANDARD_TISSUE, wavelength, bloodOxygenation);
  m_Controls.spinboxBackgroundAbsorption->setValue(result.mua);
  m_Controls.spinboxBackgroundScattering->setValue(result.mus);
  m_Controls.spinboxBackgroundAnisotropy->setValue(result.g);
}

void PASimulator::UpdateVisibilityOfBatchCreation()
{
  m_Controls.widgetBatchFile->setVisible(m_Controls.checkBoxGenerateBatch->isChecked());
}

mitk::pa::TissueGeneratorParameters::Pointer PASimulator::GetParametersFromUIInput()
{
  auto parameters = mitk::pa::TissueGeneratorParameters::New();

  // Getting settings from UI
  // General settings
  parameters->SetXDim(m_Controls.spinboxXDim->value());
  parameters->SetYDim(m_Controls.spinboxYDim->value());
  parameters->SetZDim(m_Controls.spinboxZDim->value());
  parameters->SetDoVolumeSmoothing(m_Controls.checkBoxGauss->isChecked());
  if (parameters->GetDoVolumeSmoothing())
    parameters->SetVolumeSmoothingSigma(m_Controls.spinboxSigma->value());
  parameters->SetRandomizePhysicalProperties(m_Controls.checkBoxRandomizeParameters->isChecked());
  parameters->SetRandomizePhysicalPropertiesPercentage(m_Controls.spinboxRandomizeParameters->value());
  parameters->SetVoxelSpacingInCentimeters(m_Controls.spinboxSpacing->value());
  parameters->SetUseRngSeed(m_Controls.checkBoxRngSeed->isChecked());
  parameters->SetRngSeed(m_Controls.spinBoxRngSeed->value());

  // Monte Carlo simulation parameters
  parameters->SetMCflag(m_Controls.spinboxMcFlag->value());
  parameters->SetMCLaunchflag(m_Controls.spinboxLaunchFlag->value());
  parameters->SetMCBoundaryflag(m_Controls.spinboxboundaryFlag->value());
  parameters->SetMCLaunchPointX(m_Controls.spinboxLaunchpointX->value());
  parameters->SetMCLaunchPointY(m_Controls.spinboxLaunchpointY->value());
  parameters->SetMCLaunchPointZ(m_Controls.spinboxLaunchpointZ->value());
  parameters->SetMCFocusPointX(m_Controls.spinboxFocuspointX->value());
  parameters->SetMCFocusPointY(m_Controls.spinboxFocuspointY->value());
  parameters->SetMCFocusPointZ(m_Controls.spinboxFocuspointZ->value());
  parameters->SetMCTrajectoryVectorX(m_Controls.spinboxTrajectoryVectorX->value());
  parameters->SetMCTrajectoryVectorY(m_Controls.spinboxTrajectoryVectorY->value());
  parameters->SetMCTrajectoryVectorZ(m_Controls.spinboxTrajectoryVectorZ->value());
  parameters->SetMCRadius(m_Controls.spinboxRadius->value());
  parameters->SetMCWaist(m_Controls.spinboxWaist->value());

  // Vessel settings
  parameters->SetMaxVesselAbsorption(m_Controls.spinboxMaxAbsorption->value());
  parameters->SetMinVesselAbsorption(m_Controls.spinboxMinAbsorption->value());
  parameters->SetMaxVesselBending(m_Controls.spinboxMaxBending->value());
  parameters->SetMinVesselBending(m_Controls.spinboxMinBending->value());
  parameters->SetMaxVesselRadiusInMillimeters(m_Controls.spinboxMaxDiameter->value());
  parameters->SetMinVesselRadiusInMillimeters(m_Controls.spinboxMinDiameter->value());
  parameters->SetMaxNumberOfVessels(m_Controls.spinboxMaxVessels->value());
  parameters->SetMinNumberOfVessels(m_Controls.spinboxMinVessels->value());
  parameters->SetMinVesselScattering(m_Controls.spinboxBloodVesselScatteringMinimum->value());
  parameters->SetMaxVesselScattering(m_Controls.spinboxBloodVesselScatteringMaximum->value());
  parameters->SetMinVesselAnisotropy(m_Controls.spinboxBloodVesselAnisotropyMinimum->value());
  parameters->SetMaxVesselAnisotropy(m_Controls.spinboxBloodVesselAnisotropyMaximum->value());
  parameters->SetVesselBifurcationFrequency(m_Controls.spinboxBifurcationFrequency->value());
  parameters->SetMinVesselZOrigin(m_Controls.spinboxMinSpawnDepth->value());
  parameters->SetMaxVesselZOrigin(m_Controls.spinboxMaxSpawnDepth->value());

  // Background tissue settings
  parameters->SetBackgroundAbsorption(m_Controls.spinboxBackgroundAbsorption->value());
  parameters->SetBackgroundScattering(m_Controls.spinboxBackgroundScattering->value());
  parameters->SetBackgroundAnisotropy(m_Controls.spinboxBackgroundAnisotropy->value());

  // Air settings
  parameters->SetAirThicknessInMillimeters(m_Controls.spinboxAirThickness->value());

  //Skin tissue settings
  parameters->SetSkinThicknessInMillimeters(m_Controls.spinboxSkinThickness->value());
  parameters->SetSkinAbsorption(m_Controls.spinboxSkinAbsorption->value());
  parameters->SetSkinScattering(m_Controls.spinboxSkinScattering->value());
  parameters->SetSkinAnisotropy(m_Controls.spinboxSkinAnisotropy->value());

  parameters->SetCalculateNewVesselPositionCallback(&mitk::pa::VesselMeanderStrategy::CalculateRandomlyDivergingPosition);

  return parameters;
}

void PASimulator::DoImageProcessing()
{
  int numberOfVolumes = 1;

  if (m_Controls.checkBoxGenerateBatch->isChecked())
  {
    if (m_Controls.labelBinarypath->text().isNull() || m_Controls.labelBinarypath->text().isEmpty())
    {
      QMessageBox::warning(nullptr, QString("Warning"), QString("You need to specify the binary first!"));
      return;
    }

    numberOfVolumes = m_Controls.spinboxNumberVolumes->value();

    if (numberOfVolumes < 1)
    {
      QMessageBox::warning(nullptr, QString("Warning"), QString("You need to create at least one volume!"));
      return;
    }
  }

  auto tissueParameters = GetParametersFromUIInput();

  for (int volumeIndex = 0; volumeIndex < numberOfVolumes; volumeIndex++)
  {
    mitk::pa::InSilicoTissueVolume::Pointer volume =
      mitk::pa::InSilicoTissueGenerator::GenerateInSilicoData(tissueParameters);

    mitk::Image::Pointer tissueVolume = volume->ConvertToMitkImage();

    if (m_Controls.checkBoxGenerateBatch->isChecked())
    {
      std::string nrrdFilePath = m_Controls.label_NrrdFilePath->text().toStdString();
      std::string tissueName = m_Controls.lineEditTissueName->text().toStdString();
      std::string binaryPath = m_Controls.labelBinarypath->text().toStdString();
      long numberOfPhotons = m_Controls.spinboxNumberPhotons->value() * 1000L;

      auto batchParameters = mitk::pa::SimulationBatchGeneratorParameters::New();
      batchParameters->SetBinaryPath(binaryPath);
      batchParameters->SetNrrdFilePath(nrrdFilePath);
      batchParameters->SetNumberOfPhotons(numberOfPhotons);
      batchParameters->SetTissueName(tissueName);
      batchParameters->SetVolumeIndex(volumeIndex);
      batchParameters->SetYOffsetLowerThresholdInCentimeters(m_Controls.spinboxFromValue->value());
      batchParameters->SetYOffsetUpperThresholdInCentimeters(m_Controls.spinboxToValue->value());
      batchParameters->SetYOffsetStepInCentimeters(m_Controls.spinboxStepValue->value());
      mitk::pa::SimulationBatchGenerator::WriteBatchFileAndSaveTissueVolume(batchParameters, tissueVolume);
    }
    else
    {
      mitk::DataNode::Pointer dataNode = mitk::DataNode::New();
      dataNode->SetData(tissueVolume);
      dataNode->SetName(m_Controls.lineEditTissueName->text().toStdString());
      this->GetDataStorage()->Add(dataNode);
      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
    }
  }
}

void PASimulator::ClickedGaussBox()
{
  if (m_Controls.checkBoxGauss->isChecked())
  {
    m_Controls.spinboxSigma->setEnabled(true);
    m_Controls.labelSigma->setEnabled(true);
  }
  else
  {
    m_Controls.spinboxSigma->setEnabled(false);
    m_Controls.labelSigma->setEnabled(false);
  }
}

void PASimulator::OpenFolder()
{
  m_Controls.label_NrrdFilePath->setText(QFileDialog::getExistingDirectory().append("/"));
}

void PASimulator::OpenBinary()
{
  m_Controls.labelBinarypath->setText(QFileDialog::getOpenFileName());
}
