#include "mitkCellOperation.h"

mitk::CellOperation::CellOperation(OperationType operationType, int cellId, Vector3D vector)
: mitk::Operation(operationType), m_CellId(cellId), m_Vector(vector)
{
}

mitk::CellOperation::CellOperation(OperationType operationType, int cellId)
: mitk::Operation(operationType), m_CellId(cellId)
{
  m_Vector.Fill(0);
}
