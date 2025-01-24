/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkCopyLabelToGroupDialog.h"
#include <ui_QmitkCopyLabelToGroupDialog.h>

#include <mitkLabelSetImage.h>

namespace
{
  QString GetDisplayedLabelName(mitk::LabelSetImage* segmentation, mitk::Label* label)
  {
    const auto group = segmentation->GetGroupIndexOfLabel(label->GetValue());

    QString name = QString::fromStdString(label->GetName());

    if (segmentation->GetLabelValuesByName(group, label->GetName()).size() > 1)
      name += QString(" [%1]").arg(label->GetValue());

    return name;
  }

  QString GetDisplayedGroupName(mitk::LabelSetImage* segmentation, mitk::LabelSetImage::GroupIndexType group)
  {
    auto name = QString::fromStdString(segmentation->GetGroupName(group));

    if (!name.isEmpty())
      return name;

    return QString("Group %1").arg(group + 1);
  }
}

QmitkCopyLabelToGroupDialog::QmitkCopyLabelToGroupDialog(mitk::LabelSetImage* segmentation, mitk::Label* label, QWidget* parent)
  : QDialog(parent),
    m_Ui(new Ui::QmitkCopyLabelToGroupDialog),
    m_Segmentation(segmentation),
    m_SourceLabel(label),
    m_DestinationLabel(nullptr)
{
  const auto numberOfGroups = segmentation->GetNumberOfLayers();
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
    mitk::LabelSetImage::UNLABELED_VALUE,
    mitk::LabelSetImage::UNLABELED_VALUE,
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
