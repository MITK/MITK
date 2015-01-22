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


#include "mitkMouseMovePointSetInteractor.h"
#include "mitkPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkBaseRenderer.h"

mitk::MouseMovePointSetInteractor
::MouseMovePointSetInteractor(const char * type, DataNode* dataNode, int n)
:PointSetInteractor(type, dataNode, n)
{
}

mitk::MouseMovePointSetInteractor::~MouseMovePointSetInteractor()
{
}

//##Documentation
//## overwritten cause this class can handle mouse move events!
float mitk::MouseMovePointSetInteractor::CanHandleEvent(StateEvent const* stateEvent) const
{
  float returnValue = 0.0;
  //if it is a key event that can be handled in the current state, then return 0.5
  mitk::DisplayPositionEvent const  *disPosEvent =
    dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());

  //Key event handling:
  if (disPosEvent == NULL)
  {
    //check, if the current state has a transition waiting for that key event.
    if (this->GetCurrentState()->GetTransition(stateEvent->GetId())!=NULL)
    {
      return 0.5;
    }
    else
    {
      return 0;
    }
  }

  //get the time of the sender to look for the right transition.
  mitk::BaseRenderer* sender = stateEvent->GetEvent()->GetSender();
  if (sender != NULL)
  {
    unsigned int timeStep = sender->GetTimeStep(m_DataNode->GetData());
    //if the event can be understood and if there is a transition waiting for that event
    mitk::State const* state = this->GetCurrentState(timeStep);
    if (state!= NULL)
      if (state->GetTransition(stateEvent->GetId())!=NULL)
        returnValue = 0.5;//it can be understood
  }
  return returnValue;
}


