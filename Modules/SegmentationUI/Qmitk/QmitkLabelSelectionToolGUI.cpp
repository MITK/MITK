/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLabelSelectionToolGUI.h"

#include "QmitkFloatingTextPopup.h"

#include <mitkLabelSelectionTool.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkLabelSetImage.h>

#include <ui_QmitkLabelSelectionToolGUIControls.h>

MITK_TOOL_GUI_MACRO(MITKSEGMENTATIONUI_EXPORT, QmitkLabelSelectionToolGUI, "")

QmitkLabelSelectionToolGUI::QmitkLabelSelectionToolGUI() : QmitkToolGUI(),
  m_Controls(new Ui::QmitkLabelSelectionToolGUIControls),
  m_FloatingPopup(std::make_unique<QmitkFloatingTextPopup>())
{
  m_Controls->setupUi(this);

  connect(m_Controls->m_OnlyActiveGroupCheck, &QCheckBox::stateChanged, this, &QmitkLabelSelectionToolGUI::OnCheckChanged);

  connect(this, &QmitkLabelSelectionToolGUI::NewToolAssociated, this, &QmitkLabelSelectionToolGUI::OnNewToolAssociated);

  m_FloatingPopup->SetOpacity(100);
  m_FloatingPopup->SetEnabled(false);

  this->OnIndicatedLabelsChanged();
}

QmitkLabelSelectionToolGUI::~QmitkLabelSelectionToolGUI()
{
  if (m_LabelSelectionTool.IsNotNull())
  {
    m_LabelSelectionTool->IndicatedLabelsChanged -=
      mitk::MessageDelegate<QmitkLabelSelectionToolGUI>(this, &QmitkLabelSelectionToolGUI::OnIndicatedLabelsChanged);
  }
}

void QmitkLabelSelectionToolGUI::OnNewToolAssociated(mitk::Tool *tool)
{
  if (m_LabelSelectionTool.IsNotNull())
  {
    m_LabelSelectionTool->IndicatedLabelsChanged -=
      mitk::MessageDelegate<QmitkLabelSelectionToolGUI>(this, &QmitkLabelSelectionToolGUI::OnIndicatedLabelsChanged);
  }

  m_LabelSelectionTool = dynamic_cast<mitk::LabelSelectionTool *>(tool);

  if (m_LabelSelectionTool.IsNotNull())
  {
    m_LabelSelectionTool->IndicatedLabelsChanged +=
      mitk::MessageDelegate<QmitkLabelSelectionToolGUI>(this, &QmitkLabelSelectionToolGUI::OnIndicatedLabelsChanged);

    m_InternalEventCall = true;
    m_Controls->m_OnlyActiveGroupCheck->setChecked(m_LabelSelectionTool->GetCheckOnlyActiveGroup());
    m_InternalEventCall = false;
  }
}

void QmitkLabelSelectionToolGUI::OnIndicatedLabelsChanged()
{
  if (m_LabelSelectionTool.IsNotNull())
  {
    mitk::MultiLabelSegmentation::LabelValueVectorType labels = m_LabelSelectionTool->GetIndicatedLabels();
    auto segmentation = m_LabelSelectionTool->GetWorkingData();
    if (nullptr != segmentation)
    {
      QString content;

      for (auto it = labels.crbegin(); it != labels.crend(); ++it)
      {
        content.append("<p><b>\n");
        content.append(QString::fromStdString(mitk::LabelSetImageHelper::CreateHTMLLabelName(segmentation->GetLabel(*it), segmentation)));
        content.append("</b><br/>\n");
        content.append(QString::fromStdString(mitk::LabelSetImageHelper::CreateHTMLLabelDetails(segmentation->GetLabel(*it), segmentation)));
        content.append("</p>\n");
      }

      m_FloatingPopup->SetHtmlContent(content);
    }
    m_FloatingPopup->SetEnabled(!labels.empty());
  }
  else
  {
    m_FloatingPopup->SetEnabled(false);
  }
}

void QmitkLabelSelectionToolGUI::OnCheckChanged(int /*current*/)
{
  if (m_InternalEventCall)
    return;

  if (m_LabelSelectionTool.IsNotNull())
  {
    m_LabelSelectionTool->SetCheckOnlyActiveGroup(m_Controls->m_OnlyActiveGroupCheck->isChecked());
  }
}
