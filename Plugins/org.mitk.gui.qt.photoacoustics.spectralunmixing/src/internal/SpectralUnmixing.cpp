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
  connect(m_Controls.ButtonAddWavelength, &QPushButton::clicked, this, &SpectralUnmixing::Wavelength);
  connect(m_Controls.ButtonClearWavelength, &QPushButton::clicked, this, &SpectralUnmixing::ClearWavelength);
}

// Adds Wavelength @ Plugin with button
void SpectralUnmixing::Wavelength()
{
  if (m_Wavelengths.empty())
  {
      size = 0;
  }

  unsigned int wavelength = m_Controls.spinBoxAddWavelength->value();
  m_Wavelengths.push_back(wavelength);
  size += 1; // size implemented like this because '.size' is const
  MITK_INFO << "ALL WAVELENGTHS: ";

  for (int i = 0; i < size; ++i)
  {
    MITK_INFO << m_Wavelengths[i] << "nm";
  }
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

      auto m_SpectralUnmixingFilter = mitk::pa::LinearSpectralUnmixingFilter::New();

      m_SpectralUnmixingFilter->SetInput(image);

      // Set Algortihm to filter
      auto qs = m_Controls.QComboBoxAlgorithm->currentText();
      std::string Algorithm = qs.toUtf8().constData();

      if (Algorithm == "householderQr")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::householderQr);

      else if (Algorithm == "ldlt")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::ldlt);

      else if (Algorithm == "llt")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::llt);

      else if (Algorithm == "colPivHouseholderQr")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::colPivHouseholderQr);

      else if (Algorithm == "jacobiSvd")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::jacobiSvd);

      else if (Algorithm == "fullPivLu")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivLu);

      else if (Algorithm == "fullPivHouseholderQr")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::fullPivHouseholderQr);

      else if (Algorithm == "test")
        m_SpectralUnmixingFilter->SetAlgorithm(mitk::pa::LinearSpectralUnmixingFilter::AlgortihmType::test);
      else
        mitkThrow() << "ALGORITHM ERROR!";

      // Wavelength implementation into filter
      for (unsigned int imageIndex = 0; imageIndex < m_Wavelengths.size(); imageIndex++)
      {
        unsigned int wavelength = m_Wavelengths[imageIndex];
        m_SpectralUnmixingFilter->AddWavelength(wavelength);
      }

      // Checking which chromophores wanted for SU if none throw exeption!
      unsigned int numberofChromophores = 0;
      DeOxbool = m_Controls.checkBoxDeOx->isChecked();
      Oxbool = m_Controls.checkBoxOx->isChecked();
      if (DeOxbool || Oxbool)
      {
        MITK_INFO << "CHOSEN CHROMOPHORES:";
      }
      if (Oxbool)
      {
        numberofChromophores += 1;
        MITK_INFO << "- Oxyhemoglobin";
        // Set chromophore Oxyhemoglobon:
        m_SpectralUnmixingFilter->AddChromophore(
        mitk::pa::SpectralUnmixingFilterBase::ChromophoreType::OXYGENATED_HEMOGLOBIN);
      }
      if (DeOxbool)
      {
        numberofChromophores += 1;
         MITK_INFO << "- Deoxygenated hemoglobin";
        // Set chromophore Deoxygenated hemoglobin:
        m_SpectralUnmixingFilter->AddChromophore(
        mitk::pa::SpectralUnmixingFilterBase::ChromophoreType::DEOXYGENATED_HEMOGLOBIN);
      }
      if (numberofChromophores == 0)
      {
        mitkThrow() << "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!";
      }
              
      MITK_INFO << "Updating Filter...";
      
      m_SpectralUnmixingFilter->Update();

      if (Oxbool)
      {
        mitk::Image::Pointer HbO2 = m_SpectralUnmixingFilter->GetOutput(0);
        mitk::DataNode::Pointer dataNodeHbO2 = mitk::DataNode::New();
        dataNodeHbO2->SetData(HbO2);
        dataNodeHbO2->SetName("HbO2");
        this->GetDataStorage()->Add(dataNodeHbO2);
      }

      if (DeOxbool)
      {
        mitk::Image::Pointer Hb = m_SpectralUnmixingFilter->GetOutput(1);
        mitk::DataNode::Pointer dataNodeHb = mitk::DataNode::New();
        dataNodeHb->SetData(Hb);
        dataNodeHb->SetName("Hb");
        this->GetDataStorage()->Add(dataNodeHb);
      }

      //mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

      //*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      //FRAGE zu SetInput: reicht es dort einen image pointer anzugeben?
      //wenn ja zweite Variante wenn nein dann erste. funktioniern aber leider beide nicht:/
      bool sO2bool = m_Controls.checkBoxsO2->isChecked();

      //1:
      /*
      mitk::Image::Pointer HbO2 = m_SpectralUnmixingFilter->GetOutput(0);
      mitk::DataNode::Pointer dataNodeHbO2 = mitk::DataNode::New();
      dataNodeHbO2->SetData(HbO2);
      mitk::BaseData *dataHbO2 = dataNodeHbO2->GetData();
      mitk::Image *imageHbO2 = dynamic_cast<mitk::Image *>(dataHbO2);

      mitk::Image::Pointer Hb = m_SpectralUnmixingFilter->GetOutput(1);
      mitk::DataNode::Pointer dataNodeHb = mitk::DataNode::New();
      dataNodeHb->SetData(Hb);
      mitk::BaseData *dataHb = dataNodeHb->GetData();
      mitk::Image *imageHb = dynamic_cast<mitk::Image *>(dataHb);

      auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
      m_sO2->SetInput(0,imageHbO2);
      m_sO2->SetInput(1,imageHb);

      m_sO2->Update();

      mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
      mitk::DataNode::Pointer dataNodesO2 = mitk::DataNode::New();
      dataNodesO2->SetData(sO2);
      dataNodesO2->SetName("sO2");
      this->GetDataStorage()->Add(dataNodesO2);/**/

      //2:
      /*
      if (sO2bool)
      {
        if (DeOxbool)
        {
          if (Oxbool)
          {
            MITK_INFO << "CALCULATE OXYGEN SATURATION ...";
            auto m_sO2 = mitk::pa::SpectralUnmixingSO2::New();
            m_sO2->SetInput(0, m_SpectralUnmixingFilter->GetOutput(0));
            m_sO2->SetInput(1, m_SpectralUnmixingFilter->GetOutput(1));

            //http://docs.mitk.org/nightly/PipelineingConceptPage.html
            //m_SpectralUnmixingFilter->Update();
            //m_sO2->SetInput(m_SpectralUnmixingFilter->GetOutput()); 

            m_sO2->Update();

            mitk::Image::Pointer sO2 = m_sO2->GetOutput(0);
            mitk::DataNode::Pointer dataNodesO2 = mitk::DataNode::New();
            dataNodesO2->SetData(sO2);
            dataNodesO2->SetName("sO2");
            this->GetDataStorage()->Add(dataNodesO2);

            MITK_INFO << "[DONE]";
          }
          else
            mitkThrow() << "SELECT CHROMOPHORE OXYHEMOGLOBIN!";
        }
        else
          mitkThrow() << "SELECT CHROMOPHORE DEOXYHEMOGLOBIN!";
      }
      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());/**/

      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++/**/

      MITK_INFO << "Adding images to DataStorage...[DONE]";
    }
  }
}
