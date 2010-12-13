/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-01-14 14:20:26 +0100 (Thu, 14 Jan 2010) $
Version:   $Revision: 21047 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


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
  if(posEvent==NULL) return false;

  int actionId = action->GetActionId();
  
  switch(actionId)
  {
  case AcINITMOVE:
    {
      m_Sender=posEvent->GetSender();
      m_StartDisplayCoordinate=posEvent->GetDisplayPosition();
      m_LastDisplayCoordinate=posEvent->GetDisplayPosition();
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();
      ok = true;
      break;
    }
  case AcSCROLL:
    {
      mitk::SliceNavigationController::Pointer sliceNaviController = m_Sender->GetSliceNavigationController();

      if(sliceNaviController)
      {
        int delta = m_LastDisplayCoordinate[1]-posEvent->GetDisplayPosition()[1];

        if(delta>1)
        {
          delta=-1;
        }
        else if(delta<-1)
        {
          delta=1;
        }

        int newPos = sliceNaviController->GetSlice()->GetPos() + delta; 

        int maxSlices = sliceNaviController->GetSlice()->GetSteps();

        while(newPos<0)
        {
          newPos += maxSlices;
        }

        while(newPos>=maxSlices)
        {
          newPos -= maxSlices;
        }

        sliceNaviController->GetSlice()->SetPos( newPos );
      }

      m_LastDisplayCoordinate=m_CurrentDisplayCoordinate;
      m_CurrentDisplayCoordinate=posEvent->GetDisplayPosition();   
    }
  case AcFINISHMOVE:
    {
      ok = true;
      break;
    }
  default:
    ok = false;
    break;
  }
  return ok;
}

mitk::DisplayVectorInteractorScroll::DisplayVectorInteractorScroll(const char * type, mitk::OperationActor* destination)
  : mitk::StateMachine(type), m_Sender(NULL), m_Destination(destination)
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

