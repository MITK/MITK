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

#include <mitkSlicesCoordinator.h>

#include <mitkApplicationCursor.h>


namespace mitk {


SlicesCoordinator::SlicesCoordinator(const char* machine)
: StateMachine(machine),
  m_LinkPlanes( true ),
  m_MouseCursorSet( false )
{
}

SlicesCoordinator::~SlicesCoordinator()
{
}

void SlicesCoordinator::AddSliceController(SliceNavigationController* snc)
{
  if (!snc) return;

  m_SliceNavigationControllers.push_back(snc);

  OnSliceControllerAdded(snc); // notify
}

void SlicesCoordinator::RemoveSliceController(SliceNavigationController* snc)
{
  if (!snc) return;

  // see, whether snc is in our list
  SNCVector::iterator iter;
  for (iter = m_SliceNavigationControllers.begin(); iter != m_SliceNavigationControllers.end(); ++iter)
    if (*iter == snc) break;

  // if found, remove from list
  if ( iter != m_SliceNavigationControllers.end() )
  {
    m_SliceNavigationControllers.erase( iter );

    OnSliceControllerRemoved(snc);
  }
}

void SlicesCoordinator::ResetMouseCursor()
{
  if ( m_MouseCursorSet )
  {
    ApplicationCursor::GetInstance()->PopCursor();
    m_MouseCursorSet = false;
  }
}

void SlicesCoordinator::SetMouseCursor( const char *xpm[], int hotspotX, int hotspotY )
{
  // Remove previously set mouse cursor
  if ( m_MouseCursorSet )
  {
    ApplicationCursor::GetInstance()->PopCursor();
  }

  ApplicationCursor::GetInstance()->PushCursor( xpm, hotspotX, hotspotY );
  m_MouseCursorSet = true;
}


void SlicesCoordinator::OnSliceControllerAdded(SliceNavigationController*)
{
  // implement in subclasses
}

void SlicesCoordinator::OnSliceControllerRemoved(SliceNavigationController*)
{
  // implement in subclasses
}

bool SlicesCoordinator::ExecuteAction(Action* a, StateEvent const* e)
{
  // implement in subclasses, BUT call default implementation to make these CONNECT_ACTION statements work out
  return Superclass::ExecuteAction(a,e);
}

} // namespace

