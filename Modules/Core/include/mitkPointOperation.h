/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPointOperation_h
#define mitkPointOperation_h

#include <mitkNumericTypes.h>
#include <mitkOperation.h>
#include <MitkCoreExports.h>

namespace mitk
{
  /**
   * \brief Operation that handles all actions on a single point.
   *
   * Stores the data needed for adding, moving, and deleting a point,
   * including position, index, selection state, and point type.
   *
   * \ingroup Undo
   * \sa Operation
   * \sa PlaneOperation
   * \sa PointSet
   */
  class MITKCORE_EXPORT PointOperation : public Operation
  {
  public:
    /**
     * \brief Construct a PointOperation without a time stamp.
     *
     * \param operationType The type of operation (e.g. OpINSERT, OpMOVE, OpREMOVE).
     * \param point The 3D coordinates of the point.
     * \param index The index position in the point set (-1 if unspecified).
     * \param selected Whether the point is selected.
     * \param type The point specification type.
     */
    PointOperation(OperationType operationType,
                   Point3D point,
                   int index = -1,
                   bool selected = true,
                   PointSpecificationType type = PTUNDEFINED);

    /**
     * \brief Construct a PointOperation with a time stamp.
     *
     * \param operationType The type of operation (e.g. OpINSERT, OpMOVE, OpREMOVE).
     * \param timeInMS The time stamp in milliseconds.
     * \param point The 3D coordinates of the point.
     * \param index The index position in the point set (-1 if unspecified).
     * \param selected Whether the point is selected.
     * \param type The point specification type.
     */
    PointOperation(OperationType operationType,
                   ScalarType timeInMS,
                   Point3D point,
                   int index = -1,
                   bool selected = true,
                   PointSpecificationType type = PTUNDEFINED);

    ~PointOperation() override;

    /** \brief Get the 3D point coordinates stored in this operation. */
    Point3D GetPoint();

    /** \brief Get the index position of the point in the point set. */
    int GetIndex();

    /** \brief Get whether the point is selected. */
    bool GetSelected();

    /** \brief Get the point specification type. */
    PointSpecificationType GetPointType();

    /** \brief Get the time stamp of this operation in milliseconds. */
    ScalarType GetTimeInMS() const;

  private:
    Point3D m_Point;

    /** \brief Index position of the point in the point set data. */
    int m_Index;

    /** \brief Whether the point is selected or deselected. */
    bool m_Selected;

    /** \brief The specification type of the point. \sa PointSpecificationType */
    PointSpecificationType m_Type;

    ScalarType m_TimeInMS;
  };
} // namespace mitk
#endif
