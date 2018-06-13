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
}

void SpectralUnmixing::ClearWavelength()
{
  m_Wavelengths.clear();
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
      MITK_INFO << message.str();

      //Tranfer GUI information(Algortihm) to filter
      auto qs = m_Controls.QComboBoxAlgorithm->currentText();
      std::string Algorithm = qs.toUtf8().constData();

      mitk::pa::SpectralUnmixingFilterBase::Pointer m_SpectralUnmixingFilter;

      if (Algorithm == "householderQr")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr);
      }

      else if (Algorithm == "ldlt")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::ldlt);
      }

      else if (Algorithm == "llt")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::llt);
      }

      else if (Algorithm == "colPivHouseholderQr")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::colPivHouseholderQr);
      }

      else if (Algorithm == "jacobiSvd")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::jacobiSvd);
      }

      else if (Algorithm == "fullPivLu")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivLu);
      }

      else if (Algorithm == "fullPivHouseholderQr")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivHouseholderQr);
      }

      else if (Algorithm == "test")
      {
        m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();
        dynamic_cast<mitk::pa::LinearSpectralUnmixingFilter*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::test);
      }
      
      else if (Algorithm == "NNLARS")
      {
        m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LARS);
      }

      else if (Algorithm == "NNGoldfarb")
      {
        m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::GOLDFARB);
      }

      else if (Algorithm == "weighted")
      {
        m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::WEIGHTED);
      }

      else if (Algorithm == "LS")
      {
        m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::LS);
      }

      else if (Algorithm == "vigratest")
      {
        m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterVigra::New();
        dynamic_cast<mitk::pa::SpectralUnmixingFilterVigra*>(m_SpectralUnmixingFilter.GetPointer())
          ->SetAlgorithm(mitk::pa::SpectralUnmixingFilterVigra::VigraAlgortihmType::vigratest);
      }

      else if (Algorithm == "SimplexMax")
      {
        m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilterSimplex::New();
        dynamic_cast<mitk::pa::SpectralUnmixingFilterSimplex*>(m_SpectralUnmixingFilter.GetPointer());
      }

      else
        mitkThrow() << "404 ALGORITHM NOT FOUND!";

      m_SpectralUnmixingFilter->SetInput(image);

      //Tranfer GUI information(Wavelength) to filter
      ClearWavelength();
      int col = 0;
      int Wavelength = 1;
      while (m_Controls.inputtable->item(0, col) && Wavelength > 0)
      {
        QString Text = m_Controls.inputtable->item(0, col)->text();
        Wavelength = Text.toInt();
        if (Wavelength > 0)
          m_SpectralUnmixingFilter->AddWavelength(Wavelength);
        ++col;
      }

      //Tranfer GUI information(Chromophores) to filter
      unsigned int numberofChromophores = 0;
      DeOxbool = m_Controls.checkBoxDeOx->isChecked();
      Oxbool = m_Controls.checkBoxOx->isChecked();
      bool Melaninbool = m_Controls.checkBoxMelanin->isChecked();
      bool Onebool = m_Controls.checkBoxAdd->isChecked();
      if (DeOxbool || Oxbool)
      {
        MITK_INFO << "CHOSEN CHROMOPHORES:";
      }
      if (Oxbool)
      {
        numberofChromophores += 1;
        MITK_INFO << "- Oxyhemoglobin";
        m_SpectralUnmixingFilter->AddChromophore(
        mitk::pa::PropertyCalculator::ChromophoreType::OXYGENATED);
      }
      if (DeOxbool)
      {
        numberofChromophores += 1;
         MITK_INFO << "- Deoxygenated hemoglobin";
         m_SpectralUnmixingFilter->AddChromophore(
        mitk::pa::PropertyCalculator::ChromophoreType::DEOXYGENATED);
      }
      if (Melaninbool)
      {
        numberofChromophores += 1;
        MITK_INFO << "- Melanin";
        m_SpectralUnmixingFilter->AddChromophore(
          mitk::pa::PropertyCalculator::ChromophoreType::MELANIN);
      }
      if (Onebool)
      {
        numberofChromophores += 1;
        MITK_INFO << "- Additional Chromophore";
        m_SpectralUnmixingFilter->AddChromophore(
          mitk::pa::PropertyCalculator::ChromophoreType::ONEENDMEMBER);
      }
      if (numberofChromophores == 0)
      {
        mitkThrow() << "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!";
      }

      MITK_INFO << "Updating Filter...";
      m_SpectralUnmixingFilter->Update();
      int outputCounter = 0;
      // Write Output images to Data Storage
      if (Oxbool)
      {
        mitk::Image::Pointer HbO2 = m_SpectralUnmixingFilter->GetOutput(outputCounter);
        outputCounter += 1;
        mitk::DataNode::Pointer dataNodeHbO2 = mitk::DataNode::New();
        dataNodeHbO2->SetData(HbO2);
        dataNodeHbO2->SetName("HbO2 " + Algorithm);
        this->GetDataStorage()->Add(dataNodeHbO2);
      }

      if (DeOxbool)
      {
        mitk::Image::Pointer Hb = m_SpectralUnmixingFilter->GetOutput(outputCounter);
        outputCounter += 1;
        mitk::DataNode::Pointer dataNodeHb = mitk::DataNode::New();
        dataNodeHb->SetData(Hb);
        dataNodeHb->SetName("Hb " + Algorithm);
        this->GetDataStorage()->Add(dataNodeHb);
      }

      if (Melaninbool)
      {
        mitk::Image::Pointer Melanin = m_SpectralUnmixingFilter->GetOutput(outputCounter);
        outputCounter += 1;
        mitk::DataNode::Pointer dataNodeMelanin = mitk::DataNode::New();
        dataNodeMelanin->SetData(Melanin);
        dataNodeMelanin->SetName("Melanin " + Algorithm);
        this->GetDataStorage()->Add(dataNodeMelanin);
      }

      if (Onebool)
      {
        mitk::Image::Pointer One = m_SpectralUnmixingFilter->GetOutput(outputCounter);
        mitk::DataNode::Pointer dataNodeOne = mitk::DataNode::New();
        dataNodeOne->SetData(One);
        dataNodeOne->SetName("One " + Algorithm);
        this->GetDataStorage()->Add(dataNodeOne);
      }

      //Calculate oxygen saturation
      bool sO2bool = m_Controls.checkBoxsO2->isChecked();

      if (sO2bool)
      {
        if (!DeOxbool)
          mitkThrow() << "SELECT CHROMOPHORE DEOXYHEMOGLOBIN!";
        if (!Oxbool)
          mitkThrow() << "SELECT CHROMOPHORE OXYHEMOGLOBIN!";

        MITK_INFO << "CALCULATE OXYGEN SATURATION ...";
        auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();

        // Oxygen Saturation Setting
        for (int i = 0; i < 4; ++i)
        {
          if (m_Controls.inputtable->item(0, i))
          {
            QString Text = m_Controls.tableSO2->item(0, i)->text();
            float value = Text.toFloat();
            MITK_INFO << "value: " << value;
            m_sO2->AddSO2Settings(value);
          }
          else
            m_sO2->AddSO2Settings(0);
        }

        // Initialize pipeline from SU filter class to SO2 class

        auto output1 = m_SpectralUnmixingFilter->GetOutput(0);
        auto output2 = m_SpectralUnmixingFilter->GetOutput(1);
        m_sO2->SetInput(0, output1);
        m_sO2->SetInput(1, output2);

        m_sO2->Update();

        // Write Output images to Data Storage
        mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
        mitk::DataNode::Pointer dataNodesO2 = mitk::DataNode::New();
        dataNodesO2->SetData(sO2);
        dataNodesO2->SetName("sO2");
        this->GetDataStorage()->Add(dataNodesO2);

        MITK_INFO << "[DONE]";
      }
      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());
      MITK_INFO << "Adding images to DataStorage...[DONE]";
    }
  }
}
