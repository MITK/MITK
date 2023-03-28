/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCellOperation_h
#define mitkCellOperation_h

#include "MitkDataTypesExtExports.h"
#include "mitkCommon.h"
#include "mitkOperation.h"
#include "mitkVector.h"

namespace mitk
{
  //##Documentation
  //## @brief Operation, that holds everything necessary for an operation on a cell.
  //##
  //## @ingroup Undo
  class MITKDATATYPESEXT_EXPORT CellOperation : public Operation
  {
  public:
    mitkClassMacro(CellOperation, Operation);
    //##Documentation
    //##@brief constructor
    //##
    //## @param operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
    //## @param cellId Id of the cell and a vector if needed
    //## @param vector
    CellOperation(OperationType operationType, int cellId, Vector3D vector);

    CellOperation(OperationType operationType, int cellId);

    ~CellOperation() override{};

    int GetCellId() { return m_CellId; };
    Vector3D GetVector() { return m_Vector; };
  protected:
    int m_CellId;
    Vector3D m_Vector;
  };
} // namespace mitk
#endif
