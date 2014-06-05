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
  // connect the action and condition names of the state machine pattern with function within
  // this DataInteractor
  CONNECT_CONDITION("checkPoint", CheckPoint);
  CONNECT_FUNCTION("addPoint", AddPoint);
  CONNECT_FUNCTION("enoughPoints", EnoughPoints);
}

bool mitk::ExampleInteractor::CheckPoint(const InteractionEvent *interactionEvent)
{
  // check if a point close to the clicked position already exists
  float epsilon = 0.3; // do not accept new points within 3mm range of existing points
  InteractionPositionEvent* positionEvent = dynamic_cast<InteractionPositionEvent*>(interactionEvent);
  if (positionEvent != NULL)
  {
    // query the position of the mouse in the world geometry
    mitk::Point3D point = positionEvent->GetPositionInWorld();
    int retVal = m_PointSet->SearchPoint(point, epsilon);
    if ( retVal == -1 ) // SearchPoint returns -1 if no point was found within given range
      return true;
  }
  return false; // if the positionEvent is NULL or a point was found return false. AddPoint will not be executed
  //end
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

    if (m_NumberOfPoints != 0 && m_NumberOfPoints >= m_MaximalNumberOfPoints)
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

bool mitk::ExampleInteractor::EnoughPoints(StateMachineAction*, InteractionEvent*)
{
  GetDataNode()->SetProperty("contourcolor", ColorProperty::New(1.0, 1.0, 0.0));
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  return true;
} //-

mitk::ExampleInteractor::ExampleInteractor()
{
  m_NumberOfPoints = 0;
  m_MaximalNumberOfPoints = 0;
  m_PointSet = PointSet::New(); // create PointSet to store our point in
}

mitk::ExampleInteractor::~ExampleInteractor()
{
}

void mitk::ExampleInteractor::ConfigurationChanged()
{
  // read how many points we accept from the config properties
  mitk::PropertyList::Pointer properties = GetPropertyList();
  std::string maxNumber;
  properties->GetStringProperty("NumberOfPoints",maxNumber);
  m_MaximalNumberOfPoints = atoi(maxNumber.c_str());
}
