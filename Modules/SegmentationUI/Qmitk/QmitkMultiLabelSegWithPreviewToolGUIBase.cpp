/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkMultiLabelSegWithPreviewToolGUIBase.h"
#include "mitkSegWithPreviewTool.h"

#include <QRadioButton>
#include <QBoxLayout>

QmitkMultiLabelSegWithPreviewToolGUIBase::QmitkMultiLabelSegWithPreviewToolGUIBase() : QmitkSegWithPreviewToolGUIBase(false)
{
  auto enableMLSelectedDelegate = [this](bool enabled)
  {
    auto tool = this->GetConnectedToolAs<mitk::SegWithPreviewTool>();
    return nullptr != tool
      ? (tool->GetLabelTransferScope() == mitk::SegWithPreviewTool::LabelTransferScope::AllLabels || !tool->GetSelectedLabels().empty()) && enabled
      : false;
  };

  m_EnableConfirmSegBtnFnc = enableMLSelectedDelegate;
}

void QmitkMultiLabelSegWithPreviewToolGUIBase::InitializeUI(QBoxLayout* mainLayout)
{
  auto radioTransferAll = new QRadioButton("Transfer all labels", this);
  radioTransferAll->setToolTip("Transfer all preview labels when confirmed.");
  radioTransferAll->setChecked(true);
  connect(radioTransferAll, &QAbstractButton::toggled, this, &QmitkMultiLabelSegWithPreviewToolGUIBase::OnRadioTransferAllClicked);
  mainLayout->addWidget(radioTransferAll);
  m_RadioTransferAll = radioTransferAll;

  auto radioTransferSelected = new QRadioButton("Transfer selected labels", this);
  radioTransferSelected->setToolTip("Transfer the selected preview labels when confirmed.");
  radioTransferSelected->setChecked(false);
  mainLayout->addWidget(radioTransferSelected);
  m_RadioTransferSelected = radioTransferSelected;

  m_LabelSelectionList = new QmitkSimpleLabelSetListWidget(this);
  m_LabelSelectionList->setObjectName(QString::fromUtf8("m_LabelSelectionList"));
  QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
  sizePolicy2.setHorizontalStretch(0);
  sizePolicy2.setVerticalStretch(0);
  sizePolicy2.setHeightForWidth(m_LabelSelectionList->sizePolicy().hasHeightForWidth());
  m_LabelSelectionList->setSizePolicy(sizePolicy2);
  m_LabelSelectionList->setMaximumSize(QSize(10000000, 10000000));
  m_LabelSelectionList->setVisible(false);

  mainLayout->addWidget(m_LabelSelectionList);
  connect(m_LabelSelectionList, &QmitkSimpleLabelSetListWidget::SelectedLabelsChanged, this, &QmitkMultiLabelSegWithPreviewToolGUIBase::OnLabelSelectionChanged);

  this->OnRadioTransferAllClicked(true);

  Superclass::InitializeUI(mainLayout);
}

void QmitkMultiLabelSegWithPreviewToolGUIBase::OnLabelSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels)
{
  auto tool = this->GetConnectedToolAs<mitk::SegWithPreviewTool>();
  if (nullptr != tool)
  {
    mitk::SegWithPreviewTool::SelectedLabelVectorType labelIDs;
    for (const auto& label : selectedLabels)
    {
      labelIDs.push_back(label->GetValue());
    }

    tool->SetSelectedLabels(labelIDs);
    this->ActualizePreviewLabelVisibility();
    this->EnableWidgets(true); //used to actualize the ConfirmSeg btn via the delegate;
  }
}

void QmitkMultiLabelSegWithPreviewToolGUIBase::ActualizePreviewLabelVisibility()
{
  auto tool = this->GetConnectedToolAs<mitk::SegWithPreviewTool>();
  if (nullptr != tool)
  {
    auto preview = tool->GetPreviewSegmentation();
    if (nullptr != preview)
    {
      auto labelSet = preview->GetActiveLabelSet();
      auto selectedLabels = tool->GetSelectedLabels();

      for (auto labelIter = labelSet->IteratorBegin(); labelIter != labelSet->IteratorEnd(); ++labelIter)
      {
        bool isVisible = tool->GetLabelTransferScope() == mitk::SegWithPreviewTool::LabelTransferScope::AllLabels
          || (std::find(selectedLabels.begin(), selectedLabels.end(), labelIter->second->GetValue()) != selectedLabels.end());
        labelIter->second->SetVisible(isVisible);
        labelSet->UpdateLookupTable(labelIter->second->GetValue());
      }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkMultiLabelSegWithPreviewToolGUIBase::OnRadioTransferAllClicked(bool checked)
{
  m_LabelSelectionList->setVisible(!checked);

  auto tool = this->GetConnectedToolAs<mitk::SegWithPreviewTool>();
  if (nullptr != tool)
  {
    if (checked)
    {
      tool->SetLabelTransferScope(mitk::SegWithPreviewTool::LabelTransferScope::AllLabels);
    }
    else
    {
      tool->SetLabelTransferScope(mitk::SegWithPreviewTool::LabelTransferScope::SelectedLabels);
    }
  }
  this->ActualizePreviewLabelVisibility();
}

void QmitkMultiLabelSegWithPreviewToolGUIBase::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  if (nullptr != m_LabelSelectionList)
  {
    m_LabelSelectionList->setEnabled(enabled);
  }

  if (nullptr != m_RadioTransferAll)
  {
    m_RadioTransferAll->setEnabled(enabled);
  }

  if (nullptr != m_RadioTransferSelected)
  {
    m_RadioTransferSelected->setEnabled(enabled);
  }
}

void QmitkMultiLabelSegWithPreviewToolGUIBase::SetLabelSetPreview(const mitk::LabelSetImage* preview)
{
  if (nullptr != m_LabelSelectionList)
  {
    m_LabelSelectionList->SetLabelSetImage(preview);
  }
}
