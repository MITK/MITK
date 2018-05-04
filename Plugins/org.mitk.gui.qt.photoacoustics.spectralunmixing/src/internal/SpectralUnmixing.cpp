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

/*//test mitk image
#include <mitkIOUtil.h>
#include <mitkImageWriteAccessor.h>
#include <mitkImageReadAccessor.h>*/

// Include to perform Spectral Unmixing
#include "mitkPASpectralUnmixingFilter.h"

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

// Checking which chromophores wanted for SU if none throw exeption!
void SpectralUnmixing::numberOfChromophores()
{
  auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilter::New();

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
      mitk::pa::SpectralUnmixingFilter::ChromophoreType::OXYGENATED_HEMOGLOBIN);
  }
  if (DeOxbool)
  {
    numberofChromophores += 1;
    MITK_INFO << "- Deoxygenated hemoglobin";
    // Set chromophore Deoxygenated hemoglobin:
    m_SpectralUnmixingFilter->AddChromophore(
      mitk::pa::SpectralUnmixingFilter::ChromophoreType::DEOXYGENATED_HEMOGLOBIN);
  }
  if (numberofChromophores == 0)
  {
    mitkThrow() << "PRESS 'IGNORE' AND CHOOSE A CHROMOPHORE!";
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

      MITK_INFO << "GENERATING DATA...";

      auto m_SpectralUnmixingFilter = mitk::pa::SpectralUnmixingFilter::New();

      m_SpectralUnmixingFilter->SetInput(image);

      // Wavelength implementation into fiter
      for (unsigned int imageIndex = 0; imageIndex < m_Wavelengths.size(); imageIndex++)
      {
        unsigned int wavelength = m_Wavelengths[imageIndex];
        m_SpectralUnmixingFilter->AddWavelength(wavelength);
      }
              
      MITK_INFO << "Updating Filter...";
      
      m_SpectralUnmixingFilter->Update();
     
      mitk::Image::Pointer HbO2 = m_SpectralUnmixingFilter->GetOutput(0);
      mitk::DataNode::Pointer dataNodeHbO2 = mitk::DataNode::New();
      dataNodeHbO2->SetData(HbO2);
      dataNodeHbO2->SetName("HbO2");
      this->GetDataStorage()->Add(dataNodeHbO2);  
       
      mitk::Image::Pointer Hb = m_SpectralUnmixingFilter->GetOutput(1);
      mitk::DataNode::Pointer dataNodeHb = mitk::DataNode::New();
      dataNodeHb->SetData(Hb);
      dataNodeHb->SetName("Hb");
      this->GetDataStorage()->Add(dataNodeHb);
      
      mitk::RenderingManager::GetInstance()->InitializeViewsByBoundingObjects(this->GetDataStorage());

      MITK_INFO << "Adding images to DataStorage...[DONE]";
    }
  }
}
