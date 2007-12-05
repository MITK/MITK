/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 12289 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkDisplayPointSetInteractor.h"
#include "mitkInteractionConst.h"
#include "mitkStateEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkVector.h"



mitk::DisplayPointSetInteractor
::DisplayPointSetInteractor(const char * type, DataTreeNode* dataTreeNode, int n)
:PointSetInteractor(type, dataTreeNode, n)
{


}

mitk::DisplayPointSetInteractor::~DisplayPointSetInteractor()
{
}


bool mitk::DisplayPointSetInteractor
::ExecuteAction( Action* action, mitk::StateEvent const* stateEvent )
{
  bool ok = false;//for return type bool

  //get the timestep to also support 3D+T
  const mitk::Event *theEvent = stateEvent->GetEvent();

  /*Each case must watch the type of the event!*/
  /*Each time a Point is added or removed or finished moved, the display-coordinates and the last renderer is stored.*/
  switch (action->GetActionId())
  {
  case AcADDPOINT:
    {
      mitk::DisplayPositionEvent const *posEvent = 
        dynamic_cast < const mitk::DisplayPositionEvent * > 
        (stateEvent->GetEvent());
      if ( posEvent == NULL )
      {
        return false;
      }

      m_LastDisplayCoordinates = posEvent->GetDisplayPosition();
      m_LastRenderer = posEvent->GetSender();


      ok = Superclass::ExecuteAction( action, stateEvent );
      break;
    }
  case AcREMOVEPOINT://remove the given Point from the list
    {
      mitk::DisplayPositionEvent const *posEvent = 
        dynamic_cast < const mitk::DisplayPositionEvent * > 
        (stateEvent->GetEvent());
      if ( posEvent == NULL )
      {
        return false;
      }

      m_LastDisplayCoordinates = posEvent->GetDisplayPosition();
      m_LastRenderer = posEvent->GetSender();


      ok = Superclass::ExecuteAction( action, stateEvent );
      break;
    }
  case AcREMOVEALL:
    {
      mitk::DisplayPositionEvent const *posEvent = 
        dynamic_cast < const mitk::DisplayPositionEvent * > 
        (stateEvent->GetEvent());
      if ( posEvent == NULL )
      {
        return false;
      }

      m_LastDisplayCoordinates = posEvent->GetDisplayPosition();
      m_LastRenderer = posEvent->GetSender();


      ok = Superclass::ExecuteAction( action, stateEvent );
      break;
    }
  case AcFINISHMOVEMENT:
    {
      mitk::DisplayPositionEvent const *posEvent = 
        dynamic_cast < const mitk::DisplayPositionEvent * > 
        (stateEvent->GetEvent());
      if ( posEvent == NULL )
      {
        return false;
      }

      m_LastDisplayCoordinates = posEvent->GetDisplayPosition();
      m_LastRenderer = posEvent->GetSender();


      ok = Superclass::ExecuteAction( action, stateEvent );
      break;
    }

  default:
    return Superclass::ExecuteAction( action, stateEvent );
  }

  return ok;
}

mitk::Point2D mitk::DisplayPointSetInteractor::GetLastDisplayCoordinates()
{
  return m_LastDisplayCoordinates;
}

mitk::BaseRenderer* mitk::DisplayPointSetInteractor::GetLastRenderer()
{
  return m_LastRenderer;
}
