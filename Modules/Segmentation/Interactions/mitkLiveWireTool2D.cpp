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

#include "mitkLiveWireTool2D.h"

#include "mitkToolManager.h"

#include "mitkBaseRenderer.h"
#include "mitkRenderingManager.h"

#include "mitkLiveWireTool2D.xpm"

#include <mitkInteractionConst.h>
#include <mitkContourModelInteractor.h>
#include <mitkGlobalInteraction.h>


namespace mitk {
  MITK_TOOL_MACRO(Segmentation_EXPORT, LiveWireTool2D, "LiveWire tool");
}



mitk::LiveWireTool2D::LiveWireTool2D()
:SegTool2D("LiveWireTool") 
{
  m_Contour = mitk::ContourModel::New();
  m_ContourModelNode = mitk::DataNode::New();
  m_ContourModelNode->SetData( m_Contour );
  m_ContourModelNode->SetProperty("name", StringProperty::New("contour node"));
  m_ContourModelNode->SetProperty("visible", BoolProperty::New(true));
  m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.9, 1.0, 0.1), NULL, true );
  m_ContourModelNode->AddProperty( "selectedcolor", ColorProperty::New(1.0, 0.0, 0.1), NULL, true );


  m_LiveWireContour = mitk::ContourModel::New();
  m_LiveWireContourNode = mitk::DataNode::New();
  m_LiveWireContourNode->SetData( m_LiveWireContour );
  m_LiveWireContourNode->SetProperty("name", StringProperty::New("contour node"));
  m_LiveWireContourNode->SetProperty("visible", BoolProperty::New(true));
  m_LiveWireContourNode->AddProperty( "color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_LiveWireContourNode->AddProperty( "selectedcolor", ColorProperty::New(0.5, 0.5, 0.1), NULL, true );

 
  
  // great magic numbers
  CONNECT_ACTION( AcINITNEWOBJECT, OnInitLiveWire );
  CONNECT_ACTION( AcADDPOINT, OnAddPoint );
  CONNECT_ACTION( AcMOVE, OnMouseMoved );
  CONNECT_ACTION( AcCHECKPOINT, OnCheckPoint );
  CONNECT_ACTION( AcFINISH, OnFinish );
  CONNECT_ACTION( AcCLOSECONTOUR, OnCloseContour );
}




mitk::LiveWireTool2D::~LiveWireTool2D()
{
}




const char** mitk::LiveWireTool2D::GetXPM() const
{
  return mitkLiveWireTool2D_xpm;
}




const char* mitk::LiveWireTool2D::GetName() const
{
  return "LiveWire";
}




void mitk::LiveWireTool2D::Activated()
{
  Superclass::Activated();
}




void mitk::LiveWireTool2D::Deactivated()
{
  Superclass::Deactivated();
}




bool mitk::LiveWireTool2D::OnInitLiveWire (Action* action, const StateEvent* stateEvent)
{
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  m_LastEventSender = positionEvent->GetSender();
  m_LastEventSlice = m_LastEventSender->GetSlice();

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;


  int timestep = positionEvent->GetSender()->GetTimeStep();

  m_ToolManager->GetDataStorage()->Add( m_ContourModelNode );

  m_ToolManager->GetDataStorage()->Add( m_LiveWireContourNode );
  
  m_LiveWireContour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true, timestep );


  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}




bool mitk::LiveWireTool2D::OnAddPoint (Action* action, const StateEvent* stateEvent)
{
  //complete LiveWire interaction for last segment
  //add current LiveWire contour to the finished contour and reset
  //to start new segment and computation

  /* check if event can be handled */
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;

  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;
  /* END check if event can be handled */


  int timestep = positionEvent->GetSender()->GetTimeStep();

  //remove duplicate first vertex, it's already contained in m_Contour
  m_LiveWireContour->RemoveVertexAt(0, timestep);

  //merge contours
  m_Contour->Concatenate(m_LiveWireContour, timestep);


  //clear the livewire contour and reset the corresponding datanode
  m_LiveWireContour->Clear();
  m_LiveWireContour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true, timestep );

  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}




bool mitk::LiveWireTool2D::OnMouseMoved( Action* action, const StateEvent* stateEvent)
{
  //compute LiveWire segment from last control point to current mouse position

  /* check if event can be handled */
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  /* END check if event can be handled */


  /* actual LiveWire computation */
  int timestep = positionEvent->GetSender()->GetTimeStep();

  ContourModel::VertexType* currentVertex = const_cast<ContourModel::VertexType*>(m_LiveWireContour->GetVertexAt(0));

  m_LiveWireContour->Clear();
  m_LiveWireContour->AddVertex(*currentVertex, timestep);
  m_LiveWireContour->AddVertex( const_cast<mitk::Point3D &>(positionEvent->GetWorldPosition()), true, timestep );
  /* END actual LiveWire computation */


  //render
  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}




bool mitk::LiveWireTool2D::OnCheckPoint( Action* action, const StateEvent* stateEvent)
{
  //check double click on first control point to finish the LiveWire tool
  // 
  //Check distance to first point.
  //Transition YES if click close to first control point 
  //

  /* check if event can be handled */
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  /* END check if event can be handled */


  mitk::StateEvent* newStateEvent = NULL;

  mitk::Point3D click = positionEvent->GetWorldPosition();

  mitk::Point3D first = this->m_Contour->GetVertexAt(0)->Coordinates;


  if (first.EuclideanDistanceTo(click) < 1.5)
  {
    //finish
    newStateEvent = new mitk::StateEvent(EIDYES, stateEvent->GetEvent());
  }else
  {
    //stay active
    newStateEvent = new mitk::StateEvent(EIDNO, stateEvent->GetEvent());
  }

  this->HandleEvent( newStateEvent );


  return true;
}




bool mitk::LiveWireTool2D::OnFinish( Action* action, const StateEvent* stateEvent)
{
  // finish livewire tool interaction 
  
  /* check if event can be handled */
  if ( Superclass::CanHandleEvent(stateEvent) < 1.0 ) return false;

  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  /* END check if event can be handled */


  //TODO visual feedback for completing livewire tool
  m_ContourModelNode->AddProperty( "color", ColorProperty::New(1.0, 1.0, 0.1), NULL, true );

  //set the livewire interactor to edit control points
  mitk::ContourModelInteractor::Pointer interactor = mitk::ContourModelInteractor::New(m_ContourModelNode);
  m_ContourModelNode->SetInteractor(interactor);

  //add interactor to globalInteraction instance
  mitk::GlobalInteraction::GetInstance()->AddInteractor(interactor);
  /* END complete livewire tool interaction */


  /* reset contours and datanodes */
  m_Contour = mitk::ContourModel::New();
  m_ContourModelNode = mitk::DataNode::New();
  m_ContourModelNode->SetData( m_Contour );
  m_ContourModelNode->SetProperty("name", StringProperty::New("contour node"));
  m_ContourModelNode->SetProperty("visible", BoolProperty::New(true));
  m_ContourModelNode->AddProperty( "color", ColorProperty::New(0.9, 1.0, 0.1), NULL, true );
  m_ContourModelNode->AddProperty( "selectedcolor", ColorProperty::New(1.0, 0.0, 0.1), NULL, true );

  m_LiveWireContour = mitk::ContourModel::New();
  m_LiveWireContourNode = mitk::DataNode::New();
  m_LiveWireContourNode->SetData( m_LiveWireContour );
  m_LiveWireContourNode->SetProperty("name", StringProperty::New("contour node"));
  m_LiveWireContourNode->SetProperty("visible", BoolProperty::New(true));
  m_LiveWireContourNode->AddProperty( "color", ColorProperty::New(0.1, 1.0, 0.1), NULL, true );
  m_LiveWireContourNode->AddProperty( "selectedcolor", ColorProperty::New(0.5, 0.5, 0.1), NULL, true );
  /* END reset contours and datanodes */

  //deactivate tool and go to initial state
  this->Deactivated();

  return true;
}




bool mitk::LiveWireTool2D::OnCloseContour( Action* action, const StateEvent* stateEvent)
{
  
  const PositionEvent* positionEvent = dynamic_cast<const PositionEvent*>(stateEvent->GetEvent());
  if (!positionEvent) return false;
  
  if ( CanHandleEvent(stateEvent) < 1.0 ) return false;

  m_Contour->Close();

  assert( positionEvent->GetSender()->GetRenderWindow() );
  mitk::RenderingManager::GetInstance()->RequestUpdate( positionEvent->GetSender()->GetRenderWindow() );

  return true;
}
