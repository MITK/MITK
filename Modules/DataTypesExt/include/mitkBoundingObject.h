/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkBoundingObject_h
#define mitkBoundingObject_h

#include <MitkDataTypesExtExports.h>
#include <mitkSurface.h>

namespace mitk
{
  /**
   * \brief Abstract superclass of all bounding objects (cylinder, cuboid, cone, ellipsoid, etc.).
   *
   * Manages generic bounding-object functions and provides the pure virtual
   * interface IsInside() that concrete subclasses must implement. Each bounding
   * object carries a VTK surface representation and a flag indicating whether
   * it defines a positive or negative volume for CSG operations.
   *
   * The object's actual spatial extent is encoded in its Geometry3D spacing;
   * the index bounds are always [-1, 1] on each axis.
   *
   * \sa Cone, Cuboid, Cylinder, Ellipsoid, Plane, BoundingObjectGroup
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT BoundingObject : public mitk::Surface
  {
  public:
    mitkClassMacro(BoundingObject, mitk::Surface);

    /**
     * \brief Test whether a world-coordinate point lies inside this bounding object.
     *
     * \param[in] p The point in world coordinates to test.
     * \return true if the point is inside the object.
     */
    virtual bool IsInside(const mitk::Point3D &p) const = 0;

    /**
     * \brief Calculate and return the volume of this bounding object.
     *
     * The default implementation returns 0. Subclasses override this to
     * compute the actual volume from the object's geometry extents.
     *
     * \return The volume in world-coordinate units cubed.
     */
    virtual mitk::ScalarType GetVolume();

    /** \brief Get whether this bounding object defines a positive volume. */
    itkGetMacro(Positive, bool);
    /** \brief Set whether this bounding object defines a positive volume. */
    itkSetMacro(Positive, bool);
    /** \brief Toggle the Positive flag. */
    itkBooleanMacro(Positive);

    /**
     * \brief Sets the Geometry3D of the bounding object to fit the given geometry.
     *
     * The fit is performed once. Subsequent changes to the source geometry
     * will \em not be reflected in this bounding object.
     *
     * \param[in] aGeometry3D The geometry whose extents define the new size
     *            and orientation of this bounding object.
     */
    virtual void FitGeometry(BaseGeometry *aGeometry3D);

  protected:
    BoundingObject();
    ~BoundingObject() override;

    //##Documentation
    //## \brief If \a true, the Boundingobject describes a positive volume,
    //## if \a false a negative volume.
    //##
    bool m_Positive;

  private:
    BoundingObject(const BoundingObject &);
    BoundingObject &operator=(const BoundingObject &);
  };
}
#endif
