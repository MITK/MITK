/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAbstractTransformGeometry_h
#define mitkAbstractTransformGeometry_h

#include <mitkPlaneGeometry.h>
#include <MitkCoreExports.h>

#include <itkVtkAbstractTransform.h>

class vtkAbstractTransform;

namespace mitk
{
  /**
   * \brief Geometry describing a curved 2D manifold in 3D space via a vtkAbstractTransform.
   *
   * AbstractTransformGeometry combines a PlaneGeometry (m_Plane), which
   * defines the 2D parameter rectangle, with a vtkAbstractTransform
   * (m_VtkAbstractTransform), which maps the planar rectangle into an
   * arbitrary 2D manifold embedded in 3D space.
   *
   * The workflow is: 2D parameter coordinates are first mapped through
   * the PlaneGeometry into 3D coordinates, and those 3D coordinates are
   * then transformed by the vtkAbstractTransform to produce the final
   * world coordinates on the curved surface.
   *
   * \note This class provides read-only access to the vtkAbstractTransform
   *       and the m_Plane. For full external write access, use
   *       ExternAbstractTransformGeometry.
   * \note The bounds of the PlaneGeometry serve as the parametric bounds.
   *
   * \sa PlaneGeometry, ExternAbstractTransformGeometry
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT AbstractTransformGeometry : public PlaneGeometry
  {
  public:
    mitkClassMacro(AbstractTransformGeometry, PlaneGeometry);

    itkFactorylessNewMacro(Self);

    itkCloneMacro(Self);

      /**
       * \brief Get the underlying vtkAbstractTransform.
       *
       * \return Pointer to the vtkAbstractTransform that maps the planar
       *         parameter space into the curved 3D manifold.
       */
      virtual vtkAbstractTransform *GetVtkAbstractTransform() const;

    /**
     * \brief Get the modification time, accounting for the transform.
     *
     * Returns the later of this geometry's own MTime and the
     * vtkAbstractTransform's MTime.
     *
     * \return The effective modification time.
     */
    itk::ModifiedTimeType GetMTime() const override;

    /**
     * \brief Get the planar rectangle used as the 2D parameter space.
     *
     * The bounds of this PlaneGeometry define the parametric domain that
     * is mapped through the vtkAbstractTransform.
     *
     * \return Const pointer to the internal PlaneGeometry.
     */
    itkGetConstObjectMacro(Plane, PlaneGeometry);

    /**
     * \brief Project a 3D point onto the curved surface.
     *
     * Maps the point to 2D parameter space and back, yielding the closest
     * point on the manifold.
     *
     * \param[in] pt3d_mm Point in world coordinates (mm).
     * \param[out] projectedPt3d_mm The projected point on the surface (mm).
     * \return true if the projected point lies within the parametric bounds.
     */
    bool Project(const mitk::Point3D &pt3d_mm, mitk::Point3D &projectedPt3d_mm) const override;

    /**
     * \brief Project a 3D vector at a given point onto the curved surface.
     *
     * \warning Not yet satisfactorily implemented; throws an exception.
     *
     * \param[in] atPt3d_mm The point of application in world coordinates (mm).
     * \param[in] vec3d_mm The vector to project (mm).
     * \param[out] projectedVec3d_mm The projected vector on the surface (mm).
     * \return true if projection was possible.
     */
    bool Project(const mitk::Point3D &atPt3d_mm,
                         const mitk::Vector3D &vec3d_mm,
                         mitk::Vector3D &projectedVec3d_mm) const override;

    /**
     * \brief Project a 3D vector onto the curved surface.
     *
     * \warning Not yet satisfactorily implemented; throws an exception.
     *
     * \param[in] vec3d_mm The vector to project (mm).
     * \param[out] projectedVec3d_mm The projected vector (mm).
     * \return true if projection was possible.
     */
    bool Project(const mitk::Vector3D &vec3d_mm, mitk::Vector3D &projectedVec3d_mm) const override;

    /**
     * \brief Map a 3D world point onto the 2D parameter space.
     *
     * Applies the inverse vtkAbstractTransform, then delegates to the
     * internal PlaneGeometry's Map().
     *
     * \param[in] pt3d_mm Point in world coordinates (mm).
     * \param[out] pt2d_mm Mapped 2D point in parameter space (mm).
     * \return true if the point lies within the parametric bounds.
     */
    bool Map(const mitk::Point3D &pt3d_mm, mitk::Point2D &pt2d_mm) const override;

    /**
     * \brief Map a 2D parameter-space point to 3D world coordinates.
     *
     * Delegates to the PlaneGeometry's Map(), then applies the
     * vtkAbstractTransform.
     *
     * \param[in] pt2d_mm 2D point in parameter space (mm).
     * \param[out] pt3d_mm Resulting 3D world point (mm).
     */
    void Map(const mitk::Point2D &pt2d_mm, mitk::Point3D &pt3d_mm) const override;

    /**
     * \brief Map a 3D vector at a given 3D point to a 2D parameter-space vector.
     *
     * \param[in] atPt3d_mm Application point in world coordinates (mm).
     * \param[in] vec3d_mm 3D vector (mm).
     * \param[out] vec2d_mm Mapped 2D vector in parameter space (mm).
     * \return true if the mapping was possible.
     */
    bool Map(const mitk::Point3D &atPt3d_mm,
                     const mitk::Vector3D &vec3d_mm,
                     mitk::Vector2D &vec2d_mm) const override;

    /**
     * \brief Map a 2D parameter-space vector to a 3D world vector.
     *
     * \param[in] atPt2d_mm Application point in 2D parameter space (mm).
     * \param[in] vec2d_mm 2D vector (mm).
     * \param[out] vec3d_mm Resulting 3D world vector (mm).
     */
    void Map(const mitk::Point2D &atPt2d_mm,
                     const mitk::Vector2D &vec2d_mm,
                     mitk::Vector3D &vec3d_mm) const override;

    /**
     * \brief Convert 2D index coordinates to 2D world coordinates via the internal plane.
     *
     * \param[in] pt_units 2D point in index coordinates.
     * \param[out] pt_mm 2D point in world coordinates (mm).
     */
    void IndexToWorld(const mitk::Point2D &pt_units, mitk::Point2D &pt_mm) const override;

    /**
     * \brief Convert 2D world coordinates to 2D index coordinates via the internal plane.
     *
     * \param[in] pt_mm 2D point in world coordinates (mm).
     * \param[out] pt_units 2D point in index coordinates.
     */
    void WorldToIndex(const mitk::Point2D &pt_mm, mitk::Point2D &pt_units) const override;

    /**
     * \brief Convert 2D index coordinates of a vector to world coordinates (mm).
     *
     * \deprecated The first parameter \a atPt2d_units is unused.
     *             Use IndexToWorld(const Vector2D&, Vector2D&) instead.
     *
     * \param[in] atPt2d_units Unused point parameter.
     * \param[in] vec_units Vector in index coordinates.
     * \param[out] vec_mm Vector in world coordinates (mm).
     */
    void IndexToWorld(const mitk::Point2D &atPt2d_units,
                              const mitk::Vector2D &vec_units,
                              mitk::Vector2D &vec_mm) const override;

    /**
     * \brief Convert 2D index coordinates of a vector to world coordinates (mm).
     *
     * \param[in] vec_units Vector in index coordinates.
     * \param[out] vec_mm Vector in world coordinates (mm).
     */
    void IndexToWorld(const mitk::Vector2D &vec_units, mitk::Vector2D &vec_mm) const override;

    /**
     * \brief Convert world coordinates (mm) of a 2D vector to continuous index coordinates.
     *
     * \deprecated The first parameter \a atPt2d_mm is unused.
     *             Use WorldToIndex(const Vector2D&, Vector2D&) instead.
     *
     * \param[in] atPt2d_mm Unused point parameter.
     * \param[in] vec_mm Vector in world coordinates (mm).
     * \param[out] vec_units Vector in continuous index coordinates.
     */
    void WorldToIndex(const mitk::Point2D &atPt2d_mm,
                              const mitk::Vector2D &vec_mm,
                              mitk::Vector2D &vec_units) const override;

    /**
     * \brief Convert world coordinates (mm) of a 2D vector to continuous index coordinates.
     *
     * \param[in] vec_mm Vector in world coordinates (mm).
     * \param[out] vec_units Vector in continuous index coordinates.
     */
    void WorldToIndex(const mitk::Vector2D &vec_mm, mitk::Vector2D &vec_units) const override;

    /**
     * \brief Determine whether a point is above the curved plane.
     *
     * Applies the inverse transform to obtain parametric coordinates
     * and checks whether the z-component exceeds the parametric bounds.
     *
     * \param[in] pt3d_mm Point in world coordinates (mm).
     * \param[in] considerBoundingBox Unused in this implementation.
     * \return true if the point is above the curved surface.
     */
    bool IsAbove(const Point3D &pt3d_mm, bool considerBoundingBox = false) const override;

    /**
     * \brief Get the parametric extent in mm for a given direction.
     *
     * Delegates to the internal PlaneGeometry's GetExtentInMM().
     *
     * \param[in] direction The axis (0, 1, or 2).
     * \return The parametric extent in mm.
     * \pre m_Plane must not be null.
     */
    virtual mitk::ScalarType GetParametricExtentInMM(int direction) const;

    /**
     * \brief Get the parametric transform (the ITK wrapper around the vtkAbstractTransform).
     *
     * \return Const pointer to the itk::Transform.
     */
    virtual const itk::Transform<mitk::ScalarType, 3, 3> *GetParametricTransform() const;

    /**
     * \brief Multiply the parametric bounds by an oversampling factor.
     *
     * Scales the upper bounds of the parametric bounding box (derived
     * from m_Plane) by the given factor. This change is applied
     * immediately and is not linked to future changes of m_Plane bounds.
     *
     * \param[in] oversampling The oversampling factor.
     * \pre m_Plane must not be null.
     */
    virtual void SetOversampling(mitk::ScalarType oversampling);

    /**
     * \brief Compute the frame geometry (IndexToWorldTransform + bounds) around the curved surface.
     *
     * If no frame geometry has been set explicitly, this derives one
     * from the internal PlaneGeometry's bounding box.
     *
     * \sa SetFrameGeometry
     */
    virtual void CalculateFrameGeometry();

    /**
     * \brief Set the frame geometry explicitly.
     *
     * The frame geometry provides the standard BaseGeometry
     * (IndexToWorldTransform and bounding box) as a hint for the
     * region within which interpolation should occur.
     *
     * \param[in] frameGeometry The frame geometry to use (may be nullptr to clear).
     * \sa CalculateFrameGeometry
     */
    virtual void SetFrameGeometry(const mitk::BaseGeometry *frameGeometry);

    /**
     * \brief Get the parametric bounding box.
     *
     * \return Const pointer to the parametric BoundingBox.
     */
    itkGetConstObjectMacro(ParametricBoundingBox, BoundingBox);

    /**
     * \brief Get the parametric bounds array.
     *
     * \return Reference to the BoundsArrayType [xmin,xmax,ymin,ymax,zmin,zmax]
     *         of the parametric bounding box.
     */
    const BoundingBox::BoundsArrayType &GetParametricBounds() const;

    /**
     * \brief Get the parametric extent in a given direction (in index/unit coordinates).
     *
     * \param[in] direction The axis (0, 1, or 2).
     * \return The parametric extent.
     * \throw mitk::Exception if \a direction is out of range.
     */
    mitk::ScalarType GetParametricExtent(int direction) const;

  protected:
    AbstractTransformGeometry();
    AbstractTransformGeometry(const AbstractTransformGeometry &other);

    ~AbstractTransformGeometry() override;

    //##Documentation
    //## @brief Set the vtkAbstractTransform (stored in m_VtkAbstractTransform)
    //##
    //## Protected in this class, made public in ExternAbstractTransformGeometry.
    virtual void SetVtkAbstractTransform(vtkAbstractTransform *aVtkAbstractTransform);

    //##Documentation
    //## @brief Set the rectangular area that is used for transformation by
    //## m_VtkAbstractTransform and therewith defines the 2D manifold described by
    //## ExternAbstractTransformGeometry
    //##
    //## Protected in this class, made public in ExternAbstractTransformGeometry.
    //## @note The bounds of the PlaneGeometry are used as the parametric bounds.
    //## @note The PlaneGeometry is cloned, @em not linked/referenced.
    virtual void SetPlane(const mitk::PlaneGeometry *aPlane);

    //##Documentation
    //## @brief The rectangular area that is used for transformation by
    //## m_VtkAbstractTransform and therewith defines the 2D manifold described by
    //## AbstractTransformGeometry.
    mitk::PlaneGeometry::Pointer m_Plane;

    itk::VtkAbstractTransform<ScalarType>::Pointer m_ItkVtkAbstractTransform;

    mitk::BaseGeometry::Pointer m_FrameGeometry;

    //##Documentation
    //## @brief Set the parametric bounds
    //##
    //## Protected in this class, made public in some sub-classes, e.g.,
    //## ExternAbstractTransformGeometry.
    virtual void SetParametricBounds(const BoundingBox::BoundsArrayType &bounds);

    mutable mitk::BoundingBox::Pointer m_ParametricBoundingBox;

    //##Documentation
    //## @brief PreSetSpacing
    //##
    //## These virtual function allows a different beahiour in subclasses.
    //## Do implement them in every subclass of BaseGeometry. If not needed, use
    //## {Superclass::PreSetSpacing();};
    void PreSetSpacing(const mitk::Vector3D &aSpacing) override { Superclass::PreSetSpacing(aSpacing); };

    mitkCloneMacro(Self);
  };
} // namespace mitk
#endif
