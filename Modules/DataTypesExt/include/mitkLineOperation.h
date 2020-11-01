/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKLINEOPERATION_H_INCLUDED
#define MITKLINEOPERATION_H_INCLUDED

#include "MitkDataTypesExtExports.h"
#include "mitkCellOperation.h"
#include "mitkCommon.h"

namespace mitk
{
  /** @brief Operation, that holds everything necessary for an operation on a line.
    * Stores everything for de-/ selecting, inserting , moving and removing a line.
    * @ingroup Undo
    */
  class MITKDATATYPESEXT_EXPORT LineOperation : public mitk::CellOperation
  {
  public:
    mitkClassMacro(LineOperation, CellOperation);
    /** @brief constructor.
      * @param operationType is the type of that operation (see mitkOperation.h; e.g. move or add; Information for StateMachine::ExecuteOperation());
      * @param cellId Id of the cell
      * @param vector is for movement
      * @param pIdA and pIdB are Id's of two points
      * @param pIdB
      * @param id is the Id of a line in a cell
      */
    LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA = -1, int pIdB = -1, int id = -1);
    LineOperation(OperationType operationType, int cellId = -1, int pIdA = -1, int pIdB = -1, int id = -1);

    ~LineOperation() override{};

    // int GetCellId();
    int GetPIdA();
    int GetPIdB();
    int GetId();

  protected:
    //  int m_CellId;
    int m_PIdA;
    int m_PIdB;
    int m_Id;
  };
} // namespace mitk
#endif /* MITKLINEOPERATION_H_INCLUDED*/
