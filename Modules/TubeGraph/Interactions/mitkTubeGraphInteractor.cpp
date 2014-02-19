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

#include "mitkTubeGraphInteractor.h"

#include "mitkDisplayPositionEvent.h"
#include "mitkInteractionConst.h"
#include "mitkOperationEvent.h"
#include "mitkRenderingManager.h"
#include "mitkStateEvent.h"
#include "mitkStatusBar.h"
#include "mitkTubeGraphPicker.h"

#include "mitkUndoController.h"

mitk::DataNode* mitk::TubeGraphInteractor::m_CurrentSelectedDataNode = NULL;

mitk::TubeGraphInteractor::TubeGraphInteractor( const char * type, DataNode* dataNode )
: Interactor( type, dataNode )
, m_LastPickedTube(TubeGraph::ErrorId)
, m_SecondLastPickedTube(TubeGraph::ErrorId)
, m_ActionMode(AttributationMode)
/*, m_PermissionBarrier(false)*/
{
}

mitk::TubeGraphInteractor::~TubeGraphInteractor()
{
}

float mitk::TubeGraphInteractor::CanHandleEvent( const StateEvent* ) const
{
  if ( (m_CurrentSelectedDataNode != NULL) && ( m_DataNode != NULL) )
  {
    if( (m_TubeGraph.IsNotNull() ) && ( m_DataNode->GetData() != NULL) )
    {
      if (m_TubeGraph == m_DataNode->GetData())
        return 1.0f; //Can handle event

      else
        return 0.0f;//Could not handle event
    }
  }
  else
    return 0.0f;
}

void mitk::TubeGraphInteractor::SetCurrentSelectedDataNode( DataNode* node )
{
  m_CurrentSelectedDataNode = node;
}

bool mitk::TubeGraphInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  if (!m_DataNode)
    return false;

  m_TubeGraph = dynamic_cast< TubeGraph* >(m_CurrentSelectedDataNode->GetData());

  if (!m_TubeGraph)
    return false;


  m_TubeGraphProperty = dynamic_cast< TubeGraphProperty* >(m_TubeGraph->GetProperty( "Tube Graph.Visualization Information" ).GetPointer());
  if (m_TubeGraphProperty.IsNull())
  {
    MITK_ERROR<< "Something went wrong! No tube graph property!";
    return false;
  }

  bool isExecute = false;

  switch ( action->GetActionId() )
  {
  case AcDONOTHING:
    isExecute = true;
    break;

  case AcCHECKELEMENT:
    {
      // Only WorldPosition implemented; for 2D see mitkVesselGraphInteractor
      DisplayPositionEvent const * dispPosEvent = dynamic_cast < const DisplayPositionEvent* >(stateEvent->GetEvent());
      if ( dispPosEvent != NULL )
      {
        Point3D point = dispPosEvent->GetWorldPosition();
        TubeGraphPicker* picker = new mitk::TubeGraphPicker();

        picker->SetTubeGraph(m_TubeGraph);
        TubeGraph::TubeDescriptorType tubeDescriptor = picker->GetPickedTube(point).first;


        if(tubeDescriptor != TubeGraph::ErrorId )
        {
          m_SecondLastPickedTube = m_LastPickedTube;
          this->m_LastPickedTube = tubeDescriptor;

          /* TubeGraphOperation* doOp = new mitk::TubeGraphOperation( OpSELECTSUBOBJECT );
          if ( m_UndoEnabled )
          {
          TubeGraphOperation* undoOp = new mitk::TubeGraphOperation( OpDESELECTSUBOBJECT );
          OperationEvent* operationEvent = new OperationEvent( tubeGraph, doOp, undoOp );
          m_UndoController->SetOperationEvent( operationEvent );
          }*/
          //Send answer (EventID_Yes) for GuardState in StateMachine.xml
          mitk::StateEvent* newStateEvent = new mitk::StateEvent( EIDYES, stateEvent->GetEvent() );
          this->HandleEvent(newStateEvent);
        }
        else //nothing was picked
        {
          //  TubeGraphOperation* doOp = new mitk::TubeGraphOperation ( OpSELECTPOINT );
          //  if( m_UndoEnabled )
          //  {
          //    TubeGraphOperation* undoOp = new mitk::TubeGraphOperation( OpDESELECTPOINT );
          //    OperationEvent* operationEvent = new OperationEvent( tubeGraph, doOp, undoOp );
          //    m_UndoController->SetOperationEvent( operationEvent );
          //  }
          //Send answer (EventID_No) for GuardState in StateMachine.xml
          mitk::StateEvent* newStateEvent = new mitk::StateEvent( EIDNO, stateEvent->GetEvent() );
          this->HandleEvent( newStateEvent);
        }
      }
      else
      {
        //Send answer (EventID_No) for GuardState in StateMachine.xml
        mitk::StateEvent* newStateEvent = new mitk::StateEvent( EIDNO, stateEvent->GetEvent() );
        this->HandleEvent( newStateEvent );
      }
    }
    isExecute = true;
    break;

  case AcSELECT:
    {
      this->SelectTubesByActivationModus();

      RenderingManager::GetInstance()->RequestUpdateAll();

      if (m_ActivationMode != None)
      {
        // show vessel id on status bar
        std::stringstream displayText;
        displayText << "Picked tube: ID [" << m_LastPickedTube.first << "," << m_LastPickedTube.second << "]";
        StatusBar::GetInstance()->DisplayText( displayText.str().c_str() );
        this->InvokeEvent(SelectionChangedTubeGraphEvent());
      }
    }
    isExecute = true;
    break;

  case AcDESELECT:
    {
      if ((m_ActivationMode != Multiple)
        && (m_ActivationMode != Points))
      {
        m_TubeGraphProperty->DeactivateAllTubes();
        RenderingManager::GetInstance()->RequestUpdateAll();
        this->InvokeEvent(SelectionChangedTubeGraphEvent());
      }
      // show info on status bar
      StatusBar::GetInstance()->DisplayText("No tube hit!");
    }
    isExecute = true;
    break;

  case AcSELECTANOTHEROBJECT:
    {
      this->SelectTubesByActivationModus();
      RenderingManager::GetInstance()->RequestUpdateAll();
      // show vessel id on status bar
      std::stringstream displayText;
      displayText << "Picked tube: ID [" << m_LastPickedTube.first << "," << m_LastPickedTube.second << "]";
      StatusBar::GetInstance()->DisplayText( displayText.str().c_str() );

      this->InvokeEvent(SelectionChangedTubeGraphEvent());
    }

    isExecute = true;
    break;

  case AcDEFAULT:
    {
      //TODOOOOOOO
    }
    isExecute = true;
    break;

  default:
    {
      isExecute = false;
      break;
    }
  }
  return isExecute;
}

void mitk::TubeGraphInteractor::SetActivationMode(const ActivationMode& activationMode)
{
  m_ActivationMode = activationMode;
  if (m_TubeGraph.IsNotNull())
    if (m_LastPickedTube != mitk::TubeGraph::ErrorId)
      this->UpdateActivation();
}

mitk::TubeGraphInteractor::ActivationMode mitk::TubeGraphInteractor::GetActivationMode()
{
  return m_ActivationMode;
}

void mitk::TubeGraphInteractor::SetActionMode(const ActionMode& actionMode)
{
  m_ActionMode = actionMode;
}

mitk::TubeGraphInteractor::ActionMode mitk::TubeGraphInteractor::GetActionMode()
{
  return m_ActionMode;
}

void mitk::TubeGraphInteractor::SelectTubesByActivationModus()
{
  if (m_LastPickedTube != mitk::TubeGraph::ErrorId)
  {
    this->UpdateActivation();
  }
}

void mitk::TubeGraphInteractor::UpdateActivation()
{
  if (m_ActionMode == RootMode)
  {
    m_TubeGraphProperty->DeactivateAllTubes();
    m_TubeGraphProperty->SetTubeActive(m_LastPickedTube, true);

    //QmitkTubeGraphSelectRootDialog* dialog = new QmitkTubeGraphSelectRootDialog(m_Parent);
    //int dialogReturnValue = dialog->exec();
    //delete dialog;

    //if ( dialogReturnValue != QDialog::Rejected ) // user doesn't clicked cancel or pressed Esc or something similar
    //{
    m_TubeGraph->SetRootTube(m_LastPickedTube);
    //}
    m_TubeGraphProperty->DeactivateAllTubes();
    RenderingManager::GetInstance()->RequestUpdateAll();
  }
  else
  {
    switch (m_ActivationMode)
    {
    case None :
      {
        m_TubeGraphProperty->DeactivateAllTubes();
      }
      break;

    case Single:
      {
        m_TubeGraphProperty->DeactivateAllTubes();
        m_TubeGraphProperty->SetTubeActive(m_LastPickedTube, true);
      }
      break;

    case Multiple:
      { //special deactivation for multiple modus
        //if activated--> deactivate; if not activated--> activate
        if(m_TubeGraphProperty->IsTubeActive(m_LastPickedTube))
          m_TubeGraphProperty->SetTubeActive(m_LastPickedTube, false);
        else
          m_TubeGraphProperty->SetTubeActive(m_LastPickedTube, true);
      }
      break;

    case ToRoot:
      {
        m_TubeGraphProperty->DeactivateAllTubes();
        std::vector<TubeGraph::TubeDescriptorType> activeTubes = this->GetTubesToRoot();
        m_TubeGraphProperty->SetTubesActive(activeTubes);
      }
      break;
    case ToPeriphery:
      {
        m_TubeGraphProperty->DeactivateAllTubes();
        std::vector<TubeGraph::TubeDescriptorType> activeTubes = this->GetPathToPeriphery();
        m_TubeGraphProperty->SetTubesActive(activeTubes);
      }
      break;

    case Points:
      {
        m_TubeGraphProperty->DeactivateAllTubes();
        std::vector<TubeGraph::TubeDescriptorType> activeTubes = this->GetTubesBetweenPoints();
        m_TubeGraphProperty->SetTubesActive(activeTubes);
      }
      break;

    default:
      MITK_WARN<<"the impossible happened!";
      break;
    }
  }
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphInteractor::GetTubesToRoot()
{
  TubeGraph::TubeDescriptorType root = m_TubeGraph->GetRootTube();
  if(root == TubeGraph::ErrorId)
  {
    root = m_TubeGraph->GetThickestTube();
    m_TubeGraph->SetRootTube(root);
  }

  return this->GetPathBetweenTubes(m_LastPickedTube, root);
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphInteractor::GetTubesBetweenPoints()
{
  return this->GetPathBetweenTubes(m_LastPickedTube, m_SecondLastPickedTube);
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphInteractor::GetPathBetweenTubes(const mitk::TubeGraph::TubeDescriptorType& start,const mitk::TubeGraph::TubeDescriptorType& end)
{
  std::vector<mitk::TubeGraph::TubeDescriptorType> solutionPath;
  if ((start != TubeGraph::ErrorId) && (end != TubeGraph::ErrorId))
  {
    if (start != end)
      solutionPath = m_TubeGraph->SearchAllPathBetweenVertices(start, end);
    else
      solutionPath.push_back(start);
  }

  return solutionPath;
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphInteractor::GetPathToPeriphery()
{
  std::vector<mitk::TubeGraph::TubeDescriptorType> solutionPath;

  if (m_LastPickedTube != TubeGraph::ErrorId)
    solutionPath = m_TubeGraph->SearchPathToPeriphery(m_LastPickedTube);

  return solutionPath;
}

void mitk::TubeGraphInteractor::ResetPickedTubes()
{
  m_LastPickedTube = TubeGraph::ErrorId;
  m_SecondLastPickedTube = TubeGraph::ErrorId;
}
