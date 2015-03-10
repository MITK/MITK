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


#ifndef MITKLINEOPERATION_H_INCLUDED
#define MITKLINEOPERATION_H_INCLUDED

#include "mitkCommon.h"
#include "MitkDataTypesExtExports.h"
#include "mitkCellOperation.h"


namespace mitk {

  //##Documentation
  //## @brief Operation, that holds everything necessary for an operation on a line.
  //##
  //## Stores everything for de-/ selecting, inserting , moving and removing a line.
  //## @ingroup Undo
  class MITKDATATYPESEXT_EXPORT LineOperation : public mitk::CellOperation
  {
  public:
    mitkClassMacro(LineOperation, CellOperation);
    //##Documentation
    //##@brief constructor.
    //##
    //## @param operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
    //## @param cellId Id of the cell
    //## @param vector is for movement
    //## @param pIdA and pIdB are Id's of two points
    //## @param id is the Id of a line in a cell
    LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA = -1, int pIdB = -1, int id = -1);
    LineOperation(OperationType operationType, int cellId = -1, int pIdA = -1, int pIdB = -1, int id = -1);

    virtual ~LineOperation(){};

    //int GetCellId();
    int GetPIdA();
    int GetPIdB();
    int GetId();

  protected:
    //  int m_CellId;
    int m_PIdA;
    int m_PIdB;
    int m_Id;
  };
}//namespace mitk
#endif /* MITKLINEOPERATION_H_INCLUDED*/
