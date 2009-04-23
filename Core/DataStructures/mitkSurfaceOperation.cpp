#include "mitkSurfaceOperation.h"

mitk::SurfaceOperation::SurfaceOperation(mitk::OperationType operationType, vtkPolyData* polyData, unsigned int t)
: mitk::Operation(operationType), m_polyData(polyData), m_timeStep(t)
{
}


mitk::SurfaceOperation::~SurfaceOperation()
{
}

vtkPolyData* mitk::SurfaceOperation::GetVtkPolyData()
{
  return m_polyData;
}

unsigned int mitk::SurfaceOperation::GetTimeStep()
{
  return m_timeStep;
}
