/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlane_h
#define mitkPlane_h

#include <MitkDataTypesExtExports.h>
#include <mitkBoundingObject.h>

class vtkPlaneSource;
class vtkDoubleArray;

namespace mitk
{
  /**
   * \brief Bounding object representing a flat plane surface.
   *
   * Encapsulates a vtkPlaneSource. Since a plane has no volume, GetVolume()
   * always returns 0 and IsInside() always returns false.
   *
   * \sa BoundingObject
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT Plane : public BoundingObject
  {
  public:
    mitkClassMacro(Plane, BoundingObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the extent (size) of the plane.
     *
     * \param[in] x Width of the plane.
     * \param[in] y Height of the plane.
     */
    void SetExtent(const double x, const double y);

    /**
     * \brief Get the extent (size) of the plane.
     *
     * \param[out] x Width of the plane.
     * \param[out] y Height of the plane.
     */
    void GetExtent(double &x, double &y) const;

    /**
     * \brief Set the tessellation resolution of the plane.
     *
     * \param[in] xR Number of subdivisions along the x-axis.
     * \param[in] yR Number of subdivisions along the y-axis.
     */
    void SetResolution(const int xR, const int yR);

    /**
     * \brief Get the tessellation resolution of the plane.
     *
     * \param[out] xR Number of subdivisions along the x-axis.
     * \param[out] yR Number of subdivisions along the y-axis.
     */
    void GetResolution(int &xR, int &yR) const;

    /**
     * \brief Returns 0, as a plane has no volume.
     * \return Always 0.0.
     */
    mitk::ScalarType GetVolume() override;

    /**
     * \brief Always returns false, as a plane has no interior.
     *
     * \param[in] p The point to test (unused).
     * \return Always false.
     */
    bool IsInside(const Point3D &p) const override;

  protected:
    Plane();
    ~Plane() override;

    vtkPlaneSource *m_PlaneSource;

    vtkPolyData *m_Plane;

    vtkDoubleArray *m_PlaneNormal;
  };
}
#endif
