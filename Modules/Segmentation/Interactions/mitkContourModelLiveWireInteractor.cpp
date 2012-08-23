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

#include "mitkContourModelLiveWireInteractor.h"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include <mitkPositionEvent.h>
#include <mitkStateEvent.h>

#include <mitkInteractionConst.h>


mitk::ContourModelLiveWireInteractor::ContourModelLiveWireInteractor(DataNode* dataNode)
:ContourModelInteractor(dataNode)
{
}


mitk::ContourModelLiveWireInteractor::~ContourModelLiveWireInteractor()
{
}




bool mitk::ContourModelLiveWireInteractor::OnDeletePoint( Action* action, const StateEvent* stateEvent)
{

  int timestep = stateEvent->GetEvent()->GetSender()->GetTimeStep();

  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

  contour->RemoveVertex(contour->GetSelectedVertex());


  return true;
}




bool mitk::ContourModelLiveWireInteractor::OnMovePoint( Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;


  mitk::ContourModel *contour = dynamic_cast<mitk::ContourModel *>( m_DataNode->GetData() );

  mitk::Vector3D translation;
  mitk::Point3D currentPosition = positionEvent->GetWorldPosition();
  translation[0] = currentPosition[0] - this->m_lastMousePosition[0];
  translation[1] = currentPosition[1] - this->m_lastMousePosition[1];
  translation[2] = currentPosition[2] - this->m_lastMousePosition[2];
  contour->ShiftSelectedVertex(translation);

  this->m_lastMousePosition = positionEvent->GetWorldPosition();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
