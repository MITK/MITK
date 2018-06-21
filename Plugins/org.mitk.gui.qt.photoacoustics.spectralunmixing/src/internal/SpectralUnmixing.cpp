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
#include "SpectralUnmixing.h"

// Qt
#include <QMessageBox>

// mitk image
#include <mitkImage.h>

// Include to perform Spectral Unmixing
#include "mitkPASpectralUnmixingFilterBase.h"
#include "mitkPALinearSpectralUnmixingFilter.h"
#include "mitkPASpectralUnmixingSO2.h"
#include "mitkPASpectralUnmixingFilterVigra.h"
#include "mitkPASpectralUnmixingFilterLagrange.h"
#include "mitkPASpectralUnmixingFilterSimplex.h"

#include <numeric>
#include <thread>

const std::string SpectralUnmixing::VIEW_ID = "org.mitk.views.spectralunmixing";

void SpectralUnmixing::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void SpectralUnmixing::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(m_Controls.buttonPerformImageProcessing, &QPushButton::clicked, this, &SpectralUnmixing::DoImageProcessing);
  m_Controls.tableWeight->hide();
  m_Controls.tableSO2->hide();
  connect((QObject*)(m_Controls.QComboBoxAlgorithm), SIGNAL(currentIndexChanged(int)), this, SLOT(ChangeGUIWeight()));
  connect((QObject*)(m_Controls.checkBoxsO2), SIGNAL(clicked()), this, SLOT(ChangeGUISO2()));
}

void SpectralUnmixing::SwitchGUIControls(bool change)
{
  MITK_INFO(PluginVerbose) << "ENABLE GUI " << change;
  m_Controls.inputtable->setEnabled(change);
  m_Controls.checkBoxOx->setEnabled(change);
  m_Controls.checkBoxDeOx->setEnabled(change);
  m_Controls.checkBoxMelanin->setEnabled(change);
  m_Controls.checkBoxAdd->setEnabled(change);
  m_Controls.QComboBoxAlgorithm->setEnabled(change);
  m_Controls.tableWeight->setEnabled(change);
  m_Controls.checkBoxsO2->setEnabled(change);
  m_Controls.tableSO2->setEnabled(change);
  m_Controls.checkBoxVerbose->setEnabled(change);
  m_Controls.checkBoxChrono->setEnabled(change);
  m_Controls.buttonPerformImageProcessing->setEnabled(change);
}


void SpectralUnmixing::ChangeGUIWeight()
{
  auto qs = m_Controls.QComboBoxAlgorithm->currentText();
  std::string Algorithm = qs.toUtf8().constData();
  if (Algorithm == "weighted")
    m_Controls.tableWeight->show();
  else
    m_Controls.tableWeight->hide();
}

void SpectralUnmixing::ChangeGUISO2()
{
  if (m_Controls.checkBoxsO2->isChecked())
    m_Controls.tableSO2->show();
  else
    m_Controls.tableSO2->hide();
}

void SpectralUnmixing::SetVerboseMode(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, bool PluginVerbose)
{
  m_SpectralUnmixingFilter->Verbose(PluginVerbose);
}

void SpectralUnmixing::SetWavlength(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter)
{
  int col = 0;
  int Wavelength = 1;
  while (m_Controls.inputtable->item(0, col) && Wavelength > 0)
  {
    QString Text = m_Controls.inputtable->item(0, col)->text();
    Wavelength = Text.toInt();
    if (Wavelength > 0)
    {
      m_SpectralUnmixingFilter->AddWavelength(Wavelength);
      MITK_INFO(PluginVerbose) << "Wavelength: " << Wavelength << "nm \n";
    }
    ++col;
  }
}

void SpectralUnmixing::SetChromophore(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec,
  std::vector<std::string> chromophoreNameVec)
{
  unsigned int numberofChromophores = 0;

  std::vector<mitk::pa::PropertyCalculator::ChromophoreType> m_ChromoType = { mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED,
    mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED, mitk::pa::PropertyCalculator::ChromophoreType::MELANIN,
    mitk::pa::PropertyCalculator::ChromophoreType::ONEENDMEMBER};

  for (unsigned int chromo = 0; chromo < m_ChromoType.size(); ++chromo)
  {
    if (boolVec[chromo] == true)
    {
      MITK_INFO(PluginVerbose) << "Chromophore: " << chromophoreNameVec[chromo];
      m_SpectralUnmixingFilter->AddChromophore(m_ChromoType[chromo]);
      numberofChromophores += 1;
    }
  }

  if (numberofChromophores == 0)
    mitkThrow() << "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!";
}

void SpectralUnmixing::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // iterate all selected objects, adjust warning visibility
  foreach (mitk::DataNode::Pointer node, nodes)
  {
    if (node.IsNotNull() && dynamic_cast<mitk::Image *>(node->GetData()))
    {
      m_Controls.labelWarning->setVisible(false);
      m_Controls.buttonPerformImageProcessing->setEnabled(true);
      return;
    }
  }

  m_Controls.labelWarning->setVisible(true);
  m_Controls.buttonPerformImageProcessing->setEnabled(false);
}

mitk::pa::SpectralUnmixingFilterBase::Pointer SpectralUnmixing::GetFilterInstance(std::string algorithm)
{
  mitk::pa::SpectralUnmixingFilterBase::Pointer spectralUnmixingFilter;

  if (algorithm == "householderQr")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::HOUSEHOLDERQR);
  }

  else if (algorithm == "ldlt")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LDLT);
  }

  else if (algorithm == "llt")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LLT);
  }

  else if (algorithm == "colPivHouseholderQr")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::COLPIVHOUSEHOLDERQR);
  }

  else if (algorithm == "jacobiSvd")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::JACOBISVD);
  }

  else if (algorithm == "fullPivLu")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVLU);
  }

  else if (algorithm == "fullPivHouseholderQr")
  {
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::FULLPIVHOUSEHOLDERQR);
  }

  else if (algorithm == "NNLARS")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS);
  }

  else if (algorithm == "NNGoldfarb")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::GOLDFARB);
  }

  else if (algorithm == "weighted")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED);
    //Tranfer GUI information(Weights) to filter
    unsigned int colunm = 0;
    int Weight = 1;
    while (m_Controls.tableWeight->item(0, colunm) && Weight > 0)
    {
      QString Text = m_Controls.tableWeight->item(0, colunm)->text();
      Weight = Text.toInt();
      if (Weight > 0)
      {
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
          ->AddWeight(Weight);
        MITK_INFO(PluginVerbose) << "Weight: " << Weight;
      }
      ++colunm;
    }
  }

  else if (algorithm == "LS")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
    dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LS);
  }

  else if (algorithm == "SimplexMax")
  {
    spectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterSimplex::New();
  }

  else
    mitkThrow() << "404 ALGORITHM NOT FOUND!";

    return spectralUnmixingFilter;
}


void SpectralUnmixing::SetSO2Settings(mitk::pa::SpectralUnmixingSO2::Pointer m_sO2)
{
  for (unsigned int i = 0; i < 4; ++i)
  {
    if (m_Controls.inputtable->item(0, i))
    {
      QString Text = m_Controls.tableSO2->item(0, i)->text();
      float value = Text.toFloat();
      MITK_INFO(PluginVerbose) << "SO2 setting value: " << value;
      m_sO2->AddSO2Settings(value);
    }
    else
      m_sO2->AddSO2Settings(0);
  }
}

void SpectralUnmixing::CalculateSO2(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter, std::vector<bool> boolVec)
{
  MITK_INFO(PluginVerbose) << "CALCULATE OXYGEN SATURATION ...";

  if (!boolVec[0])
    mitkThrow() << "SELECT CHROMOPHORE DEOXYHEMOGLOBIN!";
  if (!boolVec[1])
    mitkThrow() << "SELECT CHROMOPHORE OXYHEMOGLOBIN!";
  auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
  m_sO2->Verbose(PluginVerbose);
  SetSO2Settings(m_sO2);

  // Initialize pipeline from SU filter class to SO2 class
  auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
  auto output2 = m_SpectralUnmixingFilter->GetOutput(1);
  m_sO2->SetInput(0, output1);
  m_sO2->SetInput(1, output2);

  m_sO2->Update();

  mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
  sO2->SetSpacing(output1->GetGeometry()->GetSpacing());
  WriteOutputToDataStorage(sO2, "sO2");
  MITK_INFO(PluginVerbose) << "[DONE]";
}

void SpectralUnmixing::WriteOutputToDataStorage(mitk::Image::Pointer m_Image, std::string name)
{
  mitk::DataNode::Pointer dataNodeOutput = mitk::DataNode::New();
  dataNodeOutput->SetData(m_Image);
  dataNodeOutput->SetName(name);
  this->GetDataStorage()->Add(dataNodeOutput);
}

void SpectralUnmixing::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();
  if (nodes.empty())
    return;

  mitk::DataNode *node = nodes.front();

  if (!node)
  {
    // Nothing selected. Inform the user and return
    QMessageBox::information(nullptr, "Template", "Please load and select an image before starting image processing.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData *data = node->GetData();
  if (data)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image *image = dynamic_cast<mitk::Image *>(data);
    if (image)
    {
      std::stringstream message;
      std::string name;
      message << "PERFORMING SPECTRAL UNMIXING ";
      if (node->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << ".";
      SwitchGUIControls(false);
      std::chrono::steady_clock::time_point _start(std::chrono::steady_clock::now());

      PluginVerbose = m_Controls.checkBoxVerbose->isChecked();
      MITK_INFO(PluginVerbose) << message.str();

      GenerateOutput(image);

      std::chrono::steady_clock::time_point _end(std::chrono::steady_clock::now());
      MITK_INFO(m_Controls.checkBoxChrono->isChecked()) << "Time for image Processing: "
        << std::chrono::duration_cast<std::chrono::duration<double>>(_end - _start).count();
      SwitchGUIControls(true);
    }
  }
}

void SpectralUnmixing::GenerateOutput(mitk::Image::Pointer image)
{
  std::vector<bool> boolVec = { m_Controls.checkBoxOx->isChecked(),  m_Controls.checkBoxDeOx->isChecked(),
    m_Controls.checkBoxMelanin->isChecked(), m_Controls.checkBoxAdd->isChecked() };
  std::vector<std::string> outputNameVec = { "HbO2",  "Hb", "Melanin", "Static Endmember" };

  //Read GUI information(algorithm)
  auto qs = m_Controls.QComboBoxAlgorithm->currentText();
  std::string Algorithm = qs.toUtf8().constData();

  mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter = GetFilterInstance(Algorithm);
  SetVerboseMode(m_SpectralUnmixingFilter, PluginVerbose);
  m_SpectralUnmixingFilter->RelativeError(m_Controls.checkBoxError->isChecked());
  m_SpectralUnmixingFilter->SetInput(image);
  SetWavlength(m_SpectralUnmixingFilter);
  SetChromophore(m_SpectralUnmixingFilter, boolVec, outputNameVec);

  boolVec.push_back(m_Controls.checkBoxError->isChecked());
  outputNameVec.push_back("Relative Error");

  m_SpectralUnmixingFilter->AddOutputs(std::accumulate(boolVec.begin(), boolVec.end(), 0));
  MITK_INFO(PluginVerbose) << "Number of indexed outputs: " << std::accumulate(boolVec.begin(), boolVec.end(), 0);

  MITK_INFO(PluginVerbose) << "Updating Filter...";
  m_SpectralUnmixingFilter->Update();

  int outputCounter = 0;
  mitk::Image::Pointer m_Output;
  for (unsigned int chromophore = 0; chromophore < outputNameVec.size(); ++chromophore)
  {
    if (boolVec[chromophore] != false)
    {
      m_Output = m_SpectralUnmixingFilter->GetOutput(outputCounter++);
      m_Output->SetSpacing(image->GetGeometry()->GetSpacing());
      WriteOutputToDataStorage(m_Output, outputNameVec[chromophore] + Algorithm);
    }
  }

  if (m_Controls.checkBoxsO2->isChecked())
    CalculateSO2(m_SpectralUnmixingFilter, boolVec);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
  MITK_INFO(PluginVerbose) << "Adding images to DataStorage...[DONE]";
}
