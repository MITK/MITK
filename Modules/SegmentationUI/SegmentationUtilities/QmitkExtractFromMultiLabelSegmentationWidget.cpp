/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkExtractFromMultiLabelSegmentationWidget.h"
#include <ui_QmitkExtractFromMultiLabelSegmentationWidgetControls.h>

#include <mitkDataStorage.h>
#include <mitkException.h>
#include <mitkExceptionMacro.h>
#include <mitkProgressBar.h>
#include <mitkProperties.h>
#include <mitkImage.h>
#include <mitkLabelSetImage.h>
#include <mitkMultiLabelPredicateHelper.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkLabelSetImageConverter.h>

#include <QmitkNodeSelectionDialog.h>

QmitkExtractFromMultiLabelSegmentationWidget::QmitkExtractFromMultiLabelSegmentationWidget(mitk::DataStorage* dataStorage, QWidget* parent)
  : QWidget(parent), m_DataStorage(dataStorage)
{
  m_Controls = new Ui::QmitkExtractFromMultiLabelSegmentationWidgetControls;
  m_Controls->setupUi(this);

  m_Controls->segNodeSelector->SetDataStorage(dataStorage);
  m_Controls->segNodeSelector->SetNodePredicate(mitk::GetMultiLabelSegmentationPredicate());
  m_Controls->segNodeSelector->SetSelectionIsOptional(false);
  m_Controls->segNodeSelector->SetInvalidInfo(QStringLiteral("Please select segmentation for extraction."));
  m_Controls->segNodeSelector->SetPopUpTitel(QStringLiteral("Select segmentation"));
  m_Controls->segNodeSelector->SetPopUpHint(QStringLiteral("Select the segmentation that should be used as source for extraction."));

  m_Controls->labelInspector->SetMultiSelectionMode(true);

  m_Controls->checkExtractSelected->setChecked(false);

  m_Controls->checkClassMap->setChecked(false);
  m_Controls->checkInstanceMap->setChecked(true);
  m_Controls->checkInstanceMask->setChecked(true);

  this->ConfigureWidgets();

  connect (m_Controls->btnExtract, &QAbstractButton::clicked, this, &QmitkExtractFromMultiLabelSegmentationWidget::OnExtractPressed);

  connect(m_Controls->segNodeSelector, &QmitkAbstractNodeSelectionWidget::CurrentSelectionChanged,
    this, &QmitkExtractFromMultiLabelSegmentationWidget::OnSegSelectionChanged);

  auto widget = this;
  connect(m_Controls->checkInstanceMap, &QCheckBox::toggled,
    this, [widget](bool) {widget->ConfigureWidgets(); });
  connect(m_Controls->checkClassMap, &QCheckBox::toggled,
    this, [widget](bool) {widget->ConfigureWidgets(); });
  connect(m_Controls->checkInstanceMask, &QCheckBox::toggled,
    this, [widget](bool) {widget->ConfigureWidgets(); });
  connect(m_Controls->checkExtractSelected, &QCheckBox::toggled,
    this, [widget](bool) {widget->ConfigureWidgets(); });

  m_Controls->segNodeSelector->SetAutoSelectNewNodes(true);
}

QmitkExtractFromMultiLabelSegmentationWidget::~QmitkExtractFromMultiLabelSegmentationWidget()
{
  m_Controls->labelInspector->SetMultiLabelNode(nullptr);
}

void QmitkExtractFromMultiLabelSegmentationWidget::ConfigureWidgets()
{
  m_InternalEvent = true;
  m_Controls->labelInspector->setVisible(m_Controls->checkExtractSelected->isChecked());

  bool isOK = (m_Controls->checkInstanceMap->isChecked() || m_Controls->checkInstanceMask->isChecked() || m_Controls->checkClassMap->isChecked());
  m_Controls->btnExtract->setEnabled(isOK);

  m_InternalEvent = false;
}

void QmitkExtractFromMultiLabelSegmentationWidget::OnSegSelectionChanged(QmitkAbstractNodeSelectionWidget::NodeList /*nodes*/)
{
  if (!m_InternalEvent)
  {
    m_Controls->labelInspector->SetMultiLabelNode(m_Controls->segNodeSelector->GetSelectedNode());
    this->ConfigureWidgets();
  }
}

void QmitkExtractFromMultiLabelSegmentationWidget::StoreToDataStorage(mitk::Image* image, const std::string& name, mitk::DataNode* parent)
{
  auto dataStorage = m_DataStorage.Lock();
  if (dataStorage.IsNull())
  {
    mitkThrow() << "QmitkExtractFromMultiLabelSegmentationWidget is in invalid state. No datastorage is set.";
  }

  mitk::DataNode::Pointer outNode = mitk::DataNode::New();
  outNode->SetName(name);
  outNode->SetData(image);

  dataStorage->Add(outNode, parent);
}

void QmitkExtractFromMultiLabelSegmentationWidget::OnExtractPressed()
{
  auto node = m_Controls->segNodeSelector->GetSelectedNodes().front();

  auto seg = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

  auto selectedLabelValues = seg->GetAllLabelValues();
  if (m_Controls->checkExtractSelected->isChecked())
    selectedLabelValues = m_Controls->labelInspector->GetSelectedLabels();

  auto groupLabelValueMap = mitk::LabelSetImageHelper::SplitLabelValuesByGroup(seg,selectedLabelValues);

  unsigned int numSteps = 0;
  if (m_Controls->checkInstanceMap->isChecked()) numSteps += groupLabelValueMap.size();
  if (m_Controls->checkInstanceMask->isChecked()) numSteps += selectedLabelValues.size();
  if (m_Controls->checkClassMap->isChecked()) numSteps += groupLabelValueMap.size();

  mitk::ProgressBar::GetInstance()->Reset();
  mitk::ProgressBar::GetInstance()->AddStepsToDo(numSteps);

  for (auto& [groupID, labelValues] : groupLabelValueMap)
  {
    if (m_Controls->checkInstanceMap->isChecked())
    {
      auto image = seg->GetGroupImageWorkarround(groupID)->Clone();
      std::string name = "InstanceMap group "+std::to_string(groupID);
      this->StoreToDataStorage(image, name, node);
      mitk::ProgressBar::GetInstance()->Progress();
    }
    if (m_Controls->checkClassMap->isChecked())
    {
      auto [image,lookup] = mitk::CreateLabelClassMap(seg, groupID, labelValues);
      std::string name = "ClassMap group " + std::to_string(groupID);
      this->StoreToDataStorage(image, name, node);
      mitk::ProgressBar::GetInstance()->Progress();
    }
    if (m_Controls->checkInstanceMask->isChecked())
    {
      for (auto& labelValue : labelValues)
      {
        auto image = mitk::CreateLabelMask(seg,labelValue,false);
        std::string name = "LabelMask " + seg->GetLabel(labelValue)->GetName() + " [" + std::to_string(labelValue) + "]";
        this->StoreToDataStorage(image, name, node);
        mitk::ProgressBar::GetInstance()->Progress();
      }
    }
  }
  mitk::ProgressBar::GetInstance()->Reset();
}
