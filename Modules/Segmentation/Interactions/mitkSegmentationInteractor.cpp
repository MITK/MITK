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
  auto positionEvent = static_cast<InteractionPositionEvent*>(interactionEvent);

  auto toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager(
    mitk::ToolManagerProvider::MULTILABEL_SEGMENTATION);

  assert(toolManager);

  DataNode *workingNode(toolManager->GetWorkingData(0));
  if (workingNode && positionEvent)
  {
    //TODO T28561
    //Code uses a deprecated method. deactivated whole code until the refactorization is done.
    throw "TODO T28561. Was forgot to refactor in context of T28524. The new MultiLabelSegmentation class will have a dedicated function for querying the label of world position.";

    //auto *workingImage = dynamic_cast<mitk::LabelSetImage *>(workingNode->GetData());
    //assert(workingImage);

    //const auto timestep = positionEvent->GetSender()->GetTimeStep(workingImage);

   //int pixelValue = static_cast<int>(workingImage->GetPixelValueByWorldCoordinate(positionEvent->GetPositionInWorld(), timestep));
    //workingImage->GetActiveLabelSet()->SetActiveLabel(pixelValue); // can be the background

    // // Call Events
    // // workingImage->ActiveLabelEvent.Send(pixelValue);

    // // MLI TODO
    // // toolManager->WorkingDataModified.Send();

    //TODO END Refactor with T28524
  }

  RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}
