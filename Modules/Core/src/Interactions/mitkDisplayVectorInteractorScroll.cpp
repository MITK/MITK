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


#include "mitkDisplayVectorInteractorScroll.h"
#include "mitkOperation.h"
#include "mitkDisplayCoordinateOperation.h"
//#include "mitkDisplayPositionEvent.h"
#include "mitkUndoController.h"
#include "mitkStateEvent.h"
#include "mitkInteractionConst.h"
#include "mitkAction.h"


void mitk::DisplayVectorInteractorScroll::ExecuteOperation(Operation* itkNotUsed( operation ) )
{
}

bool mitk::DisplayVectorInteractorScroll::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
  bool ok=false;


  const DisplayPositionEvent* posEvent=dynamic_cast<const DisplayPositionEvent*>(stateEvent->GetEvent());

  int actionId = action->GetActionId();

  switch(actionId)
  {
  case AcSELECTALL:   // modifier key for uncoupled panning is being pressed
    {
      m_IsModifierActive = true;
      break;
    }
  case AcSELECT:   // modifier key for uncoupled panning is NOT being pressed
    {
      m_IsModifierActive = false;
      break;
    }
  case AcINITMOVE:
    {
      if(posEvent==NULL) return false;

      m_Sender=posEvent->GetSender();
      m_StartDisplayCoordinate=posEvent->GetDisplayPosition();
      m_LastDisplayCoordinate=posEvent->GetDisplayPosition();
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      ok = true;
      break;
    }
  case AcSCROLLMOUSEWHEEL:
    {
    const WheelEvent* wheelEvent=dynamic_cast<const WheelEvent*>(stateEvent->GetEvent());

    if(wheelEvent != NULL)
    {
      mitk::SliceNavigationController::Pointer sliceNaviController = wheelEvent->GetSender()->GetSliceNavigationController();

      if ( !sliceNaviController->GetSliceLocked() )
      {
        this->InvokeEvent( StartScrollInteractionEvent() );
        mitk::Stepper* stepper = sliceNaviController->GetSlice();

        if (stepper->GetSteps() <= 1)
        {
          stepper = sliceNaviController->GetTime();
        }

        // get the desired delta
        int delta = wheelEvent->GetDelta();
        if ( m_InvertScrollingDirection )
          delta *= -1;  // If we want to invert the scrolling direction -> delta * -1

        if ( delta < 0 )
        {
          stepper->Next();
        }
        else
        {
          stepper->Previous();
        }
      }
    }
      ok = true;
      break;
    }
  case AcSCROLL:
  {
      if(posEvent==NULL) return false;

      mitk::SliceNavigationController::Pointer sliceNaviController = m_Sender->GetSliceNavigationController();

      if(sliceNaviController)
      {
        this->InvokeEvent( StartScrollInteractionEvent() );

        int delta = 0;

        delta = static_cast<int>(m_LastDisplayCoordinate[1]-posEvent->GetDisplayPosition()[1]);

        // if we moved less than 'm_IndexToSliceModifier' pixels slice ONE slice only
        if ( delta>0 && delta<m_IndexToSliceModifier )
        {
          delta=m_IndexToSliceModifier;
        }
        else if(delta<0 && delta>-m_IndexToSliceModifier)
        {
          delta=-m_IndexToSliceModifier;
        }

        delta /= m_IndexToSliceModifier;

        if ( m_InvertScrollingDirection )
          delta *= -1;

        int newPos = sliceNaviController->GetSlice()->GetPos() + delta;

        // if auto repeat is on, start at first slice if you reach the last slice and vice versa
        int maxSlices = sliceNaviController->GetSlice()->GetSteps();
        if ( m_AutoRepeat )
        {
          while(newPos<0)
          {
            newPos += maxSlices;
          }

          while(newPos>=maxSlices)
          {
            newPos -= maxSlices;
          }
        }
        else
        {
          // if the new slice is below 0 we still show slice 0
          // due to the stepper using unsigned int we have to do this ourselves
          if ( newPos < 1 )
            newPos = 0;
        }

        // set the new position
        sliceNaviController->GetSlice()->SetPos( newPos );

      }

      m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();

      ok = true;
      break;
    }
  case AcFINISHMOVE:
    {
      this->InvokeEvent( EndScrollInteractionEvent() );

      ok = true;
      break;
    }
  default:
    ok = false;
    break;
  }
  return ok;
}

void mitk::DisplayVectorInteractorScroll::SetIndexToSliceModifier( int modifier )
{
  m_IndexToSliceModifier = modifier;
}

void mitk::DisplayVectorInteractorScroll::SetAutoRepeat( bool autoRepeat )
{
  m_AutoRepeat = autoRepeat;
}

mitk::DisplayVectorInteractorScroll::DisplayVectorInteractorScroll(const char * type, mitk::OperationActor* destination)
  : mitk::StateMachine(type)
  , m_Sender(NULL)
  , m_Destination(destination)
  , m_IndexToSliceModifier(4)
  , m_AutoRepeat( false )
  , m_InvertScrollingDirection( false )
  , m_IsModifierActive( false )
{
  m_StartDisplayCoordinate.Fill(0);
  m_LastDisplayCoordinate.Fill(0);
  m_CurrentDisplayCoordinate.Fill(0);
  m_UndoEnabled = false;

  //if(m_Destination==NULL)
  //  m_Destination=this;
}


mitk::DisplayVectorInteractorScroll::~DisplayVectorInteractorScroll()
{
  if ( m_Destination != this )
    delete m_Destination;
}

void mitk::DisplayVectorInteractorScroll::SetInvertScrollingDirection( bool invert )
{
  m_InvertScrollingDirection = invert;
}

bool mitk::DisplayVectorInteractorScroll::IsAltModifierActive() const
{
  return m_IsModifierActive;
}

