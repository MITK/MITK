/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBaseGeometry_h
#define mitkBaseGeometry_h

#include <mitkOperationActor.h>
#include <MitkCoreExports.h>
#include <mitkCommon.h>

#include <itkScalableAffineTransform.h>
#include <mitkNumericTypes.h>
#include <itkBoundingBox.h>
#include <itkIndex.h>
#include <itkQuaternionRigidTransform.h>
#include <mitkAffineTransform3D.h>

#include <mitkGeometryTransformHolder.h>
#include <vtkTransform.h>

class vtkMatrix4x4;
class vtkMatrixToLinearTransform;
class vtkLinearTransform;

namespace mitk
{
  /**
   * \brief Standard 3D bounding box type.
   *
   * Convenience typedef that removes template arguments (3D, ScalarType)
   * from itk::BoundingBox for use throughout MITK.
   */
  typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBox;

  /**
   * \brief Fixed-size array holding a pair of time bound values [min, max] in milliseconds.
   */
  typedef itk::FixedArray<ScalarType, 2> TimeBounds;

  /**
   * \brief Fixed-size array of three ScalarType values (e.g., for spacing or axis components).
   */
  typedef itk::FixedArray<ScalarType, 3> FixedArrayType;

  /**
   * \brief Abstract base class describing the geometry of a data object.
   *
   * BaseGeometry stores the spatial metadata that maps between intrinsic
   * (index / unit) coordinates and world coordinates (in millimeters).
   * It holds:
   * \li An axes-parallel bounding box in intrinsic coordinates (often integer
   *     pixel indices), accessible via GetBoundingBox().
   * \li An affine transform (IndexToWorldTransform) that converts intrinsic
   *     coordinates into the world-coordinate system (floating-point values
   *     in mm), accessible via GetIndexToWorldTransform().
   * \li An origin and spacing that, together with the transform, fully
   *     define the geometry.
   *
   * Coordinate conversion is provided through WorldToIndex() and
   * IndexToWorld().  When integer index coordinates are needed (e.g. for
   * pixel iteration), pass an itk::Index as target to WorldToIndex();
   * otherwise a continuous (floating-point) index is returned.
   *
   * An important sub-class is SlicedGeometry3D, which describes data
   * objects consisting of slices, e.g. objects of type Image.
   *
   * BaseGeometry instances referring to an Image need a slightly
   * different definition of corners -- see SetImageGeometry().  This
   * flag is usually set automatically by Image.
   *
   * Geometries must be initialized in GenerateOutputInformation() of
   * BaseProcess (or CopyInformation / UpdateOutputInformation of
   * BaseData) subclasses.
   *
   * This class is abstract.  The simplest concrete implementation is
   * Geometry3D.
   *
   * \note All spatial values are in millimeters (mm) and all temporal
   *       values in milliseconds (ms) unless stated otherwise.
   *
   * \sa Geometry3D, SlicedGeometry3D, PlaneGeometry, TimeGeometry
   * \ingroup Geometry
   */
  class MITKCORE_EXPORT BaseGeometry : public itk::Object, public OperationActor
  {
  public:
    mitkClassMacroItkParent(BaseGeometry, itk::Object);
    itkCloneMacro(Self);

      // ********************************** TypeDef **********************************

      /** \brief Affine transform type used for index-to-world mapping. */
      typedef GeometryTransformHolder::TransformType TransformType;
    /** \brief 3D bounding box type parameterized with ScalarType. */
    typedef itk::BoundingBox<unsigned long, 3, ScalarType> BoundingBoxType;
    /** \brief Fixed-size array of six ScalarType values representing axis-aligned bounds [xmin,xmax,ymin,ymax,zmin,zmax]. */
    typedef BoundingBoxType::BoundsArrayType BoundsArrayType;
    /** \brief Smart-pointer type for BoundingBoxType. */
    typedef BoundingBoxType::Pointer BoundingBoxPointer;

    // ********************************** Origin, Spacing **********************************

    /**
     * \brief Get the origin of the geometry in world coordinates (mm).
     *
     * For a plane, this is typically the upper-left corner.
     *
     * \return The origin as a 3D point in mm.
     */
    Point3D GetOrigin() const;

    /**
     * \brief Set the origin of the geometry in world coordinates (mm).
     *
     * \param[in] origin The new origin point in mm.
     */
    void SetOrigin(const Point3D &origin);

    /**
     * \brief Get the spacing (voxel size) in mm per unit for each axis.
     *
     * \return A 3D vector with spacing values along each axis.
     */
    mitk::Vector3D GetSpacing() const;

    /**
     * \brief Set the spacing (voxel size) for each axis.
     *
     * Updates the IndexToWorldTransform accordingly so that the
     * matrix columns reflect the new spacing.
     *
     * \param[in] aSpacing The new spacing vector in mm per unit.
     * \param[in] enforceSetSpacing If true, forces the spacing to be set
     *            even if subclass-specific constraints would normally prevent it.
     */
    void SetSpacing(const mitk::Vector3D &aSpacing, bool enforceSetSpacing = false);

    /**
     * \brief Get the origin as a VNL vector.
     *
     * \return The origin as a VnlVector.
     * \sa GetOrigin
     */
    VnlVector GetOriginVnl() const;

    // ********************************** other functions **********************************

    /**
     * \brief Get the DICOM FrameOfReferenceID referring to the world coordinate system.
     * \return The frame of reference identifier.
     */
    itkGetConstMacro(FrameOfReferenceID, unsigned int);
    /**
     * \brief Set the DICOM FrameOfReferenceID referring to the world coordinate system.
     * \param[in] _arg The frame of reference identifier to set.
     */
    itkSetMacro(FrameOfReferenceID, unsigned int);

    /**
     * \brief Get the last-modified timestamp of the IndexToWorldTransform.
     * \return The modification time as an unsigned long.
     */
    itkGetConstMacro(IndexToWorldTransformLastModified, unsigned long);

    /**
     * \brief Overloaded Modified() to support the ModifiedLock mechanism.
     *
     * When a ModifiedLock is active, calls to Modified() are deferred until
     * the lock is released, preventing redundant notifications during
     * multi-step geometry updates.
     *
     * \sa ModifiedLock
     */
    void Modified() const override;

    friend class ModifiedLock;

    /**
     * \brief Check whether this geometry is in a valid state.
     *
     * The base implementation always returns true. Subclasses may override
     * this to implement specific validity checks.
     *
     * \return true if the geometry is valid, false otherwise.
     */
    virtual bool IsValid() const;

    // ********************************** Initialize **********************************

    /**
     * \brief Initialize the geometry to default values.
     *
     * Resets the bounding box to [0,1] in each dimension, sets the
     * IndexToWorldTransform to identity, clears the FrameOfReferenceID,
     * and sets ImageGeometry to false.
     */
    void Initialize();

    /**
     * \brief Copy the essential geometry properties into another instance.
     *
     * Copies bounding box, FrameOfReferenceID, the transform holder, and
     * the ImageGeometry flag from this geometry into \a newGeometry.
     *
     * \param[in,out] newGeometry Target geometry to initialize.
     */
    void InitializeGeometry(Self *newGeometry) const;

    // ********************************** Transformations Set/Get **********************************

    /**
     * \brief Get the affine transform that maps index coordinates to world coordinates.
     *
     * \return Non-const pointer to the IndexToWorldTransform.
     */
    mitk::AffineTransform3D *GetIndexToWorldTransform();

    /**
     * \brief Get the affine transform that maps index coordinates to world coordinates (const).
     *
     * \return Const pointer to the IndexToWorldTransform.
     */
    const mitk::AffineTransform3D *GetIndexToWorldTransform() const;

    /**
     * \brief Set the affine transform for index-to-world coordinate mapping.
     *
     * The spacing stored in the geometry is updated to match the column
     * magnitudes of the new transform's matrix.
     *
     * \param[in] transform The new affine transform.
     */
    void SetIndexToWorldTransform(mitk::AffineTransform3D *transform);

    /**
     * \brief Set the IndexToWorldTransform via a vtkMatrix4x4.
     *
     * The matrix is converted to an ITK affine transform and the spacing
     * is updated accordingly.
     *
     * \param[in] vtkmatrix A 4x4 VTK matrix describing the transform.
     * \sa SetIndexToWorldTransform
     */
    void SetIndexToWorldTransformByVtkMatrix(vtkMatrix4x4 *vtkmatrix);

    /**
     * \brief Set the IndexToWorldTransform while preserving the current spacing.
     *
     * Unlike SetIndexToWorldTransform(), this method keeps the original
     * spacing values unchanged. The transform matrix columns are adjusted
     * to maintain the previous spacing.
     *
     * \param[in] transform The new affine transform.
     */
    void SetIndexToWorldTransformWithoutChangingSpacing(mitk::AffineTransform3D *transform);

    /**
     * \brief Set the IndexToWorldTransform via a vtkMatrix4x4 while preserving spacing.
     *
     * \param[in] vtkmatrix A 4x4 VTK matrix describing the transform.
     * \sa SetIndexToWorldTransform, SetIndexToWorldTransformWithoutChangingSpacing
     */
    void SetIndexToWorldTransformByVtkMatrixWithoutChangingSpacing(vtkMatrix4x4 *vtkmatrix);

    /**
     * \brief Get the IndexToWorldTransform as a 4x4 VTK matrix.
     *
     * \return Pointer to the vtkMatrix4x4.
     */
    vtkMatrix4x4 *GetVtkMatrix();

    /**
     * \brief Get the IndexToWorldTransform as a 4x4 VTK matrix (const).
     *
     * \return Const pointer to the vtkMatrix4x4.
     */
    const vtkMatrix4x4* GetVtkMatrix() const;

    /**
     * \brief Get the IndexToWorldTransform as a vtkLinearTransform.
     *
     * \return Pointer to the vtkLinearTransform representation.
     */
    vtkLinearTransform *GetVtkTransform() const;

    /**
     * \brief Reset the geometry to identity.
     *
     * Sets the IndexToWorldTransform to identity, the spacing to 1.0
     * in all directions, and the origin to (0, 0, 0).
     */
    void SetIdentity();

    // ********************************** Transformations **********************************

    /**
     * \brief Compose the IndexToWorldTransform with another transform.
     *
     * Modifies this geometry so its transform becomes the composition
     * of itself and \a other.
     *
     * \param[in] other The transform to compose with.
     * \param[in] pre If true, \a other is pre-composed (applied first, then self).
     *            If false (default), \a other is post-composed (self first, then other).
     *
     * \note This also updates the internally stored spacing.
     */
    void Compose(const TransformType *other, bool pre = false);

    /**
     * \brief Compose the IndexToWorldTransform with a vtkMatrix4x4.
     *
     * Converts the VTK matrix into an ITK transform and calls
     * Compose(const TransformType*, bool).
     *
     * \param[in] vtkmatrix The 4x4 VTK matrix to compose with.
     * \param[in] pre If true, pre-compose; if false (default), post-compose.
     */
    void Compose(const vtkMatrix4x4 *vtkmatrix, bool pre = false);

    /**
     * \brief Translate the origin by a given vector.
     *
     * No action is taken if the vector is zero.
     *
     * \param[in] vector The translation vector in mm.
     */
    void Translate(const Vector3D &vector);

    /**
     * \brief Execute an affine operation (translate, rotate, scale) on this geometry.
     *
     * Supported operation types include OpMOVE, OpSCALE, OpROTATE,
     * OpRESTOREPLANEPOSITION, and OpAPPLYTRANSFORMMATRIX.
     *
     * \param[in] operation The operation to execute.
     * \sa OperationActor
     */
    void ExecuteOperation(Operation *operation) override;

    /**
     * \brief Convert world coordinates (mm) of a point to continuous index coordinates.
     *
     * \param[in] pt_mm Point in world coordinates (mm).
     * \param[out] pt_units Point in continuous (floating-point) index coordinates.
     *
     * \warning For discrete (integer) index coordinates use the overload
     *          accepting an itk::Index instead.
     *
     * \sa IndexToWorld
     */
    void WorldToIndex(const mitk::Point3D &pt_mm, mitk::Point3D &pt_units) const;

    /**
     * \brief Convert world coordinates (mm) of a vector to continuous index coordinates.
     *
     * \param[in] vec_mm Vector in world coordinates (mm).
     * \param[out] vec_units Vector in continuous index coordinates.
     */
    void WorldToIndex(const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const;

    /**
     * \brief Convert world coordinates (mm) of a point to discrete (integer) index coordinates.
     *
     * The continuous index values are rounded to the nearest integer using
     * half-integer-up rounding.
     *
     * \tparam VIndexDimension Dimension of the ITK index (must be <= 3).
     * \param[in] pt_mm Point in world coordinates (mm).
     * \param[out] index Discrete index coordinates; dimensions beyond 3 are set to 0.
     */
    template <unsigned int VIndexDimension>
    void WorldToIndex(const mitk::Point3D &pt_mm, itk::Index<VIndexDimension> &index) const
    {
      typedef itk::Index<VIndexDimension> IndexType;
      mitk::Point3D pt_units;
      this->WorldToIndex(pt_mm, pt_units);
      int i, dim = index.GetIndexDimension();
      if (dim > 3)
      {
        index.Fill(0);
        dim = 3;
      }
      for (i = 0; i < dim; ++i)
      {
        index[i] = itk::Math::RoundHalfIntegerUp<typename IndexType::IndexValueType>(pt_units[i]);
      }
    }

    /**
     * \brief Convert index coordinates of a vector to world coordinates (mm).
     *
     * \param[in] vec_units Vector in index (unit) coordinates.
     * \param[out] vec_mm Vector in world coordinates (mm).
     */
    void IndexToWorld(const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const;

    /**
     * \brief Convert continuous or discrete index coordinates of a point to world coordinates (mm).
     *
     * \param[in] pt_units Point in index coordinates.
     * \param[out] pt_mm Point in world coordinates (mm).
     */
    void IndexToWorld(const mitk::Point3D &pt_units, mitk::Point3D &pt_mm) const;

    /**
     * \brief Convert discrete (itk::Index) index coordinates of a point to world coordinates (mm).
     *
     * \tparam VIndexDimension Dimension of the ITK index (dimensions beyond 3 are ignored).
     * \param[in] index Discrete index coordinates.
     * \param[out] pt_mm Point in world coordinates (mm).
     */
    template <unsigned int VIndexDimension>
    void IndexToWorld(const itk::Index<VIndexDimension> &index, mitk::Point3D &pt_mm) const
    {
      mitk::Point3D pt_units;
      pt_units.Fill(0);
      int i, dim = index.GetIndexDimension();
      if (dim > 3)
      {
        dim = 3;
      }
      for (i = 0; i < dim; ++i)
      {
        pt_units[i] = index[i];
      }

      IndexToWorld(pt_units, pt_mm);
    }

    /**
     * \brief Convert index coordinates of a vector to world coordinates (mm).
     *
     * \deprecated The first parameter \a atPt3d_units is unused.
     *             Use IndexToWorld(const Vector3D&, Vector3D&) instead.
     *
     * \param[in] atPt3d_units Unused point parameter.
     * \param[in] vec_units Vector in index coordinates.
     * \param[out] vec_mm Vector in world coordinates (mm).
     */
    void IndexToWorld(const mitk::Point3D &atPt3d_units, const mitk::Vector3D &vec_units, mitk::Vector3D &vec_mm) const;

    /**
     * \brief Convert world coordinates (mm) of a vector to continuous index coordinates.
     *
     * \deprecated The first parameter \a atPt3d_mm is unused.
     *             Use WorldToIndex(const Vector3D&, Vector3D&) instead.
     *
     * \param[in] atPt3d_mm Unused point parameter.
     * \param[in] vec_mm Vector in world coordinates (mm).
     * \param[out] vec_units Vector in continuous index coordinates.
     */
    void WorldToIndex(const mitk::Point3D &atPt3d_mm, const mitk::Vector3D &vec_mm, mitk::Vector3D &vec_units) const;

    /**
     * \brief Convert an ITK physical point to MITK world coordinates.
     *
     * \deprecated Since ITK 3.10 this is a no-op identity copy because
     *             ITK physical coordinates and MITK world coordinates
     *             are equivalent.
     *
     * \tparam TCoordRep Coordinate representation type of the ITK point.
     * \param[in] itkPhysicalPoint Point in ITK physical coordinates (mm).
     * \param[out] pt_mm Point in MITK world coordinates (mm).
     *
     * \sa WorldToItkPhysicalPoint
     */
    template <class TCoordRep>
    void ItkPhysicalPointToWorld(const itk::Point<TCoordRep, 3> &itkPhysicalPoint, mitk::Point3D &pt_mm) const
    {
      mitk::vtk2itk(itkPhysicalPoint, pt_mm);
    }

    /**
     * \brief Convert MITK world coordinates to ITK physical coordinates.
     *
     * \deprecated Since ITK 3.10 this is a no-op identity copy because
     *             ITK physical coordinates and MITK world coordinates
     *             are equivalent.
     *
     * Historically, ITK did not support rotated images; only origin and
     * spacing were used.  This method was needed to convert from the full
     * MITK transform (including rotation) to an ITK-compatible coordinate.
     *
     * \tparam TCoordRep Coordinate representation type of the ITK point.
     * \param[in] pt_mm Point in MITK world coordinates (mm).
     * \param[out] itkPhysicalPoint Point in ITK physical coordinates (mm).
     *
     * \sa ItkPhysicalPointToWorld
     */
    template <class TCoordRep>
    void WorldToItkPhysicalPoint(const mitk::Point3D &pt_mm, itk::Point<TCoordRep, 3> &itkPhysicalPoint) const
    {
      mitk::vtk2itk(pt_mm, itkPhysicalPoint);
    }

    // ********************************** BoundingBox **********************************

    /**
     * \brief Get the bounding box in index/unit coordinates.
     *
     * \return Const pointer to the BoundingBoxType.
     */
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);

// a bit of a misuse, but we want only doxygen to see the following:
#ifdef DOXYGEN_SKIP
    /**
     * \brief Get the bounding box in index/unit coordinates.
     *
     * \return Const pointer to the BoundingBoxType.
     */
    itkGetConstObjectMacro(BoundingBox, BoundingBoxType);
    /**
     * \brief Get the bounding box bounds as a BoundsArrayType in index/unit coordinates.
     *
     * \return An array of six ScalarType values [xmin,xmax,ymin,ymax,zmin,zmax].
     */
    const BoundsArrayType GetBounds() const;
#endif
    /**
     * \brief Get the bounding box bounds as a BoundsArrayType in index/unit coordinates.
     *
     * \return An array of six ScalarType values [xmin,xmax,ymin,ymax,zmin,zmax].
     */
    const BoundsArrayType GetBounds() const;

    /**
     * \brief Set the bounding box in index/unit coordinates.
     *
     * A copy of the bounds array is stored, not a reference.
     *
     * \param[in] bounds The bounds array [xmin,xmax,ymin,ymax,zmin,zmax].
     */
    void SetBounds(const BoundsArrayType &bounds);

    /**
     * \brief Set the bounding box via a float array.
     *
     * \param[in] bounds Six floats [xmin,xmax,ymin,ymax,zmin,zmax] in index coordinates.
     */
    void SetFloatBounds(const float bounds[6]);

    /**
     * \brief Set the bounding box via a double array.
     *
     * \param[in] bounds Six doubles [xmin,xmax,ymin,ymax,zmin,zmax] in index coordinates.
     */
    void SetFloatBounds(const double bounds[6]);

    /**
     * \brief Get a column of the IndexToWorldTransform matrix as a VnlVector.
     *
     * The returned vector points along the bounding box in the specified
     * direction and has the length of the spacing in that direction.
     *
     * \param[in] direction The column index (0, 1, or 2).
     * \return VnlVector representing the matrix column.
     * \sa GetAxisVector
     */
    VnlVector GetMatrixColumn(unsigned int direction) const;

    /**
     * \brief Calculate a bounding box relative to a given transform.
     *
     * Transforms all eight corner points of this geometry by the inverse
     * of \a transform and computes an axis-aligned bounding box around
     * the results. If \a transform is nullptr, the corner points in
     * world coordinates are used directly.
     *
     * \param[in] transform The coordinate system transform (may be nullptr).
     * \return Smart pointer to the computed bounding box.
     */
    mitk::BoundingBox::Pointer CalculateBoundingBoxRelativeToTransform(const mitk::AffineTransform3D *transform) const;

//##Documentation
//## @brief Set the time bounds (in ms)
// void SetTimeBounds(const TimeBounds& timebounds);

// ********************************** Geometry **********************************

#ifdef DOXYGEN_SKIP
    /**
     * \brief Get the extent of the bounding box in index/unit coordinates.
     *
     * \param[in] direction The axis (0=x, 1=y, 2=z).
     * \return Extent in units along the specified direction.
     * \sa GetExtentInMM
     */
    ScalarType GetExtent(unsigned int direction) const;
#endif

    /**
     * \brief Get the extent of the bounding box in index/unit coordinates.
     *
     * \param[in] direction The axis (0=x, 1=y, 2=z).
     * \return Extent in units along the specified direction.
     * \throw mitk::Exception if \a direction >= 3.
     * \sa GetExtentInMM
     */
    ScalarType GetExtent(unsigned int direction) const;

    /**
     * \brief Get the extent of the bounding box in mm along the specified direction.
     *
     * This equals the length of the axis vector in that direction:
     * GetAxisVector(direction).GetNorm().
     *
     * \param[in] direction The axis (0=x, 1=y, 2=z).
     * \return Extent in millimeters.
     * \sa GetAxisVector
     */
    ScalarType GetExtentInMM(int direction) const;

    /**
     * \brief Get a vector along the bounding box in the specified direction in mm.
     *
     * The vector's direction is the transform matrix column for that axis,
     * and its length equals the bounding-box extent in that direction (in mm).
     *
     * \param[in] direction The axis (0=x, 1=y, 2=z).
     * \return The axis vector in mm.
     * \sa GetMatrixColumn
     */
    Vector3D GetAxisVector(unsigned int direction) const;

    /**
     * \brief Check if this geometry can be converted to 2D without information loss.
     *
     * When a 2D image is saved, the 3x3 matrix is typically cropped to 2x2.
     * This method checks whether such a reduction would discard geometric
     * information (non-trivial z-spacing, z-origin, or off-axis components).
     *
     * \return true if conversion to 2D is lossless, false otherwise.
     */
    virtual bool Is2DConvertable();

    /**
     * \brief Get the center of the bounding box in world coordinates (mm).
     *
     * For image geometries the center accounts for the half-voxel offset.
     *
     * \return The center point.
     */
    Point3D GetCenter() const;

    /**
     * \brief Get the squared length of the bounding-box diagonal in mm.
     *
     * \return Squared diagonal length.
     * \sa GetDiagonalLength
     */
    double GetDiagonalLength2() const;

    /**
     * \brief Get the length of the bounding-box diagonal in mm.
     *
     * \return Diagonal length.
     * \sa GetDiagonalLength2
     */
    double GetDiagonalLength() const;

    /**
     * \brief Get the position of corner number \a id in world coordinates.
     *
     * Corners are numbered 0--7 corresponding to the eight vertices of
     * the axis-aligned bounding box. For image geometries a half-voxel
     * offset is applied.
     *
     * \param[in] id Corner index (0--7).
     * \return The corner point in mm.
     * \throw itk::ExceptionObject if \a id is out of range.
     * \sa SetImageGeometry
     */
    Point3D GetCornerPoint(int id) const;

    /**
     * \brief Get the position of a corner in world coordinates.
     *
     * \param[in] xFront If true, use the minimum x-bound; otherwise maximum.
     * \param[in] yFront If true, use the minimum y-bound; otherwise maximum.
     * \param[in] zFront If true, use the minimum z-bound; otherwise maximum.
     * \return The corner point in mm.
     * \sa SetImageGeometry
     */
    Point3D GetCornerPoint(bool xFront = true, bool yFront = true, bool zFront = true) const;

    /**
     * \brief Set the extent of the bounding box in mm along the specified direction.
     *
     * This modifies the IndexToWorldTransform matrix by scaling the
     * corresponding column, but does \em not change the bounding-box
     * bounds (which are in index/unit coordinates).
     *
     * \param[in] direction The axis (0=x, 1=y, 2=z).
     * \param[in] extentInMM The new extent in mm.
     */
    void SetExtentInMM(int direction, ScalarType extentInMM);

    /**
     * \brief Test whether a point (world coordinates, mm) lies inside the bounding box.
     *
     * Converts the point to index coordinates and delegates to IsIndexInside().
     *
     * \param[in] p The point in world coordinates.
     * \return true if the point is inside the bounding box.
     */
    bool IsInside(const mitk::Point3D &p) const;

    /**
     * \brief Test whether a point in continuous index coordinates lies inside the bounding box.
     *
     * For image geometries the index is rounded before the check so that
     * discrete pixel semantics apply.
     *
     * \param[in] index The point in continuous index coordinates.
     * \return true if the point is inside.
     */
    bool IsIndexInside(const mitk::Point3D &index) const;

    /**
     * \brief Test whether a discrete ITK index lies inside the bounding box.
     *
     * Convenience overload that converts itk::Index to Point3D and
     * delegates to IsIndexInside(const Point3D&).
     *
     * \tparam VIndexDimension Dimension of the ITK index.
     * \param[in] index The discrete index to test.
     * \return true if the index is inside.
     */
    template <unsigned int VIndexDimension>
    bool IsIndexInside(const itk::Index<VIndexDimension> &index) const
    {
      int i, dim = index.GetIndexDimension();
      Point3D pt_index;
      pt_index.Fill(0);
      for (i = 0; i < dim; ++i)
      {
        pt_index[i] = index[i];
      }
      return IsIndexInside(pt_index);
    }

    /**
     * \brief Clamp a point to lie within the geometry's world-coordinate bounds.
     *
     * If the given point lies outside the geometry's bounds it is projected
     * onto the nearest boundary. If the point is already inside, it is
     * returned unchanged.
     *
     * \param[in] point The point to clamp (world coordinates, mm).
     * \return The clamped point, guaranteed to lie within the bounds.
     */
    Point3D ClampPoint(const Point3D& point) const;

    // ********************************* Image Geometry ********************************

    /**
     * \brief Switch between image geometry and normal geometry with origin correction.
     *
     * Image geometries use center-based voxel coordinates (origin is at
     * voxel center), so switching to or from image geometry requires a
     * half-voxel offset on the origin. This method toggles the
     * ImageGeometry flag and adjusts the origin accordingly.
     *
     * \param[in] isAnImageGeometry true to switch to image geometry,
     *            false to switch to normal geometry.
     */
    virtual void ChangeImageGeometryConsideringOriginOffset(const bool isAnImageGeometry);

    /**
     * \brief Query whether this geometry refers to an image.
     *
     * \return true if this is an image geometry.
     * \sa SetImageGeometry
     */
    itkGetConstMacro(ImageGeometry, bool)

      /**
       * \brief Define that this BaseGeometry refers to an image.
       *
       * An image geometry uses center-based voxel positions: the origin
       * is at the center of the first voxel, and corner points are offset
       * by half a voxel. This flag enables that correction in
       * GetCornerPoint(), GetCenter(), IsIndexInside(), etc.
       *
       * \param[in] _arg true if this geometry refers to an image.
       * \sa GetImageGeometry, ImageGeometryOn, ImageGeometryOff
       */
      itkSetMacro(ImageGeometry, bool);

      /**
       * \brief Toggle macro for ImageGeometry (ImageGeometryOn/ImageGeometryOff).
       */
      itkBooleanMacro(ImageGeometry);

        /**
         * \brief Get the internal GeometryTransformHolder.
         *
         * \return Const pointer to the GeometryTransformHolder.
         */
        const GeometryTransformHolder *GetGeometryTransformHolder() const;

    /**
     * \brief Map geometry axes to anatomical orientations (sagittal, coronal, axial).
     *
     * Determines which geometry axis (0, 1, 2) most closely aligns with
     * each of the three anatomical orientations. The result is stored in
     * the output array where element [0] is the axis index for sagittal,
     * [1] for coronal, and [2] for axial. Each axis is mapped to exactly
     * one orientation.
     *
     * \param[out] axes Array of three ints receiving the axis index for
     *             each orientation (sagittal, coronal, axial).
     */
    void MapAxesToOrientations(int axes[]) const;

  protected:
    // ********************************** Constructor **********************************
    BaseGeometry();
    BaseGeometry(const BaseGeometry &other);
    ~BaseGeometry() override;

    void PrintSelf(std::ostream &os, itk::Indent indent) const override;

    static const std::string GetTransformAsString(TransformType *transformType);

    itkGetConstMacro(NDimensions, unsigned int);

      bool IsBoundingBoxNull() const;

    bool IsIndexToWorldTransformNull() const;

    void SetVtkMatrixDeepCopy(vtkTransform *vtktransform);

    void _SetSpacing(const mitk::Vector3D &aSpacing, bool enforceSetSpacing = false);

    //##Documentation
    //## @brief PreSetSpacing
    //##
    //## These virtual function allows a different beahiour in subclasses.
    //## Do implement them in every subclass of BaseGeometry. If not needed, use
    //## {Superclass::PreSetSpacing();};
    virtual void PreSetSpacing(const mitk::Vector3D & /*aSpacing*/){};

    //##Documentation
    //## @brief CheckBounds
    //##
    //## This function is called in SetBounds. Assertions can be implemented in this function (see PlaneGeometry.cpp).
    //## If you implement this function in a subclass, make sure, that all classes were your class inherits from
    //## have an implementation of CheckBounds
    //## (e.g. inheritance BaseGeometry <- A <- B. Implementation of CheckBounds in class B needs implementation in A as
    // well!)
    virtual void CheckBounds(const BoundsArrayType & /*bounds*/){};

    //##Documentation
    //## @brief CheckIndexToWorldTransform
    //##
    //## This function is called in SetIndexToWorldTransform. Assertions can be implemented in this function (see
    // PlaneGeometry.cpp).
    //## In Subclasses of BaseGeometry, implement own conditions or call Superclass::CheckBounds(bounds);.
    virtual void CheckIndexToWorldTransform(mitk::AffineTransform3D * /*transform*/){};

  private:
    GeometryTransformHolder *m_GeometryTransform;

    void InitializeGeometryTransformHolder(const BaseGeometry *otherGeometry);

    //##Documentation
    //## @brief Bounding Box, which is axes-parallel in intrinsic coordinates
    //## (often integer indices of pixels)
    BoundingBoxPointer m_BoundingBox;

    unsigned int m_FrameOfReferenceID;

    // mitk::TimeBounds m_TimeBounds;

    static const unsigned int m_NDimensions = 3;

    mutable TransformType::Pointer m_InvertedTransform;

    mutable unsigned long m_IndexToWorldTransformLastModified;

    bool m_ImageGeometry;

    //##Documentation
    //## @brief ModifiedLockFlag is used to prohibit the call of Modified()
    //##
    //## For the use of this Flag, see class ModifiedLock. This flag should only be set
    //## by the ModifiedLock class!
    bool m_ModifiedLockFlag;

    //##Documentation
    //## @brief ModifiedcalledFlag is used to collect calls of Modified().
    //##
    //## For the use of this Flag, see class ModifiedLock. This flag should only be set
    //## by the Modified() function!
    mutable bool m_ModifiedCalledFlag;
  };

  // ********************************** Equal Functions **********************************
  //
  // Static compare functions mainly for testing
  //
  /**
  * @brief Equal A function comparing two geometries for being identical.
  *
  * @ingroup MITKTestingAPI
  *
  * The function compares the spacing, origin, axisvectors, extents, the matrix of the
  * IndexToWorldTransform (elementwise), the bounding (elementwise) and the ImageGeometry flag.
  *
  * The parameter eps is a tolarence value for all methods which are internally used for comparison.
  * If you want to use different tolerance values for different parts of the geometry, feel free to use
  * the other comparison methods and write your own implementation of Equal.
  * @param rightHandSide Compare this against leftHandSide.
  * @param leftHandSide Compare this against rightHandSide.
  * @param coordinateEps Tolerance for comparison of all spatial aspects (spacing, origin and grid alignment).
  * You can use mitk::eps in most cases.
  * @param directionEps Tolerance for comparison of all directional aspects (axis). You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparison are true. False in any other case.
  */
  MITKCORE_EXPORT bool Equal(const mitk::BaseGeometry& leftHandSide,
    const mitk::BaseGeometry& rightHandSide,
    ScalarType coordinateEps,
    ScalarType directionEps,
    bool verbose = false);

  /**
  * @brief Equal A function comparing two geometries for being identical.
  *
  * @ingroup MITKTestingAPI
  *
  * This is an overloaded version that uses a single tolerance for spatial and directional aspects. For more details,
  * see the other overloaded version.
  *
  * @param rightHandSide Compare this against leftHandSide.
  * @param leftHandSide Compare this against rightHandSide.
  * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparison are true. False in any other case.
  */
  MITKCORE_EXPORT bool Equal(const mitk::BaseGeometry &leftHandSide,
                             const mitk::BaseGeometry &rightHandSide,
                             ScalarType eps = mitk::eps,
                             bool verbose = false);

  /**
  * @brief Equal A function comparing two transforms (TransformType) for being identical.
  *
  * @ingroup MITKTestingAPI
  *
  * The function compares the IndexToWorldTransform (elementwise).
  *
  * The parameter eps is a tolarence value for all methods which are internally used for comparison.
  * @param rightHandSide Compare this against leftHandSide.
  * @param leftHandSide Compare this against rightHandSide.
  * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparison are true. False in any other case.
  */
  MITKCORE_EXPORT bool Equal(const mitk::BaseGeometry::TransformType &leftHandSide,
                             const mitk::BaseGeometry::TransformType &rightHandSide,
                             ScalarType eps,
                             bool verbose);

  /**
  * @brief Equal A function comparing two bounding boxes (BoundingBoxType) for being identical.
  *
  * @ingroup MITKTestingAPI
  *
  * The function compares the bounds (elementwise).
  *
  * The parameter eps is a tolarence value for all methods which are internally used for comparison.
  * @param rightHandSide Compare this against leftHandSide.
  * @param leftHandSide Compare this against rightHandSide.
  * @param eps Tolarence for comparison. You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparison are true. False in any other case.
  */
  MITKCORE_EXPORT bool Equal(const mitk::BaseGeometry::BoundingBoxType &leftHandSide,
                             const mitk::BaseGeometry::BoundingBoxType &rightHandSide,
                             ScalarType eps,
                             bool verbose);

  /**
  * @brief A function checks if a test geometry is a sub geometry of
  * a given reference geometry.
  *
  * Sub geometry means that both geometries have the same voxel grid (same spacing, same axes,
  * origin is on voxel grid), but the bounding box of the checked geometry is contained or equal
  * to the bounding box of the reference geometry.\n
  * By this definition equal geometries are always sub geometries of each other.
  *
  * The function checks the spacing, origin, axis vectors, extents, the matrix of the
  * IndexToWorldTransform (elementwise), the bounding (elementwise) and the ImageGeometry flag.
  *
  * The parameter eps is a tolerance value for all methods which are internally used for comparison.
  * @param testGeo Geometry that should be checked if it is a sub geometry of referenceGeo.
  * @param referenceGeo Geometry that should contain testedGeometry as sub geometry.
  * @param coordinateEps Tolerance for comparison of all spatial aspects (spacing, origin and grid alignment).
  * You can use mitk::eps in most cases.
  * @param directionEps Tolerance for comparison of all directional aspects (axis). You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparisons are true. False otherwise.
  */
  MITKCORE_EXPORT bool IsSubGeometry(const mitk::BaseGeometry& testGeo,
    const mitk::BaseGeometry& referenceGeo,
    ScalarType coordinateEps,
    ScalarType directionEps,
    bool verbose = false);

  /**
  * @brief A function checks if a test geometry is a sub geometry of
  * a given reference geometry.
  *
  * This is a overloaded version that uses a single tolerance for spatial and directional aspects. For more details,
  * see the other overloaded version.
  *
  * @param testGeo Geometry that should be checked if it is a sub geometry of referenceGeo.
  * @param referenceGeo Geometry that should contain testedGeometry as sub geometry.
  * @param eps Tolarence for comparison (both spatial and directional). You can use mitk::eps in most cases.
  * @param verbose Flag indicating if the user wants detailed console output or not.
  * @return True, if all comparison are true. False otherwise.
  */
  MITKCORE_EXPORT bool IsSubGeometry(const mitk::BaseGeometry& testGeo,
    const mitk::BaseGeometry& referenceGeo,
    ScalarType eps = mitk::eps,
    bool verbose = false);

} // namespace mitk

#endif
