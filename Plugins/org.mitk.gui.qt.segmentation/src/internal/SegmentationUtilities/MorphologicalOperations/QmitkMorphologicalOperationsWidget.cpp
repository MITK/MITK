/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMorphologicalOperationsWidget.h"
#include <mitkProgressBar.h>
#include <mitkSliceNavigationController.h>
#include <QCheckBox>

static const char* const HelpText = "Select a segmentation above";

QmitkMorphologicalOperationsWidget::QmitkMorphologicalOperationsWidget(mitk::SliceNavigationController* timeNavigationController, QWidget* parent)
  : QmitkSegmentationUtilityWidget(timeNavigationController, parent)
{
  m_Controls.setupUi(this);

  m_Controls.dataSelectionWidget->AddDataSelection(QmitkDataSelectionWidget::SegmentationPredicate);
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

void QmitkMorphologicalOperationsWidget::OnSelectionChanged(unsigned int, const mitk::DataNode*)
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
  mitk::MorphologicalOperations::StructuralElementType structuralElement = CreateStructerElement_UI();
  try
  {
    int factor = m_Controls.spinBoxMorphFactor->isEnabled()
      ? m_Controls.spinBoxMorphFactor->value()
      : 1;

    mitk::MorphologicalOperations::Closing(image, factor, structuralElement);
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

  mitk::MorphologicalOperations::StructuralElementType structuralElement = CreateStructerElement_UI();

  try
  {
    int factor = m_Controls.spinBoxMorphFactor->isEnabled()
      ? m_Controls.spinBoxMorphFactor->value()
      : 1;

     mitk::MorphologicalOperations::Opening(image, factor, structuralElement);
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
  mitk::MorphologicalOperations::StructuralElementType structuralElement = this->CreateStructerElement_UI();

  try
  {
    int factor = m_Controls.spinBoxMorphFactor->isEnabled()
      ? m_Controls.spinBoxMorphFactor->value()
      : 1;

    mitk::MorphologicalOperations::Dilate(image, factor, structuralElement);
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
mitk::MorphologicalOperations::StructuralElementType structuralElement = CreateStructerElement_UI();

  try
  {
    int factor = m_Controls.spinBoxMorphFactor->isEnabled()
      ? m_Controls.spinBoxMorphFactor->value()
      : 1;

    mitk::MorphologicalOperations::Erode(image, factor, structuralElement);
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


mitk::MorphologicalOperations::StructuralElementType QmitkMorphologicalOperationsWidget::CreateStructerElement_UI()
{
  bool ball = m_Controls.radioButtonMorphoBall->isChecked();
  int accum_flag = 0;
  if(ball){
   if(m_Controls.planeSelectionComboBox->currentIndex() == 0) accum_flag = mitk::MorphologicalOperations::Ball; // 3D Operation
   if(m_Controls.planeSelectionComboBox->currentIndex() == 1) accum_flag = mitk::MorphologicalOperations::Ball_Axial; // 2D Operation - Axial plane
   if(m_Controls.planeSelectionComboBox->currentIndex() == 2) accum_flag = mitk::MorphologicalOperations::Ball_Sagital; // 2D Operation - Sagital plane
   if(m_Controls.planeSelectionComboBox->currentIndex() == 3) accum_flag = mitk::MorphologicalOperations::Ball_Coronal; // 2D Operation - Coronal plane
  }else{
    if(m_Controls.planeSelectionComboBox->currentIndex() == 0) accum_flag = mitk::MorphologicalOperations::Cross;
    if(m_Controls.planeSelectionComboBox->currentIndex() == 1) accum_flag = mitk::MorphologicalOperations::Cross_Axial;
    if(m_Controls.planeSelectionComboBox->currentIndex() == 2) accum_flag = mitk::MorphologicalOperations::Cross_Sagital;
    if(m_Controls.planeSelectionComboBox->currentIndex() == 3) accum_flag = mitk::MorphologicalOperations::Cross_Coronal;
  }
  return (mitk::MorphologicalOperations::StructuralElementType)accum_flag;
}
