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


#ifndef OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD
#define OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD

#include "mitkCommon.h"

namespace mitk 
{

class Operation;
class OperationEvent;

/** Macro for checking the type of an operation*/
#define mitkCheckOperationTypeMacro(OperationType, operation, newOperationName) \
  mitk::OperationType *newOperationName= dynamic_cast<mitk::OperationType *>(operation);\
	if (newOperationName == NULL)\
	{\
    itkWarningMacro("Recieved wrong type of operation!");\
		return;\
	}\

//##ModelId=3E5F62430388
//##Documentation
//## @brief abstract class, that can be used  by Undo to undo an operation.
//## 
//## @ingroup Undo
class OperationActor
{
public:
    virtual ~OperationActor() {};
    //##ModelId=3E5F62530359
    virtual void ExecuteOperation(Operation* operation) = 0;
};
}


#endif /* OPERATIONACTOR_H_HEADER_INCLUDED_C16E28BD */
