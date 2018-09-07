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
#include "PAUSMotionCompensation.h"

// Qt
#include <QMessageBox>

// mitk image
#include <mitkImage.h>

const std::string PAUSMotionCompensation::VIEW_ID = "org.mitk.views.pausmotioncompensation";

void PAUSMotionCompensation::SetFocus()
{
  m_Controls.buttonPerformImageProcessing->setFocus();
}

void PAUSMotionCompensation::CreateQtPartControl(QWidget *parent)
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi(parent);
  connect(
    m_Controls.buttonPerformImageProcessing, &QPushButton::clicked, this, &PAUSMotionCompensation::DoImageProcessing);
  connect(
    m_Controls.buttonUpdateParameters, &QPushButton::clicked, this, &PAUSMotionCompensation::DoUpdateParameters);
}

void PAUSMotionCompensation::OnSelectionChanged(berry::IWorkbenchPart::Pointer /*source*/,
                                                const QList<mitk::DataNode::Pointer> &nodes)
{
  // Clear the combo box where we choose the photoacoustic image
  m_Controls.comboBoxPA->clear();

  // Make sure that there are exactle 2 nodes selected
  if (nodes.size() != 2) {
    m_Controls.labelWarning->setVisible(true);
    m_Controls.buttonPerformImageProcessing->setEnabled(false);
    return;
  }

  // Make sure that the image data is okay
  foreach (mitk::DataNode::Pointer node, nodes) {
    if (node.IsNull() || !dynamic_cast<mitk::Image *>(node->GetData())) {
      m_Controls.labelWarning->setVisible(true);
      m_Controls.buttonPerformImageProcessing->setEnabled(false);
      return;
    }

    // Add the image names to the combo box
    m_Controls.comboBoxPA->addItem(QString::fromStdString(node->GetName()));
  }

  m_Controls.labelWarning->setVisible(false);
  m_Controls.buttonPerformImageProcessing->setEnabled(true);

}

void PAUSMotionCompensation::DoImageProcessing()
{
  QList<mitk::DataNode::Pointer> nodes = this->GetDataManagerSelection();

  // Make sure that there are two images selected
  if (nodes.empty() || nodes.size() != 2)
  {
    QMessageBox::information(nullptr, "Warning", "Please select two images before starting image processing.");
    return;
  }

  mitk::DataNode::Pointer paNode, usNode;

  foreach (mitk::DataNode::Pointer node, nodes) {
    if(m_Controls.comboBoxPA->itemText(m_Controls.comboBoxPA->currentIndex()) == QString::fromStdString(node->GetName())) {
      paNode = node;
    } else {
      usNode = node;
    }
  }

  if(paNode.IsNull() || usNode.IsNull()) {
    MITK_INFO << "One of the nodes is empty. This may happen, if the two images have the same name. Please rename one of them.";
    QMessageBox::information(nullptr, "Template", "One of the nodes is empty. This may happen, if the two images have the same name. Please rename one of them.");
    return;
  }

  // here we have a valid mitk::DataNode

  // a node itself is not very useful, we need its data item (the image)
  mitk::BaseData *paData = paNode->GetData();
  mitk::BaseData *usData = usNode->GetData();
  if (paData && usData)
  {
    // test if this data item is an image or not (could also be a surface or something totally different)
    mitk::Image *paImage = dynamic_cast<mitk::Image *>(paData);
    mitk::Image *usImage = dynamic_cast<mitk::Image *>(usData);
    if (paImage && usImage)
    {
      std::stringstream message;
      std::string name;
      message << "Performing motion compensation for image ";
      if (paNode->GetName(name))
      {
        // a property called "name" was found for this DataNode
        message << "'" << name << "'";
      }
      message << " and ";
      if (usNode->GetName(name)) {
        message << "'" << name << "'";
      }
      message << ".";
      MITK_INFO << message.str();

      // actually do something here...
      m_Filter->SetPaInput(paImage);
      m_Filter->SetUsInput(usImage);
      try{
        m_Filter->Update();
      }
      catch (const std::exception& ) {
        QMessageBox::information(nullptr, "Warning", "Please make sure that both input images have the same dimension(s).");
      }
      mitk::Image::Pointer paOutput = m_Filter->GetPaOutput();
      mitk::Image::Pointer usOutput = m_Filter->GetUsOutput();

      auto paOutNode = mitk::DataNode::New();
      paOutNode->SetData(paOutput);
      paOutNode->SetName(paNode->GetName() + " compensated");

      this->GetDataStorage()->Add(paOutNode);

      auto usOutNode = mitk::DataNode::New();
      usOutNode->SetData(usOutput);
      usOutNode->SetName(usNode->GetName() + " compensated");

      this->GetDataStorage()->Add(usOutNode);

      mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    }
  }
}

void PAUSMotionCompensation::DoUpdateParameters() {

  bool ok;

  unsigned int batchSize = m_Controls.lineBatchSize->text().toInt(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetBatchSize(batchSize);

  double pyrScale = m_Controls.linePyrScale->text().toDouble(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetPyrScale(pyrScale);

  unsigned int levels = m_Controls.lineLevels->text().toInt(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetLevels(levels);

  unsigned int winSize = m_Controls.lineWinSize->text().toInt(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetWinSize(winSize);

  unsigned int iterations = m_Controls.lineIterations->text().toInt(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetIterations(iterations);

  unsigned int polyN = m_Controls.linePolyN->text().toInt(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetPolyN(polyN);

  double polySigma = m_Controls.linePolySigma->text().toDouble(&ok);
  if(!ok) {
    QMessageBox::information(nullptr, "Warning", "Invalid parameter.");
    return;
  }
  m_Filter->SetPolySigma(polySigma);
}
