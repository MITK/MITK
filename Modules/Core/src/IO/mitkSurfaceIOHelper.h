/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSurfaceIOHelper_h
#define mitkSurfaceIOHelper_h

#include <mitkSurface.h>

#include <vtkSmartPointer.h>

#include <vector>

class vtkPolyData;

namespace mitk
{
  /** \brief Building blocks shared by the surface readers and writers. */
  namespace SurfaceIOHelper
  {
    /**
     * \brief The polygonal data of a time step with the geometry of the time
     * step baked into its points.
     *
     * A time step without points, like an empty time step of a polygon model,
     * gets an empty point set of the same data type as the points of the other
     * time steps, since some formats refuse polygonal data without points and
     * others choose the data type of all time steps from the first one.
     *
     * \throw mitk::Exception if the surface has no polygonal data at the time step.
     */
    vtkSmartPointer<vtkPolyData> GetWorldPolyData(const Surface *surface, TimeStepType timeStep);

    /** \brief Whether every time step is non-empty and starts no earlier than its predecessor ends. */
    bool AreValidTimeBounds(const std::vector<TimeBounds> &timeBounds);

    /**
     * \brief A surface with one time step per polygonal data.
     *
     * Time steps of equal duration without gaps give a ProportionalTimeGeometry,
     * other time steps an ArbitraryTimeGeometry. Without time bounds, the time
     * steps are those of an image: a single time step is valid at every time
     * point, several follow each other from 0 on.
     */
    Surface::Pointer CreateSurface(const std::vector<vtkSmartPointer<vtkPolyData>> &polyDatas,
                                   const std::vector<TimeBounds> &timeBounds);
  }
}

#endif
