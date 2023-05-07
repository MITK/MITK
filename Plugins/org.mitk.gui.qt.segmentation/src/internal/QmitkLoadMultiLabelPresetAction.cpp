/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkLoadMultiLabelPresetAction.h"

#include <mitkLabelSetImage.h>
#include <QmitkMultiLabelPresetHelper.h>


void QmitkLoadMultiLabelPresetAction::Run(const QList<mitk::DataNode::Pointer> &selectedNodes)
{
  std::vector<mitk::LabelSetImage::Pointer> images;
  for (const auto &node : selectedNodes)
  {
    if (node.IsNull())
      continue;

    mitk::LabelSetImage::Pointer image = dynamic_cast<mitk::LabelSetImage*>(node->GetData());

    if (image.IsNull())
      continue;

    images.emplace_back(image);
  }
  QmitkLoadMultiLabelPreset(images);
}

void QmitkLoadMultiLabelPresetAction::SetDataStorage(mitk::DataStorage*)
{
}

void QmitkLoadMultiLabelPresetAction::SetFunctionality(berry::QtViewPart*)
{
}

void QmitkLoadMultiLabelPresetAction::SetSmoothed(bool)
{
}

void QmitkLoadMultiLabelPresetAction::SetDecimated(bool)
{
}
