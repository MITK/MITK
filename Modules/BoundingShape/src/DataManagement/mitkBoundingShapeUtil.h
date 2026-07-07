/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeUtil_h
#define mitkBoundingShapeUtil_h

#include <mitkBaseData.h>
#include <mitkBaseGeometry.h>
#include <mitkInteractionConst.h>

#include <array>

namespace mitk
{
  /**
  * \brief helper function for calculating corner points of the bounding object from a given geometry
  */
  std::vector<mitk::Point3D> GetCornerPoints(mitk::BaseGeometry::Pointer geometry, bool visualizationOffset);

  /**
  * \brief helper function for calculating the average of two points
  */
  mitk::Point3D CalcAvgPoint(mitk::Point3D a, mitk::Point3D b);

  /**
   * \brief Return the four corner-point indices that define the face associated with the given handle index.
   *
   * \param index The handle index (0-5), corresponding to a face of the bounding box.
   * \return A vector of four indices into the corner points array.
   */
  std::vector<int> GetHandleIndices(int index);

  /**
   * \brief Return the four corner-point indices of the box face that the given handle sits on,
   *        ordered along the face perimeter so that consecutive indices (and the last-to-first
   *        pair) form the four face edges.
   *
   * This is the face that is moved when the handle is dragged. In contrast to GetHandleIndices,
   * which returns the opposite (anchor) face used during scaling, this returns the handle's own
   * face in a winding order suitable for edge-based geometry such as plane intersection.
   *
   * \param handleIndex The handle index (0-5).
   */
  std::array<int, 4> GetHandleFaceCornerIndices(int handleIndex);

  /**
   * \brief Intersect a planar, convex quad (given by its four perimeter-ordered corner points)
   *        with a plane and return the center of the resulting intersection segment.
   *
   * Used to position a face handle where its box face crosses the current slice plane, so the
   * handle stays on the rendered cross-section outline for arbitrarily oriented (oblique) boxes.
   *
   * \param faceCorners The four corner points of the face, in perimeter (winding) order.
   * \param planeOrigin A point on the plane.
   * \param planeNormal The plane normal (need not be normalized).
   * \param[out] center The center of the intersection segment, valid only if the function returns true.
   * \return true if the plane crosses the face (the handle is visible on this slice), false otherwise.
   */
  bool GetFacePlaneIntersectionCenter(const std::array<mitk::Point3D, 4> &faceCorners,
                                      const mitk::Point3D &planeOrigin,
                                      const mitk::Vector3D &planeNormal,
                                      mitk::Point3D &center);

  /**
  * \brief Helper Class for realizing the handles of bounding object encapsulated by a geometry data
  * \ingroup Data
  */
  class Handle final
  {
  public:
    /** \brief Default constructor. Creates an inactive handle at the origin. */
    Handle();

    /**
     * \brief Construct a handle with a given position, index, and associated face indices.
     *
     * \param pos The 3D position of the handle.
     * \param index The handle index (0-5).
     * \param faceIndices Indices of the four corner points forming the associated face.
     * \param active Whether the handle is initially active.
     */
    Handle(mitk::Point3D pos, int index, std::vector<int> faceIndices, bool active = false);

    ~Handle();

    /** \brief Return true if the handle is currently active (selected). */
    bool IsActive();

    /** \brief Return true if the handle is not active. */
    bool IsNotActive();

    /** \brief Set the active (selected) state of the handle. */
    void SetActive(bool status);

    /** \brief Set the index identifying this handle. */
    void SetIndex(int index);

    /** \brief Return the index identifying this handle. */
    int GetIndex();

    /** \brief Return the corner-point indices of the face associated with this handle. */
    std::vector<int> GetFaceIndices();

    /** \brief Set the 3D world position of the handle. */
    void SetPosition(mitk::Point3D pos);

    /** \brief Return the 3D world position of the handle. */
    mitk::Point3D GetPosition();

  private:
    bool m_IsActive;
    mitk::Point3D m_Position;
    std::vector<int> m_FaceIndices;
    int m_Index;
  };
}

#endif
