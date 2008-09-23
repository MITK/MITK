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


#include "mitkStateTransitionOperation.h"

mitk::StateTransitionOperation::StateTransitionOperation(OperationType operationType, State* state, unsigned int time)
: mitk::Operation(operationType), m_State(state), m_Time(time)
{}

mitk::StateTransitionOperation::~StateTransitionOperation()
{
  m_State = NULL;
}


mitk::State* mitk::StateTransitionOperation::GetState()
{
  return m_State.GetPointer();
}

unsigned int mitk::StateTransitionOperation::GetTime()
{
	return m_Time;
}
