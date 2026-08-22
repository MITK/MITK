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

#include <vtkSmartPointer.h>

#include <array>

class vtkPolyData;

namespace mitk
{
  class BaseRenderer;
  class DataNode;
  class PlaneGeometry;

  /**
  * Names of the DataNode properties through which the bounding shape interactor and the
  * mappers communicate. The interactor adds the active-handle and selected properties when
  * it attaches to a node and removes them when it detaches; the regular "color" property
  * is owned by the user and never written by the interaction.
  */
  inline constexpr const char *BoundingShapePropertyName = "Bounding Shape";
  inline constexpr const char *BoundingShapeSelectedPropertyName = "Bounding Shape.Selected";
  inline constexpr const char *BoundingShapeSelectedColorPropertyName = "Bounding Shape.Selected Color";
  inline constexpr const char *BoundingShapeActiveHandleIdPropertyName = "Bounding Shape.Active Handle ID";
  inline constexpr const char *BoundingShapeHandleSizeFactorPropertyName = "Bounding Shape.Handle Size Factor";

  /**
  * \brief helper function for calculating corner points of the bounding object from a given geometry
  */
  std::vector<mitk::Point3D> GetCornerPoints(mitk::BaseGeometry::Pointer geometry, bool visualizationOffset);

  /**
  * \brief helper function for calculating the average of two points
  */
  mitk::Point3D CalcAvgPoint(mitk::Point3D a, mitk::Point3D b);

  /** \brief The bound of one index axis a handle moves when dragged. */
  enum class MovedBound
  {
    None,
    Minimum,
    Maximum
  };

  /** \brief Per index axis (x, y, z): the bound a handle moves when dragged. */
  using AxisMovedBounds = std::array<MovedBound, 3>;

  /** \brief Fallback for the "Bounding Shape.Handle Size Factor" node property when it is not set. */
  inline constexpr double DefaultHandleSizeFactor = 0.02;

  /**
  * \brief A single interaction handle of the bounding shape: its id, world position,
  *        and the index-space bounds it moves when dragged.
  *
  * The ids are shared between the interactor and both mappers, which communicate the
  * hovered handle through the "Bounding Shape.Active Handle ID" node property:
  *
  * | id      | handle                                                              |
  * | ------- | ------------------------------------------------------------------- |
  * | 0 - 5   | face handles, in the order x-max, x-min, z-min, z-max, y-max, y-min |
  * | 6 - 17  | edge handles, 6 + 4 * a + b                                         |
  *
  * where a is the index axis the edge runs along and b combines the min/max bits of
  * the two remaining axes in ascending axis order (bit set = maximum bound), following
  * the corner numbering documented at ComputeHandles().
  *
  * \ingroup Data
  */
  class Handle final
  {
  public:
    /** \brief Creates an invalid handle (index -1) that moves no bounds. */
    Handle();

    Handle(const Point3D &position, int index, const AxisMovedBounds &movedBounds);

    int GetIndex() const;

    /** \brief Per index axis: the bound this handle moves when dragged. */
    const AxisMovedBounds &GetMovedBounds() const;

    Point3D GetPosition() const;

  private:
    Point3D m_Position;
    int m_Index;
    AxisMovedBounds m_MovedBounds;
  };

  /**
   * \brief Compute the handles visible in one render window.
   *
   * For a 3D render window (\p planeGeometry is nullptr): the 6 face handles at the face
   * centers. For a 2D render window (\p planeGeometry is the valid slice plane): the face
   * handles at the centers of their face/plane intersection segments plus an edge handle
   * wherever a box edge crosses the plane, together tracing the rendered cross-section
   * outline with handles on its sides and corners, also for oblique boxes. Handles whose
   * geometry does not cross the plane are omitted.
   *
   * A box corner lying in the plane is shared by three edges. Only the most transverse of
   * them contributes a handle there, because it is the one whose two moved bounds are
   * least parallel to the plane and therefore visibly change the cross-section.
   *
   * \param cornerPoints The 8 box corners from GetCornerPoints() (corner index = 4x + 2y + z,
   *        bit set = maximum bound).
   * \param planeGeometry The slice plane of a 2D render window (must be valid), or nullptr
   *        for a 3D render window.
   */
  std::vector<Handle> ComputeHandles(const std::vector<Point3D> &cornerPoints,
                                     const PlaneGeometry *planeGeometry);

  /**
   * \brief Edge length of the handle markers in \p renderer, in world units.
   *
   * Scales the "Bounding Shape.Handle Size Factor" node property to a constant apparent
   * size: relative to the visible extent of the slice in a 2D render window, relative to
   * the camera distance in the 3D one. Both mappers draw and the interactor picks at this
   * size, so a handle is grabbable exactly where it is visible.
   */
  double GetHandleSize(const BaseRenderer *renderer, const DataNode *node);

  /**
   * \brief Create the marker polydata for one handle: a cube of edge length \p size,
   *        oriented with the direction cosines of \p geometry and centered at \p center.
   */
  vtkSmartPointer<vtkPolyData> CreateHandlePolyData(const BaseGeometry *geometry,
                                                    const Point3D &center,
                                                    double size);

  /**
   * \brief Effective color of the bounding shape body: the highlight color while the
   *        interactor reports the shape as selected, the regular node color otherwise.
   */
  void GetBoundingShapeColor(const DataNode *node, const BaseRenderer *renderer, float color[3]);
}

#endif
