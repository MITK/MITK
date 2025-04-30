/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMorphologicalOperationsWidget.h"
#include <ui_QmitkMorphologicalOperationsWidgetControls.h>

#include <mitkMultiLabelPredicateHelper.h>
#include <mitkProgressBar.h>
#include <mitkLabelSetImageConverter.h>
#include <mitkLabelSetImageHelper.h>

QmitkMorphologicalOperationsWidget::QmitkMorphologicalOperationsWidget(mitk::DataStorage* dataStorage, QWidget* parent)
  : QWidget(parent)
{
  m_Controls = new Ui::QmitkMorphologicalOperationsWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->segNodeSelector->SetDataStorage(dataStorage);
  m_Controls->segNodeSelector->SetNodePredicate(mitk::GetMultiLabelSegmentationPredicate());
  m_Controls->segNodeSelector->SetSelectionIsOptional(false);
  m_Controls->segNodeSelector->SetInvalidInfo(QStringLiteral("Please select segmentation"));
  m_Controls->segNodeSelector->SetPopUpTitel(QStringLiteral("Select segmentation"));
  m_Controls->segNodeSelector->SetPopUpHint(QStringLiteral("Select the segmentation that should be modified."));

  connect(m_Controls->btnClosing, SIGNAL(clicked()), this, SLOT(OnClosingButtonClicked()));
  connect(m_Controls->btnOpening, SIGNAL(clicked()), this, SLOT(OnOpeningButtonClicked()));
  connect(m_Controls->btnDilatation, SIGNAL(clicked()), this, SLOT(OnDilatationButtonClicked()));
  connect(m_Controls->btnErosion, SIGNAL(clicked()), this, SLOT(OnErosionButtonClicked()));
  connect(m_Controls->btnFillHoles, SIGNAL(clicked()), this, SLOT(OnFillHolesButtonClicked()));
  connect(m_Controls->radioButtonMorphoCross, SIGNAL(clicked()), this, SLOT(OnRadioButtonsClicked()));
  connect(m_Controls->radioButtonMorphoBall, SIGNAL(clicked()), this, SLOT(OnRadioButtonsClicked()));
  connect(m_Controls->segNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkMorphologicalOperationsWidget::OnSelectionChanged);

  connect(m_Controls->labelInspector, &QmitkMultiLabelInspector::CurrentSelectionChanged,
    this, [this](mitk::MultiLabelSegmentation::LabelValueVectorType /*labels*/) {this->ConfigureButtons(); });

  m_Controls->segNodeSelector->SetAutoSelectNewNodes(true);
}

QmitkMorphologicalOperationsWidget::~QmitkMorphologicalOperationsWidget()
{
  m_Controls->labelInspector->SetMultiLabelNode(nullptr);
}

void QmitkMorphologicalOperationsWidget::OnSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  auto node = m_Controls->segNodeSelector->GetSelectedNode();
  m_Controls->labelInspector->SetMultiLabelNode(node);

  this->ConfigureButtons();
}

void QmitkMorphologicalOperationsWidget::ConfigureButtons()
{
  auto node = m_Controls->segNodeSelector->GetSelectedNode();

  bool enable = node.IsNotNull() && !m_Controls->labelInspector->GetSelectedLabels().empty();

  m_Controls->btnClosing->setEnabled(enable);
  m_Controls->btnDilatation->setEnabled(enable);
  m_Controls->btnErosion->setEnabled(enable);
  m_Controls->btnFillHoles->setEnabled(enable);
  m_Controls->btnOpening->setEnabled(enable);
}

void QmitkMorphologicalOperationsWidget::OnRadioButtonsClicked()
{
  bool enable = m_Controls->radioButtonMorphoBall->isChecked();

  m_Controls->sliderMorphFactor->setEnabled(enable);
  m_Controls->spinBoxMorphFactor->setEnabled(enable);
}

void QmitkMorphologicalOperationsWidget::OnClosingButtonClicked()
{
  this->Processing(mitk::MorphologicalOperations::Closing, "Closing");
}

void QmitkMorphologicalOperationsWidget::OnOpeningButtonClicked()
{
  this->Processing(mitk::MorphologicalOperations::Opening, "Open");
}

void QmitkMorphologicalOperationsWidget::OnDilatationButtonClicked()
{
  this->Processing(mitk::MorphologicalOperations::Dilate, "Dilate");
}

void QmitkMorphologicalOperationsWidget::OnErosionButtonClicked()
{
  this->Processing(mitk::MorphologicalOperations::Erode, "Erode");
}

void QmitkMorphologicalOperationsWidget::OnFillHolesButtonClicked()
{
  auto wrapper = [](mitk::Image::Pointer& image, int,
    mitk::MorphologicalOperations::StructuralElementType)
    {
      mitk::MorphologicalOperations::FillHoles(image);
    };

  this->Processing(wrapper, "FillHoles");
}

mitk::MorphologicalOperations::StructuralElementType QmitkMorphologicalOperationsWidget::CreateStructerElement_UI() const
{
  bool isBall = m_Controls->radioButtonMorphoBall->isChecked();
  int accum_flag = 0;
  if(isBall){
   if(m_Controls->planeSelectionComboBox->currentIndex() == 0) accum_flag = mitk::MorphologicalOperations::Ball; // 3D Operation
   if(m_Controls->planeSelectionComboBox->currentIndex() == 1) accum_flag = mitk::MorphologicalOperations::Ball_Axial; // 2D Operation - Axial plane
   if(m_Controls->planeSelectionComboBox->currentIndex() == 2) accum_flag = mitk::MorphologicalOperations::Ball_Sagittal; // 2D Operation - Sagittal plane
   if(m_Controls->planeSelectionComboBox->currentIndex() == 3) accum_flag = mitk::MorphologicalOperations::Ball_Coronal; // 2D Operation - Coronal plane
  }else{
    if(m_Controls->planeSelectionComboBox->currentIndex() == 0) accum_flag = mitk::MorphologicalOperations::Cross;
    if(m_Controls->planeSelectionComboBox->currentIndex() == 1) accum_flag = mitk::MorphologicalOperations::Cross_Axial;
    if(m_Controls->planeSelectionComboBox->currentIndex() == 2) accum_flag = mitk::MorphologicalOperations::Cross_Sagittal;
    if(m_Controls->planeSelectionComboBox->currentIndex() == 3) accum_flag = mitk::MorphologicalOperations::Cross_Coronal;
  }
  return (mitk::MorphologicalOperations::StructuralElementType)accum_flag;
}

mitk::Image::Pointer QmitkMorphologicalOperationsWidget::GetSelectedLabelMask() const
{
  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation(); 
  if (seg == nullptr) mitkThrow() << "Widget is in invalid state. Processing was triggered with no segmentation selected.";

  auto labels = m_Controls->labelInspector->GetSelectedLabels();
  if (labels.empty()) mitkThrow() << "Widget is in invalid state. Processing was triggered with no label selected.";

  return mitk::CreateLabelMask(seg, labels.front(), true);
}

void QmitkMorphologicalOperationsWidget::SaveResultLabelMask(const mitk::Image* resultMask, const std::string& labelName) const
{
  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();
  if (seg == nullptr) mitkThrow() << "Widget is in invalid state. Processing was triggered with no segmentation selected.";

  auto labels = m_Controls->labelInspector->GetSelectedLabels();
  if (labels.empty()) mitkThrow() << "Widget is in invalid state. Processing was triggered with no label selected.";

  if (m_Controls->checkNewLabel->isChecked())
  {
    auto groupID = seg->AddGroup();
    auto newLabel = mitk::LabelSetImageHelper::CreateNewLabel(seg, labelName, true);
    seg->AddLabelWithContent(newLabel, resultMask, groupID, 1);
  }
  else
  {
    auto groupID = seg->GetGroupIndexOfLabel(labels.front());
    mitk::TransferLabelContent(resultMask, seg->GetGroupImage(groupID), seg->GetConstLabelsByValue(seg->GetLabelValuesByGroup(groupID)),
      mitk::MultiLabelSegmentation::UNLABELED_VALUE, mitk::MultiLabelSegmentation::UNLABELED_VALUE, false, { {1, labels.front()} },
      mitk::MultiLabelSegmentation::MergeStyle::Replace, mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks);
  }

  m_Controls->labelInspector->GetMultiLabelSegmentation()->Modified();
  m_Controls->labelInspector->GetMultiLabelNode()->Modified();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkMorphologicalOperationsWidget::Processing(std::function<MorphFunctionType> morphFunction, const std::string& opsName) const
{
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(3);

  mitk::Image::Pointer image = this->GetSelectedLabelMask();
  mitk::ProgressBar::GetInstance()->Progress();

  mitk::MorphologicalOperations::StructuralElementType structuralElement = CreateStructerElement_UI();

  int factor = m_Controls->spinBoxMorphFactor->isEnabled()
    ? m_Controls->spinBoxMorphFactor->value()
    : 1;

  try
  {
    morphFunction(image, factor, structuralElement);
  }
  catch (const itk::ExceptionObject& exception)
  {
    MITK_WARN << "Exception caught: " << exception.GetDescription();

    QApplication::restoreOverrideCursor();
    mitk::ProgressBar::GetInstance()->Reset();
    return;
  }
  mitk::ProgressBar::GetInstance()->Progress();

  auto seg = m_Controls->labelInspector->GetMultiLabelSegmentation();
  auto labels = m_Controls->labelInspector->GetSelectedLabels();

  std::stringstream labelName;
  labelName << opsName << " " << " (r=" << factor << ") " << seg->GetLabel(labels.front())->GetName();

  this->SaveResultLabelMask(image, labelName.str());
  mitk::ProgressBar::GetInstance()->Progress();

  QApplication::restoreOverrideCursor();
  mitk::ProgressBar::GetInstance()->Reset();
}
