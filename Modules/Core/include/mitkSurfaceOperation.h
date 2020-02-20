/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#pragma once

#include "mitkOperation.h"
#include <MitkCoreExports.h>

#include <vtkPolyData.h>

namespace mitk
{
  /*
   * @brief Operation that stores polydata for changing surfaces
   */
  class MITKCORE_EXPORT SurfaceOperation : public Operation
  {
  public:
    /*
     * Constructor
     * @param operationType type of the operation (OpSURFACECHANGED)
   * @param polyData the polydata object to replace in the surface
   * @param t the time step
     */
    SurfaceOperation(mitk::OperationType operationType, vtkPolyData *polyData, unsigned int t);

    ~SurfaceOperation() override;

    vtkPolyData *GetVtkPolyData();

    unsigned int GetTimeStep();

  private:
    vtkPolyData *m_polyData;
    unsigned int m_timeStep;
  };
}; // end namespace mitk
