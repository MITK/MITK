/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCopyLabelToGroupDialog.h"
#include <ui_QmitkCopyLabelToGroupDialog.h>

#include <mitkLabelSetImageHelper.h>

namespace
{
  QString GetDisplayedLabelName(const mitk::MultiLabelSegmentation* segmentation, const mitk::Label* label)
  {
    return QString::fromStdString(mitk::LabelSetImageHelper::CreateDisplayLabelName(segmentation, label));
  }

  QString GetDisplayedGroupName(const mitk::MultiLabelSegmentation* segmentation, mitk::MultiLabelSegmentation::GroupIndexType group)
  {
    return QString::fromStdString(mitk::LabelSetImageHelper::CreateDisplayGroupName(segmentation, group));
  }
}

QmitkCopyLabelToGroupDialog::QmitkCopyLabelToGroupDialog(mitk::MultiLabelSegmentation* segmentation, mitk::Label* label, QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitkCopyLabelToGroupDialog),
    m_Segmentation(segmentation),
    m_SourceLabel(label),
    m_DestinationLabel(nullptr)
{
  const auto numberOfGroups = segmentation->GetNumberOfGroups();
  const auto sourceGroup = segmentation->GetGroupIndexOfLabel(m_SourceLabel->GetValue());

  m_Ui->setupUi(this);

  m_Ui->copyLabel->setText(QString("Copy <b>%1</b> from <b>%2</b> to:")
    .arg(GetDisplayedLabelName(segmentation, label))
    .arg(GetDisplayedGroupName(segmentation, sourceGroup)));

  for (unsigned int group = 0; group < numberOfGroups; ++group)
  {
    if (group != sourceGroup)
      m_Ui->groupComboBox->addItem(GetDisplayedGroupName(segmentation, group), group);
  }
}

QmitkCopyLabelToGroupDialog::~QmitkCopyLabelToGroupDialog() = default;

void QmitkCopyLabelToGroupDialog::accept()
{
  const auto destinationGroup = m_Ui->groupComboBox->currentIndex() != 0
    ? m_Ui->groupComboBox->currentData().toUInt()
    : m_Segmentation->AddLayer();

  m_DestinationLabel = m_Segmentation->AddLabel(m_SourceLabel, destinationGroup);

  mitk::TransferLabelContent(
    m_Segmentation->GetGroupImage(m_Segmentation->GetGroupIndexOfLabel(m_SourceLabel->GetValue())),
    m_Segmentation->GetGroupImage(destinationGroup),
    m_Segmentation->GetConstLabelsByValue(m_Segmentation->GetLabelValuesByGroup(destinationGroup)),
    mitk::MultiLabelSegmentation::UNLABELED_VALUE,
    mitk::MultiLabelSegmentation::UNLABELED_VALUE,
    false,
    {{ m_SourceLabel->GetValue(), m_DestinationLabel->GetValue() }},
    mitk::MultiLabelSegmentation::MergeStyle::Replace,
    m_Ui->regardLocksRadioButton->isChecked()
      ? mitk::MultiLabelSegmentation::OverwriteStyle::RegardLocks
      : mitk::MultiLabelSegmentation::OverwriteStyle::IgnoreLocks);

  m_Segmentation->SetActiveLabel(m_DestinationLabel->GetValue());

  QDialog::accept();
}

mitk::Label* QmitkCopyLabelToGroupDialog::GetDestinationLabel() const
{
  return m_DestinationLabel;
}
