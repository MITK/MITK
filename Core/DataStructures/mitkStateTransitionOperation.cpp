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

//##ModelId=3E78311303E7
mitk::StateTransitionOperation::StateTransitionOperation(OperationType operationType, State* state)
: mitk::Operation(operationType), m_State(state)
{}

//##ModelId=3E78307601D8
mitk::State* mitk::StateTransitionOperation::GetState()
{
	return m_State;
}
