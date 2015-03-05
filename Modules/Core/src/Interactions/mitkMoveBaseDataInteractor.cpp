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


#include "mitkMoveBaseDataInteractor.h"
#include "mitkInteractionConst.h"
#include <mitkDataNode.h>
#include "mitkDisplayPositionEvent.h"
#include "mitkStateEvent.h"
#include "mitkProperties.h"

//for an temporary update
#include "mitkRenderingManager.h"

//## Default Constructor
mitk::MoveBaseDataInteractor
::MoveBaseDataInteractor(const char * type, DataNode* dataNode)
:Interactor(type, dataNode)
{
    //define the colors for selected/deselected state
    m_DataNode->AddProperty( "MovingInteractor.SelectedColor", ColorProperty::New(0.0,1.0,0.0) );
    m_DataNode->AddProperty( "MovingInteractor.DeselectedColor", ColorProperty::New(0.0,0.0,1.0) );
    //save the previous color of the node, in order to restore it after the interactor is destroyed
    mitk::ColorProperty::Pointer priorColor = dynamic_cast<mitk::ColorProperty*>(m_DataNode->GetProperty("color"));
    if ( priorColor.IsNotNull() )
    {
        mitk::ColorProperty::Pointer tmpCopyOfPriorColor = mitk::ColorProperty::New();
        tmpCopyOfPriorColor->SetColor( priorColor->GetColor() );
        m_DataNode->AddProperty( "MovingInteractor.PriorColor", tmpCopyOfPriorColor );
    }
}

mitk::MoveBaseDataInteractor::~MoveBaseDataInteractor()
{
    mitk::ColorProperty::Pointer color = dynamic_cast<mitk::ColorProperty*>(m_DataNode->GetProperty("MovingInteractor.PriorColor"));
    if ( color.IsNotNull() )
    {
        m_DataNode->GetPropertyList()->SetProperty("color", color);
    }

    m_DataNode->GetPropertyList()->DeleteProperty("MovingInteractor.SelectedColor");
    m_DataNode->GetPropertyList()->DeleteProperty("MovingInteractor.DeselectedColor");
    m_DataNode->GetPropertyList()->DeleteProperty("MovingInteractor.PriorColor");
    //update rendering
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}


bool mitk::MoveBaseDataInteractor::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;

  /*Each case must watch the type of the event!*/
  switch (action->GetActionId())
  {
  case AcDONOTHING:
    ok = true;
    break;
  case AcCHECKELEMENT:
    /*
    * picking: Answer the question if the given position within stateEvent is close enough to select an object
    * send yes if close enough and no if not picked
    */
    {
      mitk::DisplayPositionEvent const *posEvent = dynamic_cast <const mitk::DisplayPositionEvent *> (stateEvent->GetEvent());
      if (posEvent == NULL)
      {
        MITK_WARN<<"Wrong usage of mitkMoveBaseDataInteractor! Aborting interaction!\n";
        return false;
      }

      mitk::Point3D worldPoint = posEvent->GetWorldPosition();
      /* now we have a worldpoint. check if it is inside our object and select/deselect it accordingly */

      std::auto_ptr<StateEvent> newStateEvent;
      const BaseGeometry* geometry = GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep( m_TimeStep );
      if (geometry->IsInside(worldPoint))
        newStateEvent.reset(new mitk::StateEvent(EIDYES, stateEvent->GetEvent()));
      else
        newStateEvent.reset(new mitk::StateEvent(EIDNO, stateEvent->GetEvent()));

      /* write new state (selected/not selected) to the property */
      this->HandleEvent( newStateEvent.get() );

    ok = true;
    break;
    }
  case AcSELECT:
    // select the data
    {
      mitk::BoolProperty::Pointer selected = dynamic_cast<mitk::BoolProperty*>(m_DataNode->GetProperty("selected"));
      if ( selected.IsNull() )
      {
        selected = mitk::BoolProperty::New();
        m_DataNode->GetPropertyList()->SetProperty("selected", selected);
      }

      mitk::ColorProperty::Pointer selectedColor = dynamic_cast<mitk::ColorProperty*>(m_DataNode->GetProperty("MovingInteractor.SelectedColor"));
      if ( selectedColor.IsNotNull() )
      {
        m_DataNode->GetPropertyList()->SetProperty("color", selectedColor);
      }
      selected->SetValue(true);

      //update rendering
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }
  case AcDESELECT:
    //deselect the data
    {
      mitk::BoolProperty::Pointer selected = dynamic_cast<mitk::BoolProperty*>(m_DataNode->GetProperty("selected"));
      if ( selected.IsNull() )
      {
        selected = mitk::BoolProperty::New();
        m_DataNode->GetPropertyList()->SetProperty("selected", selected);
      }

      mitk::ColorProperty::Pointer deselectedColor =
              dynamic_cast<mitk::ColorProperty*>(m_DataNode->GetProperty("MovingInteractor.DeselectedColor"));
      if ( deselectedColor.IsNotNull() )
      {
        m_DataNode->GetPropertyList()->SetProperty("color", deselectedColor);
      }

      selected = mitk::BoolProperty::New(false);

      //update rendering
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }
  case AcMOVE:
    {
      //modify Geometry from data as given in parameters or in event
      mitk::IntProperty* xP = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_X"));
      mitk::IntProperty* yP = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_Y"));
      mitk::IntProperty* zP = dynamic_cast<mitk::IntProperty*>(action->GetProperty("DIRECTION_Z"));
      if (xP == NULL || yP == NULL || zP == NULL)
      {
        MITK_WARN<<"No properties returned\n!";
        return false;
      }
      mitk::Vector3D movementVector;
      movementVector.SetElement(0, (float) xP->GetValue());
      movementVector.SetElement(1, (float) yP->GetValue());
      movementVector.SetElement(2, (float) zP->GetValue());

      BaseGeometry* geometry = m_DataNode->GetData()->GetUpdatedTimeGeometry()->GetGeometryForTimeStep( m_TimeStep );
      geometry->Translate(movementVector);

      // indicate modification of data tree node
      m_DataNode->Modified();

      //update rendering
      mitk::RenderingManager::GetInstance()->RequestUpdateAll();

      ok = true;
      break;
    }

  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}

/**
\example mitkMoveBaseDataInteractor.cpp
 * This is an example of how to implement a new Interactor.
 * See more details about this example in tutorial Step10.
 */
