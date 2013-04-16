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


#ifndef STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462
#define STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462

#include <MitkExports.h>
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
    /**
    * @brief default constructor
    * @param[in] operationType The type of the operation
    * @param[in] state The state to be stored
    * @param[in] time The time according to the state; obligatory when there is only one timestep.
    **/
    StateTransitionOperation(OperationType operationType, State* state, unsigned int time = 0);

    /**
    * @brief default constructor
    **/
    ~StateTransitionOperation();

    /**
    * @brief Return the state
    **/
    State* GetState();

    /**
    * @brief Return the time
    **/
    unsigned int GetTime();

  private:
    State::Pointer m_State;

    unsigned int m_Time;

  };
}//namespace mitk
#endif /* STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462 */
