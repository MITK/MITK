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


#ifndef MITKCELLOPERATION_H_INCLUDED
#define MITKCELLOPERATION_H_INCLUDED

#include "mitkCommon.h"
#include "mitkOperation.h"
#include "mitkVector.h"

namespace mitk {

//##Documentation
//## @brief Operation, that holds everything necessary for an operation on a cell.
//##
//## @ingroup Undo
class MITK_CORE_EXPORT CellOperation : public Operation
{
  public:
    mitkClassMacro(CellOperation, Operation);
	//##Documentation
	//##@brief constructor
	//##
	//## @param
	//## operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
	//## cellId: Id of the cell and a vector if needed
    CellOperation(OperationType operationType, int cellId, Vector3D vector);
    
    CellOperation(OperationType operationType, int cellId);
  
    virtual ~CellOperation(){};

    int GetCellId() {return m_CellId;};
    Vector3D GetVector() {return m_Vector;};

  protected:
    int m_CellId;
    Vector3D m_Vector;
};
}//namespace mitk
#endif /* MITKCELLOPERATION_H_INCLUDED*/
