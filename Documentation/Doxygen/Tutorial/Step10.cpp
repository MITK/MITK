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

#include "Step10.h"
#include "mitkMouseMoveEvent.h"
#include <mitkPointOperation.h>
#include "mitkRenderingManager.h"
#include "mitkInternalEvent.h"
//
#include "mitkDispatcher.h"
#include "mitkBaseRenderer.h"
#include <mitkPropertyList.h>

void mitk::ExampleInteractor::ConnectActionsAndFunctions()
{
  // connect the action names of the state machine pattern with function within
  // this DataInteractor
  CONNECT_FUNCTION("addPoint", AddPoint);
  CONNECT_FUNCTION("enoughPoints", EnoughPoints);
}

bool mitk::ExampleInteractor::AddPoint(StateMachineAction*, InteractionEvent* interactionEvent)
{
  // cast InteractionEvent to a position event in order to read out the mouse position
  // we stay here as general as possible so that a different state machine pattern
  // can reuse this code with MouseRelease or MouseMoveEvents.
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    // query the position of the mouse in the world geometry
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    m_PointSet->InsertPoint(m_NumberOfPoints, point, 0);
    m_NumberOfPoints++;
    GetDataNode()->SetData(m_PointSet);
    GetDataNode()->Modified();

    // read how many points we accept from the config properties
    mitk::PropertyList::Pointer properties = GetPropertyList();
    std::string maxNumber;
    properties->GetStringProperty("NumberOfPoints",maxNumber);
    if (m_NumberOfPoints >=  atoi(maxNumber.c_str()))
    {
      // create internal event that signal that the maximal number of points is reached
      InternalEvent::Pointer event = InternalEvent::New(NULL,this, "enoughPointsAdded");
      // add the internal event to the event queue of the Dispatcher
      positionEvent->GetSender()->GetDispatcher()->QueueEvent(event.GetPointer());
    }
    // update the RenderWindow to show new points
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
    return true;
  }
  else
  {
    return false;
  }
}

mitk::ExampleInteractor::ExampleInteractor()
{
  m_NumberOfPoints = 0;
  m_PointSet = PointSet::New(); // create PointSet to store our point in
}

mitk::ExampleInteractor::~ExampleInteractor()
{
}

bool mitk::ExampleInteractor::EnoughPoints(StateMachineAction*, InteractionEvent*)
{
  GetDataNode()->SetProperty("contourcolor", ColorProperty::New(1.0, 1.0, 0.0));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
}
