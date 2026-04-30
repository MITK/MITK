/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCellOperation_h
#define mitkCellOperation_h

#include <MitkDataTypesExtExports.h>
#include <mitkCommon.h>
#include <mitkOperation.h>
#include <mitkVector.h>

namespace mitk
{
  /**
   * \brief Operation that holds everything necessary for an operation on a cell.
   *
   * Stores the operation type, cell identifier, and an optional displacement
   * vector for undo/redo support.
   *
   * \sa Operation, LineOperation
   * \ingroup Undo
   */
  class MITKDATATYPESEXT_EXPORT CellOperation : public Operation
  {
  public:
    mitkClassMacro(CellOperation, Operation);

    /**
     * \brief Construct a CellOperation with a displacement vector.
     *
     * \param[in] operationType The type of operation (e.g. move, add).
     * \param[in] cellId The identifier of the cell to operate on.
     * \param[in] vector The displacement or direction vector.
     */
    CellOperation(OperationType operationType, int cellId, Vector3D vector);

    /**
     * \brief Construct a CellOperation without a displacement vector.
     *
     * \param[in] operationType The type of operation.
     * \param[in] cellId The identifier of the cell to operate on.
     */
    CellOperation(OperationType operationType, int cellId);

    ~CellOperation() override{};

    /**
     * \brief Get the cell identifier.
     * \return The cell ID.
     */
    int GetCellId() { return m_CellId; };

    /**
     * \brief Get the displacement vector.
     * \return The vector associated with this operation.
     */
    Vector3D GetVector() { return m_Vector; };
  protected:
    int m_CellId;
    Vector3D m_Vector;
  };
} // namespace mitk
#endif
