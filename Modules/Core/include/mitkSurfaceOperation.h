/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceOperation_h
#define mitkSurfaceOperation_h

#include <mitkOperation.h>
#include <MitkCoreExports.h>

#include <vtkPolyData.h>

namespace mitk
{
  /**
   * \brief Operation that stores polydata for changing surfaces.
   */
  class MITKCORE_EXPORT SurfaceOperation : public Operation
  {
  public:
    /**
     * \brief Construct a SurfaceOperation.
     *
     * \param[in] operationType type of the operation (e.g. OpSURFACECHANGED).
     * \param[in] polyData the polydata object to replace in the surface.
     * \param[in] t the time step.
     */
    SurfaceOperation(mitk::OperationType operationType, vtkPolyData *polyData, unsigned int t);

    ~SurfaceOperation() override;

    /**
     * \brief Get the VTK polydata stored in this operation.
     * \return Pointer to the vtkPolyData object.
     */
    vtkPolyData *GetVtkPolyData();

    /**
     * \brief Get the time step associated with this operation.
     * \return The time step index.
     */
    unsigned int GetTimeStep();

  private:
    vtkPolyData *m_polyData;
    unsigned int m_timeStep;
  };
}

#endif
