/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkSeedsInteractor.h"

#include "mitkProperties.h"
#include "mitkStringProperty.h"
#include "mitkAction.h"
#include "mitkDisplayPositionEvent.h"
#include "mitkOperationEvent.h"
#include "mitkInteractionConst.h"
#include "mitkUndoController.h"
#include "mitkDataNode.h"
#include "mitkStateEvent.h"
#include "mitkState.h"
#include "mitkDrawOperation.h"


mitk::SeedsInteractor::SeedsInteractor(const char * type, mitk::DataNode* dataTreeNode)
  : mitk::Interactor(type, dataTreeNode)
{
  m_Radius = 1;
  m_Config = 0; // default config 254, 255 are added for fore- and backgrund seeds
  m_CurrentDrawState = 255;
}

mitk::SeedsInteractor::~SeedsInteractor()
{
}


bool mitk::SeedsInteractor::ExecuteAction(mitk::Action* action, mitk::StateEvent const* stateEvent)
{
  const mitk::DisplayPositionEvent* posEvent = dynamic_cast<const mitk::DisplayPositionEvent*>(stateEvent->GetEvent());
  if (posEvent == NULL)
    return false;
  
  event_point = posEvent->GetWorldPosition();

  m_DataNode->SetVisibility(true);
  m_SeedsImage = dynamic_cast<mitk::SeedsImage*>(m_DataNode->GetData());


  bool ok = false;
  switch (action->GetActionId())
  {
  case mitk::AcINITFOREGROUND:
    {
      if (m_Config == 0)      m_DrawState = m_CurrentDrawState;
      else                    m_DrawState = 1;
      ok = true;
      break;
    }
  case mitk::AcINITBACKGROUND:
    {
      if (m_Config == 0)      m_DrawState = 254;
      else                    m_DrawState = -1;
      ok = true;
      break;
    }
  case mitk::AcINITNEUTRAL:
    {
      m_DrawState = 0;
      ok = true;
      break;
    }
  case mitk::AcADD:
    {
      last_point = event_point;
      mitk::DrawOperation* doOp = new mitk::DrawOperation(OpADD, event_point, last_point, m_DrawState, m_Radius);
      if (m_UndoEnabled){ //write to UndoMechanism/ Can probably be removed!
        mitk::DrawOperation* undoOp = new mitk::DrawOperation(OpUNDOADD, event_point, last_point, m_DrawState, m_Radius);
        mitk::OperationEvent *operationEvent = new mitk::OperationEvent(m_SeedsImage, doOp, undoOp, "Add seed point");
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      m_SeedsImage->ExecuteOperation(doOp);
      ok = true;
      break;
    }
  case mitk::AcMOVE:
    {
      last_point = event_point;
      mitk::DrawOperation* doOp = new mitk::DrawOperation(OpMOVE, event_point, last_point, m_DrawState, m_Radius);
      if (m_UndoEnabled){ //write to UndoMechanism/ Can probably be removed!
        mitk::DrawOperation* undoOp = new mitk::DrawOperation(OpUNDOMOVE, event_point, last_point, m_DrawState, m_Radius);
        mitk::OperationEvent *operationEvent = new mitk::OperationEvent(m_SeedsImage, doOp, undoOp, "Move seed point");
        m_UndoController->SetOperationEvent(operationEvent);
      }
      //execute the Operation
      m_SeedsImage->ExecuteOperation(doOp);   
      ok = true;
      //this->Modified();
      break;
    }
  case mitk::AcFINISH:
    {
      last_point = event_point;
      ok = true;
      this->Modified();
      
      this->InvokeEvent( itk::EndEvent() );

      break;
    }
  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }
  return ok;
}
