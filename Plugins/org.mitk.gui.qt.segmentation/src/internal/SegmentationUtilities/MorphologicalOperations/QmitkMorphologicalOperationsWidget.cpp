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

#include "QmitkMorphologicalOperationsWidget.h"
#include <mitkSliceNavigationController.h>
#include <mitkMorphologicalOperations.h>
#include "mitkProgressBar.h"

static const char* const HelpText = "Select a segmentation above";

QmitkMorphologicalOperationsWidget::QmitkMorphologicalOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  connect(m_Controls.btnClosing, SIGNAL(clicked()), this, SLOT(OnbtnClosingClicked()) );
  connect(m_Controls.btnOpening, SIGNAL(clicked()), this, SLOT(OnbtnOpeningClicked()) );
  connect(m_Controls.btnDilatation, SIGNAL(clicked()), this, SLOT(OnbtnDilatationClicked()) );
  connect(m_Controls.btnErosion, SIGNAL(clicked()), this, SLOT(OnbtnErosionClicked()) );
  connect(m_Controls.btnFillHoles, SIGNAL(clicked()), this, SLOT(OnbtnFillHolesClicked()) );

  connect(m_Controls.radioButtonMorphoCross, SIGNAL(clicked()), this, SLOT(OnRadioButtonsClicked()) );
  connect(m_Controls.radioButtonMorphoBall, SIGNAL(clicked()), this, SLOT(OnRadioButtonsClicked()) );

  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));
}

QmitkMorphologicalOperationsWidget::~QmitkMorphologicalOperationsWidget()
{
}


void QmitkMorphologicalOperationsWidget::OnSelectionChanged(unsigned int index, const mitk::DataNode* selection)
{
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);

  if (node.IsNotNull())
  {
    m_Controls.dataSelectionWidget->SetHelpText("");
    this->EnableButtons(true);
  }
  else
  {
    m_Controls.dataSelectionWidget->SetHelpText(HelpText);
    this->EnableButtons(false);
  }
}


void QmitkMorphologicalOperationsWidget::EnableButtons(bool enable)
{
  m_Controls.btnClosing->setEnabled(enable);
  m_Controls.btnDilatation->setEnabled(enable);
  m_Controls.btnErosion->setEnabled(enable);
  m_Controls.btnFillHoles->setEnabled(enable);
  m_Controls.btnOpening->setEnabled(enable);
}

void QmitkMorphologicalOperationsWidget::OnRadioButtonsClicked()
{
  m_Controls.sliderMorphFactor->setEnabled(
    m_Controls.radioButtonMorphoBall->isChecked());

  m_Controls.spinBoxMorphFactor->setEnabled(
    m_Controls.radioButtonMorphoBall->isChecked());
}

void QmitkMorphologicalOperationsWidget::OnbtnClosingClicked()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = dynamic_cast< mitk::Image* >(node->GetData());
  mitk::MorphologicalOperations::StructuralElementType structuralElement;

  if (m_Controls.radioButtonMorphoBall->isChecked())
    structuralElement = mitk::MorphologicalOperations::BALL;
  else
    structuralElement = mitk::MorphologicalOperations::CUBE;

  try
  {
    mitk::MorphologicalOperations::Closing(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch ( itk::ExceptionObject* itkObj)
  {
    MITK_WARN << "Exception caught: " << itkObj->GetDescription();
  }

  node->SetData(image);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMorphologicalOperationsWidget::OnbtnOpeningClicked()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = dynamic_cast< mitk::Image* >(node->GetData());
  mitk::MorphologicalOperations::StructuralElementType structuralElement;

  if (m_Controls.radioButtonMorphoBall->isChecked())
    structuralElement = mitk::MorphologicalOperations::BALL;
  else
    structuralElement = mitk::MorphologicalOperations::CUBE;

  try
  {
    mitk::MorphologicalOperations::Opening(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch ( itk::ExceptionObject* itkObj)
  {
    MITK_WARN << "Exception caught: " << itkObj->GetDescription();
  }

  node->SetData(image);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMorphologicalOperationsWidget::OnbtnDilatationClicked()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = dynamic_cast< mitk::Image* >(node->GetData());
  mitk::MorphologicalOperations::StructuralElementType structuralElement;

  if (m_Controls.radioButtonMorphoBall->isChecked())
    structuralElement = mitk::MorphologicalOperations::BALL;
  else
    structuralElement = mitk::MorphologicalOperations::CUBE;

  try
  {
    mitk::MorphologicalOperations::Dilate(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch ( itk::ExceptionObject* itkObj)
  {
    MITK_WARN << "Exception caught: " << itkObj->GetDescription();
  }

  node->SetData(image);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMorphologicalOperationsWidget::OnbtnErosionClicked()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = dynamic_cast< mitk::Image* >(node->GetData());
  mitk::MorphologicalOperations::StructuralElementType structuralElement;

  if (m_Controls.radioButtonMorphoBall->isChecked())
    structuralElement = mitk::MorphologicalOperations::BALL;
  else
    structuralElement = mitk::MorphologicalOperations::CUBE;

  try
  {
    mitk::MorphologicalOperations::Erode(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch ( itk::ExceptionObject* itkObj)
  {
    MITK_WARN << "Exception caught: " << itkObj->GetDescription();
  }

  node->SetData(image);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMorphologicalOperationsWidget::OnbtnFillHolesClicked()
{
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = dynamic_cast< mitk::Image* >(node->GetData());

  try
  {
    mitk::MorphologicalOperations::Fillhole(image);
  }
  catch ( itk::ExceptionObject* itkObj)
  {
    MITK_WARN << "Exception caught: " << itkObj->GetDescription();
  }

  node->SetData(image);
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}
