/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelPresetHelper.h>

#include <mitkMultiLabelIOHelper.h>

#include <mitkSegChangeOperationApplier.h>

#include <QFileDialog>
#include <QMessageBox>

void QmitkSaveMultiLabelPreset(const mitk::MultiLabelSegmentation* segmentation)
{
  if (nullptr == segmentation)
    mitkThrow() << "Invalid call of QmitkSaveMultiLabelPreset. Passed image is a null pointer.";

  const auto filename = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Save Multi Label Preset"),
    QString(), QStringLiteral("Multi label preset (*.mitklabel.json)")).toStdString();

  if (filename.empty())
    return;

  if (!mitk::MultiLabelIOHelper::SaveMultiLabelSegmentationPreset(filename, segmentation))
  {
    QMessageBox::critical(nullptr, QStringLiteral("Save Multi Label Preset"),
      QString("Could not save \"%1\" as label preset.").arg(QString::fromStdString(filename)));
  }
}

void QmitkLoadMultiLabelPreset(const std::vector<mitk::MultiLabelSegmentation::Pointer>& segmentations)
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Load Multi Label Preset"),
    QString(), QStringLiteral("Multi label preset (*.mitklabel.json);;Legacy label set preset (*.lsetp)")).toStdString();

  if (filename.empty())
    return;

  std::vector<std::unique_ptr<mitk::SegGroupModifyUndoRedoHelper>> undoHelpers;

  for (const auto& segmentation : segmentations)
  {
    if (segmentation.IsNull())
      continue;

    //create a set with all group IDs as all groups should be captured.
    std::vector<mitk::MultiLabelSegmentation::GroupIndexType> temp(segmentation->GetNumberOfGroups());
    std::iota(temp.begin(), temp.end(), 0);
    std::set<int> s(temp.begin(), temp.end());
    mitk::SegGroupModifyUndoRedoHelper::GroupIndexSetType groupIDs(temp.begin(), temp.end());

    undoHelpers.push_back(std::make_unique<mitk::SegGroupModifyUndoRedoHelper>(segmentation, groupIDs, true));

    mitk::MultiLabelIOHelper::LoadMultiLabelSegementationPreset(filename, segmentation);
  }

  for (auto& undoHelper : undoHelpers)
  {
    undoHelper->RegisterUndoRedoOperationEvent("Apply label preset on segmentation");
  }
}
