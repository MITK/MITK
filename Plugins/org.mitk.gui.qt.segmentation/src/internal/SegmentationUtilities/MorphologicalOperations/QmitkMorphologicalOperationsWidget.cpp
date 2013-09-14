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
#include <mitkMorphologicalOperations.h>
#include <mitkProgressBar.h>
#include <mitkSliceNavigationController.h>

static const char* const HelpText = "Select a segmentation above";

QmitkMorphologicalOperationsWidget::QmitkMorphologicalOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataStorageComboBox(QmitkDataSelectionWidget::SegmentationPredicate);
  m_Controls.dataSelectionWidget->SetHelpText(HelpText);

  connect(m_Controls.btnClosing, SIGNAL(clicked()), this, SLOT(OnClosingButtonClicked()));
  connect(m_Controls.btnOpening, SIGNAL(clicked()), this, SLOT(OnOpeningButtonClicked()));
  connect(m_Controls.btnDilatation, SIGNAL(clicked()), this, SLOT(OnDilatationButtonClicked()));
  connect(m_Controls.btnErosion, SIGNAL(clicked()), this, SLOT(OnErosionButtonClicked()));
  connect(m_Controls.btnFillHoles, SIGNAL(clicked()), this, SLOT(OnFillHolesButtonClicked()));
  connect(m_Controls.radioButtonMorphoCross, SIGNAL(clicked()), this, SLOT(OnRadioButtonsClicked()));
  connect(m_Controls.radioButtonMorphoBall, SIGNAL(clicked()), this, SLOT(OnRadioButtonsClicked()));
  connect(m_Controls.dataSelectionWidget, SIGNAL(SelectionChanged(unsigned int, const mitk::DataNode*)), this, SLOT(OnSelectionChanged(unsigned int, const mitk::DataNode*)));

  if (m_Controls.dataSelectionWidget->GetSelection(0).IsNotNull())
    this->OnSelectionChanged(0, m_Controls.dataSelectionWidget->GetSelection(0));
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
  bool enable = m_Controls.radioButtonMorphoBall->isChecked();

  m_Controls.sliderMorphFactor->setEnabled(enable);
  m_Controls.spinBoxMorphFactor->setEnabled(enable);
}

void QmitkMorphologicalOperationsWidget::OnClosingButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = static_cast<mitk::Image*>(node->GetData());
  bool ball = m_Controls.radioButtonMorphoBall->isChecked();

  mitk::MorphologicalOperations::StructuralElementType structuralElement = ball
    ? mitk::MorphologicalOperations::Ball
    : mitk::MorphologicalOperations::Cross;

  try
  {
    mitk::MorphologicalOperations::Closing(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch (const itk::ExceptionObject& exception)
  {
    MITK_WARN << "Exception caught: " << exception.GetDescription();

    QApplication::restoreOverrideCursor();
    return;
  }

  node->SetData(image);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QApplication::restoreOverrideCursor();
}

void QmitkMorphologicalOperationsWidget::OnOpeningButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = static_cast<mitk::Image*>(node->GetData());
  bool ball = m_Controls.radioButtonMorphoBall->isChecked();

  mitk::MorphologicalOperations::StructuralElementType structuralElement = ball
    ? mitk::MorphologicalOperations::Ball
    : mitk::MorphologicalOperations::Cross;

  try
  {
     mitk::MorphologicalOperations::Opening(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch (const itk::ExceptionObject& exception)
  {
     MITK_WARN << "Exception caught: " << exception.GetDescription();

     QApplication::restoreOverrideCursor();
     return;
  }

  node->SetData(image);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QApplication::restoreOverrideCursor();
}

void QmitkMorphologicalOperationsWidget::OnDilatationButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = static_cast<mitk::Image*>(node->GetData());
  bool ball = m_Controls.radioButtonMorphoBall->isChecked();

  mitk::MorphologicalOperations::StructuralElementType structuralElement = ball
    ? mitk::MorphologicalOperations::Ball
    : mitk::MorphologicalOperations::Cross;

  try
  {
    mitk::MorphologicalOperations::Dilate(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch (const itk::ExceptionObject& exception)
  {
    MITK_WARN << "Exception caught: " << exception.GetDescription();

    QApplication::restoreOverrideCursor();
    return;
  }

  node->SetData(image);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QApplication::restoreOverrideCursor();
}

void QmitkMorphologicalOperationsWidget::OnErosionButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = static_cast<mitk::Image*>(node->GetData());
  bool ball = m_Controls.radioButtonMorphoBall->isChecked();

  mitk::MorphologicalOperations::StructuralElementType structuralElement = ball
    ? mitk::MorphologicalOperations::Ball
    : mitk::MorphologicalOperations::Cross;

  try
  {
    mitk::MorphologicalOperations::Erode(image, m_Controls.spinBoxMorphFactor->value(), structuralElement);
  }
  catch (const itk::ExceptionObject& exception)
  {
    MITK_WARN << "Exception caught: " << exception.GetDescription();

    QApplication::restoreOverrideCursor();
    return;
 }

  node->SetData(image);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QApplication::restoreOverrideCursor();
}

void QmitkMorphologicalOperationsWidget::OnFillHolesButtonClicked()
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  QmitkDataSelectionWidget* dataSelectionWidget = m_Controls.dataSelectionWidget;
  mitk::DataNode::Pointer node = dataSelectionWidget->GetSelection(0);
  mitk::Image::Pointer image = static_cast<mitk::Image*>(node->GetData());

  try
  {
    mitk::MorphologicalOperations::FillHoles(image);
  }
  catch (const itk::ExceptionObject& exception)
  {
    MITK_WARN << "Exception caught: " << exception.GetDescription();

    QApplication::restoreOverrideCursor();
    return;
  }

  node->SetData(image);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  QApplication::restoreOverrideCursor();
}
