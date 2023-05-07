/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <QmitkMultiLabelPresetHelper.h>

#include <mitkMultiLabelIOHelper.h>

#include <QFileDialog>
#include <QMessageBox>

void QmitkSaveMultiLabelPreset(const mitk::LabelSetImage* segmentation)
{
  if (nullptr == segmentation)
    mitkThrow() << "Invalid call of QmitkSaveMultiLabelPreset. Passed image is a null pointer.";

  const auto filename = QFileDialog::getSaveFileName(nullptr, QStringLiteral("Save Label Set Preset"),
    QString(), QStringLiteral("Label set preset (*.lsetp)")).toStdString();

  if (filename.empty())
    return;

  if (!mitk::MultiLabelIOHelper::SaveLabelSetImagePreset(filename, segmentation))
  {
    QMessageBox::critical(nullptr, QStringLiteral("Save Label Set Preset"),
      QString("Could not save \"%1\" as label set preset.").arg(QString::fromStdString(filename)));
  }
}

void QmitkLoadMultiLabelPreset(const std::vector<mitk::LabelSetImage::Pointer>& segmentations)
{
  const auto filename = QFileDialog::getOpenFileName(nullptr, QStringLiteral("Load Label Set Preset"),
    QString(), QStringLiteral("Label set preset (*.lsetp)")).toStdString();

  if (filename.empty())
    return;

  for (const auto& segmentation : segmentations)
  {
    if (segmentation.IsNull())
      continue;

    mitk::MultiLabelIOHelper::LoadLabelSetImagePreset(filename, segmentation);
  }
}
