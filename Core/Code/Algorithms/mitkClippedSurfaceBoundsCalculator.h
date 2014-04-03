/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef ClippedSurfaceBoundsCalculator_h_included
#define ClippedSurfaceBoundsCalculator_h_included

#include "mitkImage.h"
#include "mitkPlaneGeometry.h"
#include <vector>

/**
 * \brief Find image slices visible on a given plane.
 *
 * The class name is not helpful in finding this class. Good suggestions welcome.
 *
 * Given a PlaneGeometry (e.g. the 2D plane of a render window), this class
 * calculates which slices of an mitk::Image are visible on this plane.
 * Calculation is done for X, Y, and Z direction, the result is available in
 * form of a pair (minimum,maximum) slice index.
 *
 * Such calculations are useful if you want to display information about the
 * currently visible slice (overlays, statistics, ...) and you don't want to
 * depend on any prior information about hat the renderwindow is currently showing.
 *
 * \warning The interface attempts to look like an ITK filter but it is far from being one.
 */

namespace mitk
{

class MITK_CORE_EXPORT ClippedSurfaceBoundsCalculator
{

  public:

    typedef std::vector<mitk::Point3D> PointListType;

    ClippedSurfaceBoundsCalculator(const mitk::PlaneGeometry* geometry = NULL,
                                   mitk::Image::Pointer image = NULL);
    ClippedSurfaceBoundsCalculator(const mitk::BaseGeometry* geometry,
                                   mitk::Image::Pointer image);
    ClippedSurfaceBoundsCalculator(const PointListType pointlist,
                                   mitk::Image::Pointer image);

    void InitializeOutput();

    virtual ~ClippedSurfaceBoundsCalculator();


    void SetInput(const mitk::PlaneGeometry* geometry, mitk::Image* image);
    void SetInput(const mitk::BaseGeometry *geometry, mitk::Image *image);
    void SetInput(const PointListType pointlist, mitk::Image *image);

    /**
      \brief Request calculation.

      How cut/visible slice indices are determined:
       1. construct a bounding box of the image. This is the box that connect the outer voxel centers(!).
       2. check the edges of this box.
       3. intersect each edge with the plane geometry
          - if the intersection point is within the image box,
            we update the visible/cut slice indices for all dimensions.
          - else we ignore the cut
    */
    void Update();

    /**
      \brief Minimum (first) and maximum (second) slice index.
    */
    typedef std::pair<int, int> OutputType;

    /**
      \brief What X coordinates (slice indices) are cut/visible in given plane.
    */
    OutputType GetMinMaxSpatialDirectionX();

    /**
      \brief What Y coordinates (slice indices) are cut/visible in given plane.
    */
    OutputType GetMinMaxSpatialDirectionY();

    /**
      \brief What Z coordinates (slice indices) are cut/visible in given plane.
    */
    OutputType GetMinMaxSpatialDirectionZ();

  protected:
    void CalculateIntersectionPoints(const mitk::PlaneGeometry* geometry);
    void CalculateIntersectionPoints( PointListType pointList );

    /**
    * \brief Clips the resulting index-coordinates to make sure they do
    * not exceed the imagebounds.
    */
    void EnforceImageBounds();


    mitk::PlaneGeometry::ConstPointer m_PlaneGeometry;
    mitk::BaseGeometry::ConstPointer m_Geometry3D;
    mitk::Image::Pointer m_Image;
    std::vector<mitk::Point3D> m_ObjectPointsInWorldCoordinates;
    std::vector< OutputType > m_MinMaxOutput;

};

} //namespace mitk

#endif

