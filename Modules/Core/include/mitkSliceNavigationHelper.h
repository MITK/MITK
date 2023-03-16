/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkSliceNavigationHelper_h
#define mitkSliceNavigationHelper_h

#include <MitkCoreExports.h>

#include <mitkPlaneGeometry.h>
#include <mitkTimeGeometry.h>

namespace mitk
{
  namespace SliceNavigationHelper
  {

    /**
    * \brief Select a specific slice from the given time geometry given a 3D point.
    *
    * The function uses the currently selected time point to retrieve the current base geometry from
    * the given time geometry.
    * If this base geometry is a SlicedGeometry3D, the best fitting slice is computed by projecting
    * the point onto the plane geometry of this SlicedGeometry.
    * The slice with the shortest distance to the point is returned as the selected slice.
    *
    * \param timeGeometry  The TimeGeometry of which the slice should be selected.
    * \param point         The Point3D which is used to select the specific slice.
    *
    * \pre The given TimeGeometry must cover the currently selected time point. If not, an exception is thrown.
    *      If the given TimeGeomety is a nullptr, -1 is returned.
    *
    * \return The selected slice as unsigned int. If the computed slice is negative, "0" (zero) is
    *         returned as the selected slice.
    */
    MITKCORE_EXPORT unsigned int SelectSliceByPoint(const TimeGeometry* timeGeometry, const Point3D& point);

    /**
    * \brief Create a new time geometry, which is oriented with the given plane orientation.
    *
    * The function uses the currently selected time point to retrieve the current base geometry from
    * the given time geometry.
    * A new SlicedGeometry3D is created and initialized with the given parameters and the extracted base geometry.
    * This new SlicedGeometry3D is then used to replace the geometries of each time step of a cloned version
    * of the given TimeGeometry. This allows to only replace the contained geometries for each time step,
    * keeping the time steps (time point + time duration) from the original time geometry.
    *
    * \param timeGeometry       The TimeGeometry which should be used for cloning.
    * \param orientation        The AnatomicalPlane that specifies the new orientation of the time geometry.
    * \param top                If true, create the plane at top, otherwise at bottom.
    * \param frontside          Defines the side of the plane.
    * \param rotated            Rotates the plane by 180 degree around its normal.
    *                           s.a. PlaneGeometry::InitializeStandardPlane
    *
    * \pre The given TimeGeometry must cover the currently selected time point. If not, an exception is thrown.
    *      If the given TimeGeomety is a nullptr, a nullptr is returned immediately.
    *
    * \return The created geometry as TimeGeometry.
    */
    MITKCORE_EXPORT TimeGeometry::Pointer CreateOrientedTimeGeometry(const TimeGeometry* timeGeometry,
                                                                     AnatomicalPlane orientation,
                                                                     bool top,
                                                                     bool frontside,
                                                                     bool rotated);

    /**
    * \brief Extracts the plane geometry for the given time step and slice position.
    *
    * The function uses the given time point to retrieve the current base geometry from
    * the given time geometry.
    * If this base geometry is a SlicedGeometry3D, the plane geometry of the given
    * slice position is extracted and returned.
    * If not, a nullptr is returned.
    *
    * \param timeGeometry     The TimeGeometry of which the plane geometry should be extracted
    * \param timePoint        The time point to extract the current base geometry.
    * \param slicePosition    The slice position to extract the current plane geometry.
    *
    * \pre The given TimeGeometry must cover the currently selected time point. If not, an exception is thrown.
    *      If the given TimeGeomety is a nullptr, a nullptr is returned immediately.
    *
    * \return The extracted plane geometry as PlaneGeometry.
    *         Nullptr, if no SlicedGeometry3D was found for the given time step.
    */
    MITKCORE_EXPORT PlaneGeometry* GetCurrentPlaneGeometry(const TimeGeometry* timeGeometry,
                                                           TimePointType timePoint,
                                                           unsigned int slicePosition);
  } // namespace SliceNavigationHelper
} // namespace mitk

#endif
