/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkAutoMLSegmentationToolGUIBase.h"
#include "mitkAutoMLSegmentationWithPreviewTool.h"

#include <qboxlayout.h>

QmitkAutoMLSegmentationToolGUIBase::QmitkAutoMLSegmentationToolGUIBase() : QmitkAutoSegmentationToolGUIBase(false)
{
  auto enableOtsuDelegate = [this](bool enabled)
  {
    bool result = enabled;
    auto tool = this->GetConnectedToolAs<mitk::AutoMLSegmentationWithPreviewTool>();
    if (nullptr != tool)
    {
      result = !tool->GetSelectedLabels().empty() && enabled;
    }
    else
    {
      result = false;
    }

    return result;
  };

  m_EnableConfirmSegBtnFnc = enableOtsuDelegate;
}

void QmitkAutoMLSegmentationToolGUIBase::ConnectNewTool(mitk::AutoSegmentationWithPreviewTool* newTool)
{
  Superclass::ConnectNewTool(newTool);
  this->EnableWidgets(true);
}

void QmitkAutoMLSegmentationToolGUIBase::InitializeUI(QBoxLayout* mainLayout)
{
  m_LabelSelectionList = new QmitkSimpleLabelSetListWidget(this);
  m_LabelSelectionList->setObjectName(QString::fromUtf8("m_LabelSelectionList"));
  QSizePolicy sizePolicy2(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);
  sizePolicy2.setHorizontalStretch(0);
  sizePolicy2.setVerticalStretch(0);
  sizePolicy2.setHeightForWidth(m_LabelSelectionList->sizePolicy().hasHeightForWidth());
  m_LabelSelectionList->setSizePolicy(sizePolicy2);
  m_LabelSelectionList->setMaximumSize(QSize(10000000, 10000000));

  mainLayout->addWidget(m_LabelSelectionList);

  connect(m_LabelSelectionList, &QmitkSimpleLabelSetListWidget::SelectedLabelsChanged, this, &QmitkAutoMLSegmentationToolGUIBase::OnLabelSelectionChanged);

  Superclass::InitializeUI(mainLayout);
}

void QmitkAutoMLSegmentationToolGUIBase::OnLabelSelectionChanged(const QmitkSimpleLabelSetListWidget::LabelVectorType& selectedLabels)
{
  auto tool = this->GetConnectedToolAs<mitk::AutoMLSegmentationWithPreviewTool>();
  if (nullptr != tool)
  {
    mitk::AutoMLSegmentationWithPreviewTool::SelectedLabelVectorType labelIDs;
    for (const auto& label : selectedLabels)
    {
      labelIDs.push_back(label->GetValue());
    }

    tool->SetSelectedLabels(labelIDs);
    tool->UpdatePreview();
    this->EnableWidgets(true); //used to actualize the ConfirmSeg btn via the delegate;
  }
}

void QmitkAutoMLSegmentationToolGUIBase::EnableWidgets(bool enabled)
{
  Superclass::EnableWidgets(enabled);
  if (nullptr != m_LabelSelectionList)
  {
    m_LabelSelectionList->setEnabled(enabled);
  }
}

void QmitkAutoMLSegmentationToolGUIBase::SetLabelSetPreview(const mitk::LabelSetImage* preview)
{
  if (nullptr != m_LabelSelectionList)
  {
    m_LabelSelectionList->SetLabelSetImage(preview);
  }
}
