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

  auto filename = QFileDialog::getSaveFileName(
    nullptr,
    QStringLiteral("Save Multi Label Preset"),
    QString(),
    QStringLiteral("Multi label preset (*.mitklabel.json)")).toUtf8().toStdString();

  if (filename.empty())
    return;

  const std::string ext = ".mitklabel.json";

  if (filename.size() < ext.size() || filename.compare(filename.size() - ext.size(), ext.size(), ext) != 0)
    filename += ext;

  if (!mitk::MultiLabelIOHelper::SaveMultiLabelSegmentationPreset(filename, segmentation))
  {
    QMessageBox::critical(nullptr, QStringLiteral("Save Multi Label Preset"),
      QString("Could not save \"%1\" as label preset.").arg(QString::fromStdString(filename)));
  }
}

void QmitkLoadMultiLabelPreset(const std::vector<mitk::MultiLabelSegmentation::Pointer>& segmentations)
{
#if defined(_WIN32)
  const auto filter = QStringLiteral("Multi label preset (*.mitklabel.json;*.json;*.lsetp)");
#else
  const auto filter = "All JSON files (*.json);;Multi label preset (*.mitklabel.json);;Legacy label set preset (*.lsetp)";
#endif

  const auto filename = QFileDialog::getOpenFileName(
    nullptr,
    QStringLiteral("Load Multi Label Preset"),
    QString(),
    filter).toUtf8().toStdString();

  if (filename.empty())
    return;

  std::vector<std::unique_ptr<mitk::SegGroupModifyUndoRedoHelper>> undoHelpers;

  for (const auto& segmentation : segmentations)
  {
    if (segmentation.IsNull())
      continue;

    //create a set with all group IDs as all groups should be captured.
    mitk::SegGroupModifyUndoRedoHelper::GroupIndexSetType groupIDs;
    const auto numGroups = segmentation->GetNumberOfGroups();
    for (unsigned int i = 0; i < numGroups; ++i)
      groupIDs.insert(i);

    undoHelpers.push_back(std::make_unique<mitk::SegGroupModifyUndoRedoHelper>(segmentation, groupIDs, true));

    mitk::MultiLabelIOHelper::LoadMultiLabelSegmentationPreset(filename, segmentation);
  }

  for (auto& undoHelper : undoHelpers)
  {
    undoHelper->RegisterUndoRedoOperationEvent("Apply label preset on segmentation");
  }
}
