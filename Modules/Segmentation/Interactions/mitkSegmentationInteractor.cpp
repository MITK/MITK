/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkSegmentationInteractor.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkLabelSetImage.h"
#include "mitkToolManager.h"
#include "mitkToolManagerProvider.h"
#include <mitkImagePixelReadAccessor.h>

#include <cstring>

void mitk::SegmentationInteractor::ConnectActionsAndFunctions()
{
  Superclass::ConnectActionsAndFunctions();

  // CONNECT_FUNCTION("change_active_label", ChangeActiveLabel);
}

bool mitk::SegmentationInteractor::ChangeActiveLabel(StateMachineAction *, InteractionEvent *interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  auto *positionEvent = static_cast<InteractionPositionEvent *>(interactionEvent);

  // MLI TODO
  // m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  // m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();

  auto *toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(
    mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION);

  assert(toolManager);

  DataNode *workingNode(toolManager->GetWorkingData(0));
  if (workingNode)
  {
    auto *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
    assert(workingImage);

    const auto timestep = positionEvent->GetSender()->GetTimeStep(workingImage);
    int pixelValue = static_cast<int>(workingImage->GetPixelValueByWorldCoordinate(positionEvent->GetPositionInWorld(), timestep));

    workingImage->GetActiveLabelSet()->SetActiveLabel(pixelValue); // can be the background

    // Call Events
    // workingImage->ActiveLabelEvent.Send(pixelValue);

    // MLI TODO
    // toolManager->WorkingDataModified.Send();
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}
