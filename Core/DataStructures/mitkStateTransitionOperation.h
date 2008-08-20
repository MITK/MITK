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


#ifndef STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462
#define STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462

#include "mitkCommon.h"
#include "mitkOperation.h"
#include "mitkState.h"

namespace mitk {
  //##Documentation
  //## @brief operation, that changes the stateMachine and sets a state according to a
  //## transition to an other state.
  //##
  //## @ingroup Undo
  class MITK_CORE_EXPORT StateTransitionOperation : public Operation
  {
  public:
    StateTransitionOperation(OperationType operationType, State* state);

    State* GetState();

  private:
    State* m_State;

  };
}//namespace mitk
#endif /* STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462 */
