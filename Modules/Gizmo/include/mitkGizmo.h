/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGizmo_h
#define mitkGizmo_h

#include <mitkDataNode.h>
#include <mitkSurface.h>

#include <MitkGizmoExports.h>

namespace mitk
{
  class DataStorage;
  class GizmoRemover;

  /**
   * \brief A geometry manipulation "gizmo" for interactive 3D transformations.
   *
   * This class represents the principal axes of some arbitrary BaseGeometry as a
   * visual 3D widget (Surface). The visualization shows three axes (x, y, z),
   * orthogonal rings around each axis, and a small sphere at the center.
   *
   * The Gizmo is intended to be visualized alongside another data object whose
   * BaseGeometry it follows. It automatically updates its representation in
   * response to ModifiedEvents from the followed geometry, enabling interactive
   * visualization of geometry modifications such as translation, rotation, and
   * scaling.
   *
   * The gizmo is defined by:
   * - a center point
   * - three normalized axis direction vectors (x, y, z)
   * - a radius vector (half-extent per axis in mm)
   *
   * The radius determines the size of the axes and the rings around them.
   * Vertex scalar data encodes which HandleType each part of the mesh corresponds
   * to, enabling handle-based picking.
   *
   * A nice overview of similar / alternative representations can be found in
   * "Schmidt R, Singh K, and Balakrishnan R. Sketching and Composing Widgets for
   * 3D Manipulation. EUROGRAPHICS 2008"
   *
   * \sa GizmoInteractor
   * \sa Surface
   * \sa BaseGeometry
   */
  class MITKGIZMO_EXPORT Gizmo : public Surface
  {
  public:
    /**
     * \brief Enumeration of the three principal axes.
     */
    enum AxisType
    {
      AxisX, ///< The X axis
      AxisY, ///< The Y axis
      AxisZ  ///< The Z axis
    };

    /**
     * \brief Enumeration of interactive handle types for the gizmo.
     *
     * Each value identifies a distinct interactive part of the gizmo
     * visualization. The values are also used as scalar data in the
     * underlying vtkPolyData to enable picking.
     */
    enum HandleType
    {
      MoveFreely,        ///< The central sphere for free translation parallel to the camera plane
      MoveAlongAxisX,    ///< The X axis shaft for constrained translation along X
      MoveAlongAxisY,    ///< The Y axis shaft for constrained translation along Y
      MoveAlongAxisZ,    ///< The Z axis shaft for constrained translation along Z
      RotateAroundAxisX, ///< The ring around X for rotation about the X axis
      RotateAroundAxisY, ///< The ring around Y for rotation about the Y axis
      RotateAroundAxisZ, ///< The ring around Z for rotation about the Z axis
      ScaleX,            ///< The X axis arrow tips for scaling
      ScaleY,            ///< The Y axis arrow tips for scaling
      ScaleZ,            ///< The Z axis arrow tips for scaling
      NoHandle           ///< Indicates that no handle was picked
    };

    /**
     * \brief Convert a HandleType value to a human-readable string.
     *
     * Useful for logging, debugging, and diagnostic output.
     *
     * \param[in] type The handle type to convert.
     * \return A string representation of the handle type, or
     *         "InvalidHandleType" if the value is unrecognized.
     */
    static std::string HandleTypeToString(HandleType type);

    mitkClassMacro(Gizmo, Surface);
    itkNewMacro(Gizmo);

    /** \brief Get the center point of the gizmo. */
    itkGetConstMacro(Center, Point3D);
    /** \brief Set the center point of the gizmo. */
    itkSetMacro(Center, Point3D);

    /** \brief Get the normalized X axis direction vector. */
    itkGetConstMacro(AxisX, Vector3D);
    /** \brief Set the X axis direction vector. */
    itkSetMacro(AxisX, Vector3D);

    /** \brief Get the normalized Y axis direction vector. */
    itkGetConstMacro(AxisY, Vector3D);
    /** \brief Set the Y axis direction vector. */
    itkSetMacro(AxisY, Vector3D);

    /** \brief Get the normalized Z axis direction vector. */
    itkGetConstMacro(AxisZ, Vector3D);
    /** \brief Set the Z axis direction vector. */
    itkSetMacro(AxisZ, Vector3D);

    /** \brief Get the radius vector (half-extent per axis in mm). */
    itkGetConstMacro(Radius, Vector3D);
    /** \brief Set the radius vector (half-extent per axis in mm). */
    itkSetMacro(Radius, Vector3D);

    /** \brief Get whether translation handles are enabled. */
    itkGetConstMacro(AllowTranslation, bool);
    /** \brief Set whether translation handles are enabled. */
    itkSetMacro(AllowTranslation, bool);
    /** \brief Toggle whether translation handles are enabled. */
    itkBooleanMacro(AllowTranslation);

    /** \brief Get whether rotation handles are enabled. */
    itkGetConstMacro(AllowRotation, bool);
    /** \brief Set whether rotation handles are enabled. */
    itkSetMacro(AllowRotation, bool);
    /** \brief Toggle whether rotation handles are enabled. */
    itkBooleanMacro(AllowRotation);

    /** \brief Get whether scaling handles are enabled. */
    itkGetConstMacro(AllowScaling, bool);
    /** \brief Set whether scaling handles are enabled. */
    itkSetMacro(AllowScaling, bool);
    /** \brief Toggle whether scaling handles are enabled. */
    itkBooleanMacro(AllowScaling);

    /**
     * \brief Return the longest of the three axis radii.
     * \return The maximum value among m_Radius[0], m_Radius[1], and m_Radius[2].
     */
    double GetLongestRadius() const;

    /**
     * \brief Rebuild the VTK surface representation after property changes.
     *
     * Call this after modifying center, axes, radius, or permission flags
     * to regenerate the underlying vtkPolyData and update the bounding box.
     */
    void UpdateRepresentation();

    /**
     * \brief Begin observing a geometry for automatic gizmo updates.
     *
     * Registers an ITK observer on the given geometry's ModifiedEvent.
     * Whenever the geometry changes, the gizmo automatically re-reads
     * center, axes, and radius from the geometry and rebuilds itself.
     *
     * \param[in] geom The geometry to follow. Must not be nullptr.
     * \pre \p geom is a valid, initialized BaseGeometry.
     * \post The gizmo reflects the current state of \p geom.
     *
     * \sa OnFollowedGeometryModified
     */
    void FollowGeometry(BaseGeometry *geom);

    /**
     * \brief ITK callback invoked when the followed geometry is modified.
     *
     * Re-reads center, axes, and radius from the followed geometry and
     * calls UpdateRepresentation(). This is registered via FollowGeometry().
     *
     * \sa FollowGeometry
     */
    void OnFollowedGeometryModified();

    /**
     * \brief Identify the gizmo handle that corresponds to a picked VTK point ID.
     *
     * Looks up the scalar data value assigned to the given point in the
     * underlying vtkPolyData and maps it to a HandleType.
     *
     * \param[in] id The VTK point ID obtained from picking.
     * \return The HandleType for the picked point, or NoHandle if identification fails.
     * \pre The gizmo's vtkPolyData and its point scalars must be valid.
     *
     * \sa GetHandleFromPointDataValue
     */
    HandleType GetHandleFromPointID(vtkIdType id);

    /**
     * \brief Identify the gizmo handle from a point data scalar value.
     *
     * Maps a double scalar value (as stored in the vtkPolyData point data)
     * to the corresponding HandleType enumeration value.
     *
     * \param[in] value The scalar value from the point data array.
     * \return The matching HandleType, or NoHandle if no match is found.
     *
     * \sa GetHandleFromPointID
     */
    mitk::Gizmo::HandleType GetHandleFromPointDataValue(double value);

    /**
     * \brief Create and attach a gizmo to a data node.
     *
     * Creates a new Gizmo instance that follows the geometry of the given
     * node, wraps it in a DataNode named "Gizmo", attaches a GizmoInteractor,
     * and optionally adds it to the data storage as a child of \p node.
     * The manipulated object's opacity is lowered to 0.5 for visual clarity.
     *
     * \param[in] node The node whose geometry the gizmo will visualize and
     *                  whose geometry the interactor will manipulate.
     *                  Must not be nullptr and must hold data with a geometry.
     * \param[in] storage The DataStorage to add the gizmo node to. May be
     *                     nullptr, in which case the gizmo is not added to
     *                     any storage.
     * \return A DataNode containing the gizmo, or nullptr if the node has
     *         no data or geometry.
     *
     * \sa RemoveGizmoFromNode
     * \sa HasGizmoAttached
     */
    static DataNode::Pointer AddGizmoToNode(DataNode *node, DataStorage *storage);

    /**
     * \brief Remove a previously attached gizmo from a data node.
     *
     * Finds any Gizmo child nodes of \p node in \p storage, removes them,
     * and restores the original opacity of the manipulated object.
     *
     * \param[in] node The node being currently manipulated.
     * \param[in] storage The DataStorage where the gizmo was added.
     * \return true if at least one gizmo was found and removed; false otherwise.
     *
     * \note Pass the same parameters that were provided to AddGizmoToNode().
     *
     * \sa AddGizmoToNode
     * \sa HasGizmoAttached
     */
    static bool RemoveGizmoFromNode(DataNode *node, DataStorage *storage);

    /**
     * \brief Check whether a gizmo is attached to the given node.
     *
     * Searches for Gizmo-typed child nodes of \p node in \p storage.
     *
     * \param[in] node The node to check.
     * \param[in] storage The DataStorage to search in.
     * \return true if at least one Gizmo child node exists; false otherwise.
     *
     * \sa AddGizmoToNode
     * \sa RemoveGizmoFromNode
     */
    static bool HasGizmoAttached(mitk::DataNode *node, DataStorage *storage);

  protected:
    Gizmo();
    ~Gizmo() override;

    Gizmo(const Gizmo &);            // = delete;
    Gizmo &operator=(const Gizmo &); // = delete;

    /**
     * \brief Build the vtkPolyData that visually represents the gizmo.
     *
     * Constructs axis shafts, arrow cones, rings, and a center sphere
     * based on the current center, axes, radius, and permission flags.
     * Each part is assigned scalar point data encoding its HandleType.
     *
     * \return A new vtkPolyData with the complete gizmo geometry.
     */
    vtkSmartPointer<vtkPolyData> BuildGizmo();

  private:
    Point3D m_Center;
    Vector3D m_AxisX;
    Vector3D m_AxisY;
    Vector3D m_AxisZ;
    Vector3D m_Radius;

    bool m_AllowTranslation;
    bool m_AllowRotation;
    bool m_AllowScaling;

    BaseGeometry::Pointer m_FollowedGeometry;
    //! ITK tag for the observing of m_FollowedGeometry
    unsigned long m_FollowerTag;

    //! Observes a data storage for removal of the manipulated object.
    //! Removes gizmo together with the manipulated object
    std::unique_ptr<GizmoRemover> m_GizmoRemover;
  };
}
#endif
