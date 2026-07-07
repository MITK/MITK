/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkClippedSurfaceBoundsCalculator_h
#define mitkClippedSurfaceBoundsCalculator_h

#include <mitkImage.h>
#include <mitkPlaneGeometry.h>
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
 *
 * \sa PlaneGeometry BaseGeometry Image
 */

namespace mitk
{
  class MITKCORE_EXPORT ClippedSurfaceBoundsCalculator
  {
  public:
    /** \brief List of 3D world-coordinate points. */
    typedef std::vector<mitk::Point3D> PointListType;

    /**
     * \brief Construct from a PlaneGeometry and image.
     * \param geometry The clipping plane (may be nullptr).
     * \param image    The image whose slice bounds are computed (may be nullptr).
     */
    ClippedSurfaceBoundsCalculator(const mitk::PlaneGeometry *geometry = nullptr, mitk::Image::Pointer image = nullptr);

    /**
     * \brief Construct from a BaseGeometry (e.g. SlicedGeometry3D) and image.
     * \param geometry The 3D geometry defining the clipping volume.
     * \param image    The image whose slice bounds are computed.
     */
    ClippedSurfaceBoundsCalculator(const mitk::BaseGeometry *geometry, mitk::Image::Pointer image);

    /**
     * \brief Construct from a list of world-coordinate points and image.
     * \param pointlist Points whose bounding slice indices are determined.
     * \param image     The image whose slice bounds are computed.
     */
    ClippedSurfaceBoundsCalculator(const PointListType pointlist, mitk::Image::Pointer image);

    /** \brief Reset the output min/max indices to their initial (invalid) state. */
    void InitializeOutput();

    virtual ~ClippedSurfaceBoundsCalculator();

    /**
     * \brief Set a PlaneGeometry and image as input.
     * \param geometry The clipping plane.
     * \param image    The image whose slice bounds are computed.
     */
    void SetInput(const mitk::PlaneGeometry *geometry, mitk::Image *image);

    /**
     * \brief Set a BaseGeometry and image as input.
     * \param geometry The 3D geometry defining the clipping volume.
     * \param image    The image whose slice bounds are computed.
     */
    void SetInput(const mitk::BaseGeometry *geometry, mitk::Image *image);

    /**
     * \brief Set a list of world-coordinate points and image as input.
     * \param pointlist Points whose bounding slice indices are determined.
     * \param image     The image whose slice bounds are computed.
     */
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
     * \brief Get the min/max slice indices visible in the X direction.
     * \return A pair of (minimum, maximum) slice index in X.
     */
    OutputType GetMinMaxSpatialDirectionX();

    /**
     * \brief Get the min/max slice indices visible in the Y direction.
     * \return A pair of (minimum, maximum) slice index in Y.
     */
    OutputType GetMinMaxSpatialDirectionY();

    /**
     * \brief Get the min/max slice indices visible in the Z direction.
     * \return A pair of (minimum, maximum) slice index in Z.
     */
    OutputType GetMinMaxSpatialDirectionZ();

  protected:
    /**
     * \brief Calculate intersection points of the image bounding box edges with the given plane.
     * \param geometry The plane geometry to intersect with.
     */
    void CalculateIntersectionPoints(const mitk::PlaneGeometry *geometry);

    /**
     * \brief Calculate bounding slice indices from a list of world-coordinate points.
     * \param pointList The points to convert to index coordinates.
     */
    void CalculateIntersectionPoints(PointListType pointList);

    /**
     * \brief Clamp the resulting index coordinates so they do not exceed the image bounds.
     */
    void EnforceImageBounds();

    mitk::PlaneGeometry::ConstPointer m_PlaneGeometry;  ///< The input plane geometry (if set).
    mitk::BaseGeometry::ConstPointer m_Geometry3D;  ///< The input 3D geometry (if set).
    mitk::Image::Pointer m_Image;  ///< The image whose bounds are calculated.
    std::vector<mitk::Point3D> m_ObjectPointsInWorldCoordinates;  ///< World-coordinate points (if set).
    std::vector<OutputType> m_MinMaxOutput;  ///< Min/max slice index results for each axis.
  };

} // namespace mitk

#endif
