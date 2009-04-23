#pragma once

#include "mitkCommon.h"
#include "mitkOperation.h"

#include <vtkPolyData.h>

namespace mitk {

  /*
   * @brief Operation that stores polydata for changing surfaces
   */
  class MITK_CORE_EXPORT SurfaceOperation : public Operation
  {
  public:

     /*
      * Constructor
      * @param operationType type of the operation (OpSURFACECHANGED)
    * @param polyData the polydata object to replace in the surface
    * @param t the time step
      */
    SurfaceOperation(mitk::OperationType operationType,  vtkPolyData* polyData, unsigned int t);

    virtual ~SurfaceOperation();

    vtkPolyData* GetVtkPolyData();

    unsigned int GetTimeStep();

  private:

    vtkPolyData* m_polyData;
    unsigned int m_timeStep;
  };
}; // end namespace mitk

