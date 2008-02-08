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
//##ModelId=3E782FC201BB
//##Documentation
//## @brief operation, that changes the stateMachine and sets a state according to a
//## transition to an other state.
//##
//## @ingroup Undo
class MITK_CORE_EXPORT StateTransitionOperation : public Operation
{
  public:
	  //##ModelId=3E78311303E7
      StateTransitionOperation(OperationType operationType, State* state);

	  //##ModelId=3E78307601D8
	  State* GetState();

  private:
    //##ModelId=3E78305B00D5
    State* m_State;

};
}//namespace mitk
#endif /* STATETRANSITIONOPERATION_H_HEADER_INCLUDED_C1695462 */
