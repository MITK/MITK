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

#include "mitkTubeGraphDataInteractor.h"

#include <mitkInteractionConst.h>
#include <mitkInteractionPositionEvent.h>
#include <mitkStatusBar.h>

#include "mitkTubeGraphPicker.h"

#include <vtkCamera.h>
#include <vtkInteractorStyle.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderWindowInteractor.h>

mitk::TubeGraphDataInteractor::TubeGraphDataInteractor()
  : m_LastPickedTube(TubeGraph::ErrorId)
  , m_SecondLastPickedTube(TubeGraph::ErrorId)
  , m_ActivationMode(None)
  , m_ActionMode(AttributationMode)
{
}

mitk::TubeGraphDataInteractor::~TubeGraphDataInteractor()
{
}

void mitk::TubeGraphDataInteractor::ConnectActionsAndFunctions()
{
  // **Conditions** that can be used in the state machine, to ensure that certain conditions are met, before actually executing an action
  CONNECT_CONDITION("isOverTube", CheckOverTube);

  // **Function** in the statmachine patterns also referred to as **Actions**
  CONNECT_FUNCTION("selectTube",SelectTube);
  CONNECT_FUNCTION("deselectTube",DeselectTube);
}

void mitk::TubeGraphDataInteractor::DataNodeChanged()
{
  if (GetDataNode().IsNotNull())
  {
    if(GetDataNode()->GetData() != NULL)
    {
      m_TubeGraph = dynamic_cast< TubeGraph* >(GetDataNode()->GetData());
      m_TubeGraphProperty = dynamic_cast< TubeGraphProperty* >(m_TubeGraph->GetProperty( "Tube Graph.Visualization Information" ).GetPointer());
      if (m_TubeGraphProperty.IsNull())
        MITK_ERROR<< "Something went wrong! No tube graph property!";
    }
    else
      m_TubeGraph = NULL;
  }
  else
    m_TubeGraph = NULL;
}

bool mitk::TubeGraphDataInteractor::CheckOverTube(const InteractionEvent* interactionEvent)
{
  const InteractionPositionEvent* positionEvent = dynamic_cast<const InteractionPositionEvent*>(interactionEvent);
  if(positionEvent == NULL)
    return false;

  TubeGraphPicker* picker = new mitk::TubeGraphPicker();
  picker->SetTubeGraph(m_TubeGraph);

  TubeGraph::TubeDescriptorType tubeDescriptor = picker->GetPickedTube(positionEvent->GetPositionInWorld()).first;

  if(tubeDescriptor != TubeGraph::ErrorId )
  {
    m_SecondLastPickedTube = m_LastPickedTube;
    m_LastPickedTube = tubeDescriptor;
    return true;
  }
  else//nothing picked
    return false;
}

bool mitk::TubeGraphDataInteractor::SelectTube(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (m_TubeGraph.IsNull())
    return false;

  this->SelectTubesByActivationModus();

  interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();

  if (m_ActivationMode != None)
  {
    // show tube id on status bar
    std::stringstream displayText;
    displayText << "Picked tube: ID [" << m_LastPickedTube.first << "," << m_LastPickedTube.second << "]";
    StatusBar::GetInstance()->DisplayText( displayText.str().c_str() );
    //TODO!!!  this->InvokeEvent(SelectionChangedTubeGraphEvent());
  }

  return true;
}

bool mitk::TubeGraphDataInteractor::DeselectTube(StateMachineAction*, InteractionEvent* interactionEvent)
{
  if (m_TubeGraph.IsNull())
    return false;

  if ((m_ActivationMode != Multiple)
    && (m_ActivationMode != Points))
  {
    m_TubeGraphProperty->DeactivateAllTubes();
    interactionEvent->GetSender()->GetRenderingManager()->RequestUpdateAll();
    //TODO!!!this->InvokeEvent(SelectionChangedTubeGraphEvent());
  }
  // show info on status bar
  StatusBar::GetInstance()->DisplayText("No tube hit!");

  return true;
}

void mitk::TubeGraphDataInteractor::SetActivationMode(const ActivationMode& activationMode)
{
  m_ActivationMode = activationMode;
  if (m_TubeGraph.IsNotNull())
    if (m_LastPickedTube != mitk::TubeGraph::ErrorId)
      this->UpdateActivation();
}

mitk::TubeGraphDataInteractor::ActivationMode mitk::TubeGraphDataInteractor::GetActivationMode()
{
  return m_ActivationMode;
}

void mitk::TubeGraphDataInteractor::SetActionMode(const ActionMode& actionMode)
{
  m_ActionMode = actionMode;
}

mitk::TubeGraphDataInteractor::ActionMode mitk::TubeGraphDataInteractor::GetActionMode()
{
  return m_ActionMode;
}

void mitk::TubeGraphDataInteractor::SelectTubesByActivationModus()
{
  if (m_LastPickedTube != mitk::TubeGraph::ErrorId)
  {
    this->UpdateActivation();
  }
}

void mitk::TubeGraphDataInteractor::UpdateActivation()
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
      MITK_WARN<<"Unknown tube graph interaction mode!";
      break;
    }
  }
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphDataInteractor::GetTubesToRoot()
{
  TubeGraph::TubeDescriptorType root = m_TubeGraph->GetRootTube();
  if(root == TubeGraph::ErrorId)
  {
    root = m_TubeGraph->GetThickestTube();
    m_TubeGraph->SetRootTube(root);
  }

  return this->GetPathBetweenTubes(m_LastPickedTube, root);
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphDataInteractor::GetTubesBetweenPoints()
{
  return this->GetPathBetweenTubes(m_LastPickedTube, m_SecondLastPickedTube);
}

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphDataInteractor::GetPathBetweenTubes(const mitk::TubeGraph::TubeDescriptorType& start,const mitk::TubeGraph::TubeDescriptorType& end)
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

std::vector<mitk::TubeGraph::TubeDescriptorType> mitk::TubeGraphDataInteractor::GetPathToPeriphery()
{
  std::vector<mitk::TubeGraph::TubeDescriptorType> solutionPath;

  if (m_LastPickedTube != TubeGraph::ErrorId)
    solutionPath = m_TubeGraph->SearchPathToPeriphery(m_LastPickedTube);

  return solutionPath;
}

void mitk::TubeGraphDataInteractor::ResetPickedTubes()
{
  m_LastPickedTube = TubeGraph::ErrorId;
  m_SecondLastPickedTube = TubeGraph::ErrorId;
}
