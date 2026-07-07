/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkLineOperation_h
#define mitkLineOperation_h

#include <MitkDataTypesExtExports.h>
#include <mitkCellOperation.h>
#include <mitkCommon.h>

namespace mitk
{
  /**
   * \brief Operation that holds everything necessary for an operation on a line.
   *
   * Stores operation type, cell ID, two endpoint IDs, an optional line ID,
   * and a displacement vector. Used for selecting, inserting, moving, and
   * removing lines within cells.
   *
   * \sa CellOperation, Operation
   * \ingroup Undo
   */
  class MITKDATATYPESEXT_EXPORT LineOperation : public mitk::CellOperation
  {
  public:
    mitkClassMacro(LineOperation, CellOperation);

    /**
     * \brief Construct a LineOperation with a displacement vector.
     *
     * \param[in] operationType The type of operation (e.g. move, add).
     * \param[in] cellId The cell identifier.
     * \param[in] vector The displacement vector.
     * \param[in] pIdA First endpoint ID (default -1 = unset).
     * \param[in] pIdB Second endpoint ID (default -1 = unset).
     * \param[in] id The line ID within the cell (default -1 = unset).
     */
    LineOperation(OperationType operationType, int cellId, Vector3D vector, int pIdA = -1, int pIdB = -1, int id = -1);

    /**
     * \brief Construct a LineOperation without a displacement vector.
     *
     * \param[in] operationType The type of operation.
     * \param[in] cellId The cell identifier (default -1).
     * \param[in] pIdA First endpoint ID (default -1).
     * \param[in] pIdB Second endpoint ID (default -1).
     * \param[in] id The line ID within the cell (default -1).
     */
    LineOperation(OperationType operationType, int cellId = -1, int pIdA = -1, int pIdB = -1, int id = -1);

    ~LineOperation() override{};

    /**
     * \brief Get the first endpoint ID.
     * \return The point ID A.
     */
    int GetPIdA();

    /**
     * \brief Get the second endpoint ID.
     * \return The point ID B.
     */
    int GetPIdB();

    /**
     * \brief Get the line ID within the cell.
     * \return The line ID.
     */
    int GetId();

  protected:
    //  int m_CellId;
    int m_PIdA;
    int m_PIdB;
    int m_Id;
  };
} // namespace mitk
#endif
