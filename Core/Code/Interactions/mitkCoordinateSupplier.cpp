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


#include "mitkDisplayCoordinateOperation.h"
//has to be on top, otherwise compiler error!
#include "mitkCoordinateSupplier.h"
#include "mitkOperation.h"
#include "mitkOperationActor.h"
#include "mitkPointOperation.h"
#include "mitkPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkUndoController.h"
//and not here!
#include <string>
#include "mitkInteractionConst.h"
#include "mitkAction.h"


mitk::CoordinateSupplier::CoordinateSupplier(const char * type, mitk::OperationActor* operationActor)
: mitk::StateMachine(type), m_Destination(operationActor)
{
  m_CurrentPoint.Fill(0);
}

mitk::CoordinateSupplier::~CoordinateSupplier()
{
}

bool mitk::CoordinateSupplier::ExecuteAction(Action* action, mitk::StateEvent const* stateEvent)
{
    bool ok = false;

    const PositionEvent* posEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());

    PointOperation* doOp=NULL;
    if(posEvent!=NULL)
    {
      ScalarType timeInMS = 0;
      if(stateEvent->GetEvent()->GetSender()!=NULL)
      {
        timeInMS = stateEvent->GetEvent()->GetSender()->GetTime();
      }
      else
      {
        itkWarningMacro(<<"StateEvent::GetSender()==NULL - setting timeInMS to 0");
      }

      switch (action->GetActionId())
      {
        case AcNEWPOINT:
        {
          if (m_Destination == NULL)
            return false;
          m_OldPoint = posEvent->GetWorldPosition();

          doOp = new mitk::PointOperation(OpADD, timeInMS, m_OldPoint, 0);
          //Undo
          if (m_UndoEnabled)
          {
            PointOperation* undoOp = new PointOperation(OpDELETE, m_OldPoint, 0);
            OperationEvent *operationEvent = new OperationEvent( m_Destination, doOp, undoOp );
            m_UndoController->SetOperationEvent(operationEvent);
          }
          //execute the Operation
          m_Destination->ExecuteOperation(doOp);

          if (!m_UndoEnabled)
            delete doOp;

          ok = true;
          break;
        }
        case AcINITMOVEMENT:
        {
          if (m_Destination == NULL)
            return false;
          //move the point to the coordinate //not used, cause same to MovePoint... check xml-file
          mitk::Point3D movePoint = posEvent->GetWorldPosition();

          mitk::PointOperation doPointOp(OpMOVE, timeInMS, movePoint, 0);
          //execute the Operation
          m_Destination->ExecuteOperation(&doPointOp);
          ok = true;
          break;
        }
        case AcMOVEPOINT:
        case AcMOVE:
        {
          mitk::Point3D movePoint = posEvent->GetWorldPosition();
          m_CurrentPoint = movePoint;
          if (m_Destination == NULL)
            return false;
          mitk::PointOperation doPointOp(OpMOVE, timeInMS, movePoint, 0);
          //execute the Operation
          m_Destination->ExecuteOperation(&doPointOp);
          ok = true;
          break;
        }
        case AcFINISHMOVEMENT:
        {
          if (m_Destination == NULL)
            return false;
          /*finishes a Movement from the coordinate supplier:
          gets the lastpoint from the undolist and writes an undo-operation so
          that the movement of the coordinatesupplier is undoable.*/
          mitk::Point3D movePoint = posEvent->GetWorldPosition();
          mitk::Point3D oldMovePoint; oldMovePoint.Fill(0);

          doOp = new mitk::PointOperation(OpMOVE, timeInMS, movePoint, 0);
          PointOperation finishOp(OpTERMINATE, movePoint, 0);
          if (m_UndoEnabled )
          {
            //get the last Position from the UndoList
            OperationEvent *lastOperationEvent = m_UndoController->GetLastOfType(m_Destination, OpMOVE);
            if (lastOperationEvent != NULL)
            {
              PointOperation* lastOp = dynamic_cast<PointOperation *>(lastOperationEvent->GetOperation());
              if (lastOp != NULL)
              {
                oldMovePoint = lastOp->GetPoint();
              }
            }
            PointOperation* undoOp = new PointOperation(OpMOVE, timeInMS, oldMovePoint, 0, "Move slices");
            OperationEvent *operationEvent = new OperationEvent(m_Destination, doOp, undoOp, "Move slices");
            m_UndoController->SetOperationEvent(operationEvent);
          }
          //execute the Operation
          m_Destination->ExecuteOperation(doOp);

          if (!m_UndoEnabled)
            delete doOp;

          m_Destination->ExecuteOperation(&finishOp);
          ok = true;

          break;
        }
        default:
          ok = false;
          break;
      }
      return ok;
    }

    const mitk::DisplayPositionEvent* displPosEvent = dynamic_cast<const mitk::DisplayPositionEvent *>(stateEvent->GetEvent());
    if(displPosEvent!=NULL)
    {
        return true;
    }

  return false;
}
