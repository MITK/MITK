/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLayersWidget.h"
#include <ui_QmitkLayersWidgetControls.h>

// mitk
#include <mitkDataNode.h>
#include <mitkLabelSetImage.h>
#include <mitkToolManagerProvider.h>

// Qt
#include <QMessageBox>

QmitkLayersWidget::QmitkLayersWidget(QWidget *parent)
  : QWidget(parent)
  , m_Controls(new Ui::QmitkLayersWidgetControls)
  , m_ToolManager(nullptr)
{
  m_Controls->setupUi(this);

  m_ToolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();

  connect(m_Controls->addLayerButton, &QToolButton::clicked, this, &QmitkLayersWidget::OnAddLayer);
  connect(m_Controls->deleteLayerButton, &QToolButton::clicked, this, &QmitkLayersWidget::OnDeleteLayer);
  connect(m_Controls->previousLayerButton, &QToolButton::clicked, this, &QmitkLayersWidget::OnPreviousLayer);
  connect(m_Controls->nextLayerButton, &QToolButton::clicked, this, &QmitkLayersWidget::OnNextLayer);
  connect(m_Controls->activeLayerComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &QmitkLayersWidget::OnChangeLayer);

  this->UpdateGUI();
}

QmitkLayersWidget::~QmitkLayersWidget()
{
  delete m_Controls;
}

void QmitkLayersWidget::UpdateGUI()
{
  m_Controls->addLayerButton->setEnabled(false);
  m_Controls->deleteLayerButton->setEnabled(false);
  m_Controls->previousLayerButton->setEnabled(false);
  m_Controls->nextLayerButton->setEnabled(false);
  m_Controls->activeLayerComboBox->setEnabled(false);

  mitk::LabelSetImage* workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  m_Controls->addLayerButton->setEnabled(true);

  m_Controls->activeLayerComboBox->blockSignals(true);
  m_Controls->activeLayerComboBox->clear();

  unsigned int numberOfLayers = workingImage->GetNumberOfLayers();
  for (unsigned int lidx = 0; lidx < numberOfLayers; ++lidx)
  {
    m_Controls->activeLayerComboBox->addItem(QString::number(lidx));
  }

  unsigned int activeLayer = workingImage->GetActiveLayer();
  m_Controls->activeLayerComboBox->setCurrentIndex(activeLayer);
  m_Controls->activeLayerComboBox->blockSignals(false);

  m_Controls->deleteLayerButton->setEnabled(numberOfLayers > 1);
  m_Controls->previousLayerButton->setEnabled(activeLayer > 0);
  m_Controls->nextLayerButton->setEnabled(activeLayer != numberOfLayers - 1);
  m_Controls->activeLayerComboBox->setEnabled(numberOfLayers > 0);
}

mitk::LabelSetImage* QmitkLayersWidget::GetWorkingImage()
{
  mitk::DataNode* workingNode = GetWorkingNode();
  if (nullptr == workingNode)
  {
    return nullptr;
  }

  auto workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
  return workingImage;
}

mitk::DataNode* QmitkLayersWidget::GetWorkingNode()
{
  mitk::DataNode* workingNode = m_ToolManager->GetWorkingData(0);
  return workingNode;
}

void QmitkLayersWidget::OnAddLayer()
{
  m_ToolManager->ActivateTool(-1);

  auto workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  try
  {
    this->WaitCursorOn();
    workingImage->AddLayer();
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      this, "Add layer", "Could not add a new layer. See error log for details.\n");
    return;
  }

  this->UpdateGUI();
  emit LayersChanged();
}

void QmitkLayersWidget::OnDeleteLayer()
{
  m_ToolManager->ActivateTool(-1);

  auto workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  if (workingImage->GetNumberOfLayers() < 2)
  {
    return;
  }

  QString question = "Do you really want to delete the current layer?";
  QMessageBox::StandardButton answerButton = QMessageBox::question(
    this, "Delete layer", question, QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Yes);
  
  if (answerButton != QMessageBox::Yes)
  {
    return;
  }

  try
  {
    this->WaitCursorOn();
    workingImage->RemoveLayer();
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      this, "Delete layer", "Could not delete the currently active layer. See error log for details.\n");
    return;
  }

  this->UpdateGUI();
  emit LayersChanged();
}

void QmitkLayersWidget::OnPreviousLayer()
{
  auto workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  this->OnChangeLayer(workingImage->GetActiveLayer() - 1);
}

void QmitkLayersWidget::OnNextLayer()
{
  auto workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  this->OnChangeLayer(workingImage->GetActiveLayer() + 1);
}

void QmitkLayersWidget::OnChangeLayer(int layer)
{
  m_ToolManager->ActivateTool(-1);

  auto workingImage = this->GetWorkingImage();
  if (nullptr == workingImage)
  {
    return;
  }

  try
  {
    this->WaitCursorOn();
    workingImage->SetActiveLayer(layer);
    this->WaitCursorOff();
  }
  catch (mitk::Exception& e)
  {
    this->WaitCursorOff();
    MITK_ERROR << "Exception caught: " << e.GetDescription();
    QMessageBox::information(
      this, "Change layer", "Could not change the layer. See error log for details.\n");
    return;
  }

  this->UpdateGUI();
  emit LayersChanged();
}

void QmitkLayersWidget::WaitCursorOn()
{
  QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
}

void QmitkLayersWidget::WaitCursorOff()
{
  this->RestoreOverrideCursor();
}

void QmitkLayersWidget::RestoreOverrideCursor()
{
  QApplication::restoreOverrideCursor();
}

