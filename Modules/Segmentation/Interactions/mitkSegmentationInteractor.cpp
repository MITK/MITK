/*===================================================================

 The Medical Imaging Interaction Toolkit (MITK)

 Copyright (c) German Cancer Research Center,
 Division of Medical and Biological Informatics.
 All rights reserved.

 This software is distributed WITHOUT ANY WARRANTY; without
 even the implied warranty of MERCHANTABILITY or FITNESS FOR
 A PARTICULAR PURPOSE.

 See LICENSE.txt or http://www.mitk.org for details.

 ===================================================================*/

#include "mitkSegmentationInteractor.h"
#include "mitkLabelSetImage.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkToolManager.h"
#include "mitkToolManagerProvider.h"

#include <string.h>

void mitk::SegmentationInteractor::ConnectActionsAndFunctions()
{
  Superclass::ConnectActionsAndFunctions();

  //CONNECT_FUNCTION("change_active_label", ChangeActiveLabel);
}

bool mitk::SegmentationInteractor::ChangeActiveLabel(StateMachineAction*, InteractionEvent* interactionEvent)
{
  BaseRenderer::Pointer sender = interactionEvent->GetSender();
  InteractionPositionEvent* positionEvent = static_cast<InteractionPositionEvent*>(interactionEvent);

  //MLI TODO
  //m_LastDisplayCoordinate = m_CurrentDisplayCoordinate;
  //m_CurrentDisplayCoordinate = positionEvent->GetPointerPositionOnScreen();

  mitk::ToolManager* toolManager = mitk::ToolManagerProvider::GetInstance()->GetToolManager();
  assert(toolManager);

  DataNode* workingNode( toolManager->GetWorkingData(0) );
  if (workingNode)
  {
    mitk::LabelSetImage* workingImage = dynamic_cast<mitk::LabelSetImage*>(workingNode->GetData());
    assert(workingImage);

    int timestep = positionEvent->GetSender()->GetTimeStep();
    int pixelValue = workingImage->GetPixelValueByWorldCoordinate( positionEvent->GetPositionInWorld(), timestep );
    workingImage->GetActiveLabelSet()->SetActiveLabel(pixelValue); // can be the background

    // Call Events
    //workingImage->ActiveLabelEvent.Send(pixelValue);

    //MLI TODO
    //toolManager->WorkingDataModified.Send();
  }

  sender->GetRenderingManager()->RequestUpdateAll();
  return true;
}
