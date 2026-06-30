/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "QmitkAssignDistinctColorsAction.h"

#include <mitkColorProperty.h>
#include <mitkLabelSetImageHelper.h>
#include <mitkNodePredicateAnd.h>
#include <mitkNodePredicateDataType.h>
#include <mitkNodePredicateProperty.h>
#include <mitkProperties.h>
#include <mitkRenderingManager.h>

#include <vector>

QmitkAssignDistinctColorsAction::QmitkAssignDistinctColorsAction() = default;

QmitkAssignDistinctColorsAction::~QmitkAssignDistinctColorsAction() = default;

void QmitkAssignDistinctColorsAction::Run(const QList<mitk::DataNode::Pointer>& selectedNodes)
{
  // The framework hands us the entire selection, which may mix data types.
  // Operate only on the binary masks so that the color scheme starts at the
  // first mask and no unrelated node consumes a color slot.
  auto isBinaryMask = mitk::NodePredicateAnd::New(
    mitk::NodePredicateDataType::New("Image"),
    mitk::NodePredicateProperty::New("binary", mitk::BoolProperty::New(true)));

  std::vector<mitk::DataNode::Pointer> binaryMasks;
  for (const auto& node : selectedNodes)
  {
    if (node.IsNotNull() && isBinaryMask->CheckNode(node))
      binaryMasks.push_back(node);
  }

  if (binaryMasks.empty())
    return;

  // Assign colors using the same scheme as segmentation labels: the first mask
  // gets palette index 0, each subsequent mask the color most distinct from the
  // ones already assigned. A mask that already carries its intended color is
  // skipped, so re-running the action (or running it on a single, already
  // correctly colored mask) changes nothing.
  std::vector<mitk::Color> assignedColors;
  bool changed = false;
  for (const auto& mask : binaryMasks)
  {
    const mitk::Color color = mitk::LabelSetImageHelper::SuggestNewLabelColor(assignedColors);
    assignedColors.push_back(color);

    float currentRgb[3];
    if (mask->GetColor(currentRgb) &&
        mitk::MakeColor(currentRgb[0], currentRgb[1], currentRgb[2]) == color)
    {
      continue;
    }

    auto colorProperty = mitk::ColorProperty::New(color);
    mask->SetProperty("color", colorProperty);
    if (mask->GetProperty("binaryimage.selectedcolor"))
      mask->SetProperty("binaryimage.selectedcolor", colorProperty);

    changed = true;
  }

  if (changed)
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkAssignDistinctColorsAction::SetDataStorage(mitk::DataStorage* /*dataStorage*/)
{
  // not needed
}

void QmitkAssignDistinctColorsAction::SetFunctionality(berry::QtViewPart* /*view*/)
{
  // not needed
}
