/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSaveMultiLabelPresetAction.h"

#include <mitkLabelSetImage.h>
#include <QmitkMultiLabelPresetHelper.h>

void QmitkSaveMultiLabelPresetAction::Run(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  for (const auto &node : selectedNodes)
  {
    if (node.IsNull())
      continue;

    mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

    if (image.IsNull())
      continue;

    QmitkSaveMultiLabelPreset(image);
  }
}

void QmitkSaveMultiLabelPresetAction::SetDataStorage(mitk::DataStorage*)
{
}

void QmitkSaveMultiLabelPresetAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkSaveMultiLabelPresetAction::SetSmoothed(bool)
{
}

void QmitkSaveMultiLabelPresetAction::SetDecimated(bool)
{
}
