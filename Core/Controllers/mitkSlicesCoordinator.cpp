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

#include <mitkSlicesCoordinator.h>

namespace mitk {

//SlicesCoordinator::Pointer SlicesCoordinator::New()
//{
//  // from itkNewMacro()
//  Pointer smartPtr;
//  SlicesCoordinator* rawPtr = new SlicesCoordinator("slices-coordinator");
//  smartPtr = rawPtr;
//  rawPtr->UnRegister();
//  return smartPtr;
//}

SlicesCoordinator::SlicesCoordinator(const char* machine)
:StateMachine(machine)
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

void SlicesCoordinator::OnSliceControllerAdded(SliceNavigationController*)
{
  // implement in subclasses
}
  
void SlicesCoordinator::OnSliceControllerRemoved(SliceNavigationController*)
{
  // implement in subclasses
}

bool SlicesCoordinator::ExecuteAction(Action*, StateEvent const*)
{
  // implement in subclasses
  return false;
}

} // namespace 

