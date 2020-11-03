/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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

#include <chrono>
#include <numeric>
#include <QtConcurrentRun>

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
  m_Controls.tableError->hide();
  connect((QObject*)(m_Controls.QComboBoxAlgorithm), SIGNAL(currentIndexChanged(int)), this, SLOT(EnableGUIWeight()));
  connect((QObject*)(m_Controls.checkBoxsO2), SIGNAL(clicked()), this, SLOT(EnableGUISO2()));
  connect((QObject*)(m_Controls.checkBoxError), SIGNAL(clicked()), this, SLOT(EnableGUIError()));
  this->connect(this, SIGNAL(finishSignal()), this, SLOT(storeOutputs()));
  this->connect(this, SIGNAL(crashSignal()), this, SLOT(crashInfo()));
  this->connect(this, SIGNAL(enableSignal()), this, SLOT(EnableGUIControl()));
}

void SpectralUnmixing::EnableGUIControl()
{
  SwitchGUIControl(false);
}

void SpectralUnmixing::SwitchGUIControl(bool change)
{
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
  m_Controls.checkBoxError->setEnabled(change);
}

void SpectralUnmixing::EnableGUIWeight()
{
  auto qs = m_Controls.QComboBoxAlgorithm->currentText();
  std::string Algorithm = qs.toUtf8().constData();
  if (Algorithm == "weighted")
    m_Controls.tableWeight->show();
  else
    m_Controls.tableWeight->hide();
}

void SpectralUnmixing::EnableGUISO2()
{
  if (m_Controls.checkBoxsO2->isChecked())
    m_Controls.tableSO2->show();
  else
    m_Controls.tableSO2->hide();
}

void SpectralUnmixing::EnableGUIError()
{
  if (m_Controls.checkBoxError->isChecked())
    m_Controls.tableError->show();
  else
    m_Controls.tableError->hide();
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
    MITK_WARN << "Unfortunaly algorithm is likley to fail!";
    spectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
    dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(spectralUnmixingFilter.GetPointer())
      ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::LDLT);
  }

  else if (algorithm == "llt")
  {
    MITK_WARN << "Unfortunaly algorithm is likley to fail!";
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
        MITK_INFO(PluginVerbose) << "Weight: " << Weight;
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(spectralUnmixingFilter.GetPointer())
          ->AddWeight(Weight);
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
    if (m_Controls.tableSO2->item(0, i))
    {
      QString Text = m_Controls.tableSO2->item(0, i)->text();
      int value = Text.toInt();
      MITK_INFO(PluginVerbose) << "SO2 setting value: " << value;
      m_sO2->AddSO2Settings(value);
    }
    else
      m_sO2->AddSO2Settings(0);
  }
}

void SpectralUnmixing::SetRelativeErrorSettings(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter)
{
  for (unsigned int i = 0; i < 2; ++i)
  {
    if (m_Controls.tableError->item(0, i))
    {
      QString Text = m_Controls.tableError->item(0, i)->text();
      int value = Text.toInt();
      MITK_INFO(PluginVerbose) << "Relative error setting value: " << value;
      m_SpectralUnmixingFilter->AddRelativeErrorSettings(value);
    }
    else
      m_SpectralUnmixingFilter->AddRelativeErrorSettings(0);
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

void SpectralUnmixing::Settings(mitk::Image::Pointer image)
{
  boolVec = { m_Controls.checkBoxOx->isChecked(),  m_Controls.checkBoxDeOx->isChecked(),
    m_Controls.checkBoxMelanin->isChecked(), m_Controls.checkBoxAdd->isChecked() };
  outputNameVec = { "HbO2",  "Hb", "Melanin", "Static Endmember" };
  sO2Bool = (m_Controls.checkBoxsO2->isChecked());

  //Read GUI information(algorithm)
  auto qs = m_Controls.QComboBoxAlgorithm->currentText();
  Algorithm = qs.toUtf8().constData();

  m_SpectralUnmixingFilter = GetFilterInstance(Algorithm);
  SetVerboseMode(m_SpectralUnmixingFilter, PluginVerbose);
  m_SpectralUnmixingFilter->RelativeError(m_Controls.checkBoxError->isChecked());
  m_SpectralUnmixingFilter->SetInput(image);
  SetWavlength(m_SpectralUnmixingFilter);
  SetChromophore(m_SpectralUnmixingFilter, boolVec, outputNameVec);

  boolVec.push_back(m_Controls.checkBoxError->isChecked());
  outputNameVec.push_back("Relative Error");
  if (m_Controls.checkBoxError->isChecked())
    SetRelativeErrorSettings(m_SpectralUnmixingFilter);

  m_SpectralUnmixingFilter->AddOutputs(std::accumulate(boolVec.begin(), boolVec.end(), 0));
  MITK_INFO(PluginVerbose) << "Number of indexed outputs: " << std::accumulate(boolVec.begin(), boolVec.end(), 0);
}

void SpectralUnmixing::storeOutputs()
{
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

  if (sO2Bool)
    CalculateSO2(m_SpectralUnmixingFilter, boolVec);

  mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
  MITK_INFO(PluginVerbose) << "Adding images to DataStorage...[DONE]";

  std::chrono::steady_clock::time_point _end(std::chrono::steady_clock::now());
  MITK_INFO(m_Controls.checkBoxChrono->isChecked()) << "Time for image Processing: "
    << std::chrono::duration_cast<std::chrono::duration<double>>(_end - _start).count();
  QApplication::setOverrideCursor(Qt::ArrowCursor);
  SwitchGUIControl(true);
}

void SpectralUnmixing::WorkingThreadUpdateFilter(mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter)
{
  //SwitchGUIControl(false);
  emit enableSignal();
  try
  {
    m_SpectralUnmixingFilter->Update();
    emit finishSignal();
  }
  catch (const mitk::Exception& e)
  {
    QApplication::setOverrideCursor(Qt::ArrowCursor);
    SwitchGUIControl(true);
    errorMessage = e.GetDescription();
    emit crashSignal();
  }
}

void SpectralUnmixing::crashInfo()
{
  const char *error = errorMessage.c_str();
  QMessageBox::information(nullptr, "Template", error);
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
    image = dynamic_cast<mitk::Image *>(data);
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

      _start = std::chrono::steady_clock::now();
      PluginVerbose = m_Controls.checkBoxVerbose->isChecked();
      MITK_INFO(PluginVerbose) << message.str();

      try
      {
        Settings(image);
        MITK_INFO(PluginVerbose) << "Updating Filter...";
        QApplication::setOverrideCursor(Qt::WaitCursor);
        QtConcurrent::run(this, &SpectralUnmixing::WorkingThreadUpdateFilter, m_SpectralUnmixingFilter);
      }
      catch (const mitk::Exception& e)
      {
        QApplication::setOverrideCursor(Qt::ArrowCursor);
        QMessageBox::information(nullptr, "Template", e.GetDescription());
      }
    }
  }
}
