/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingShapeUtil_h
#define mitkBoundingShapeUtil_h

#include <mitkBaseGeometry.h>

#include <vtkSmartPointer.h>

#include <array>
#include <vector>

class vtkPolyData;

namespace mitk
{
  class BaseRenderer;
  class DataNode;
  class PlaneGeometry;

  /**
  * Names of the DataNode properties through which the bounding shape interactor reports
  * its state to the mappers. The interactor adds them when it attaches to a node and
  * removes them again when it detaches, except for the "Bounding Shape" flag, which it
  * only clears.
  */
  inline constexpr const char *BoundingShapePropertyName = "Bounding Shape";
  inline constexpr const char *BoundingShapeSelectedPropertyName = "Bounding Shape.Selected";
  inline constexpr const char *BoundingShapeActiveHandleIdPropertyName = "Bounding Shape.Active Handle ID";

  /**
  * Names of the DataNode properties that configure the appearance of a bounding shape.
  * Like the regular "color" property, they come with the mapper default properties, belong
  * to the user, and are never written by the interaction.
  */
  inline constexpr const char *BoundingShapeSelectedColorPropertyName = "Bounding Shape.Selected Color";
  inline constexpr const char *BoundingShapeHandleSizeFactorPropertyName = "Bounding Shape.Handle Size Factor";

  /**
  * \brief The value the mappers default the "Bounding Shape.Handle Size Factor" property
  *        to, and the fallback wherever it is missing.
  */
  inline constexpr double DefaultHandleSizeFactor = 0.02;

  /**
  * \brief The 8 corners of the box described by \p geometry, in world coordinates.
  *
  * The corner index is 4x + 2y + z, a set bit meaning the maximum bound of that index axis.
  * With \p visualizationOffset, the corners are shifted by half a voxel, matching the box
  * the mappers render.
  */
  std::array<Point3D, 8> GetCornerPoints(BaseGeometry::Pointer geometry, bool visualizationOffset);

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
  * the corner numbering of GetCornerPoints().
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
   * \param cornerPoints The box corners from GetCornerPoints().
   * \param planeGeometry The slice plane of a 2D render window (must be valid), or nullptr
   *        for a 3D render window.
   */
  std::vector<Handle> ComputeHandles(const std::array<Point3D, 8> &cornerPoints,
                                     const PlaneGeometry *planeGeometry);

  /**
   * \brief Edge length of the handle markers in \p renderer, in world units.
   *
   * Scales the "Bounding Shape.Handle Size Factor" node property to a constant apparent
   * size: relative to the visible extent of the slice in a 2D render window, relative to
   * the camera distance in the 3D one. The mappers draw and the interactor picks at this
   * size, so that the region a handle can be grabbed in follows the size it is drawn at.
   */
  double GetHandleSize(const BaseRenderer *renderer, const DataNode *node);

  /** \brief The handle marker polydata of one render window, as needed by both mappers. */
  struct HandleMarkers
  {
    /** \brief All handles except the hovered one, merged into a single polydata. */
    vtkSmartPointer<vtkPolyData> idleHandles;

    /** \brief The handle the interactor reports as hovered. */
    vtkSmartPointer<vtkPolyData> selectedHandle;
  };

  /**
   * \brief Create the handle markers to render in \p renderer: cubes of the size
   *        GetHandleSize() reports, oriented with \p geometry and centered on the handles
   *        ComputeHandles() places.
   *
   * Either member of the result is null when there is no such handle, and both are while
   * the interactor is detached: handles are interaction affordances, and the interactor
   * adds the active-handle property when it attaches to a node.
   */
  HandleMarkers CreateHandleMarkers(const DataNode *node,
                                    const BaseRenderer *renderer,
                                    const BaseGeometry *geometry,
                                    const std::array<Point3D, 8> &cornerPoints,
                                    const PlaneGeometry *planeGeometry);

  /**
   * \brief Effective color of the bounding shape body: the highlight color while the
   *        interactor reports the shape as selected, the regular node color otherwise.
   */
  void GetBoundingShapeColor(const DataNode *node, const BaseRenderer *renderer, float color[3]);
}

#endif
