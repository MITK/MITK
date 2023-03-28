/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlane_h
#define mitkPlane_h

#include "MitkDataTypesExtExports.h"
#include "mitkBoundingObject.h"

class vtkPlaneSource;
class vtkDoubleArray;

namespace mitk
{
  /**
   * \brief Plane surface representation
   *
   * Encapsulates vtkPlaneSource.
   *
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT Plane : public BoundingObject
  {
  public:
    mitkClassMacro(Plane, BoundingObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void SetExtent(const double x, const double y);
    void GetExtent(double &x, double &y) const;

    void SetResolution(const int xR, const int yR);
    void GetResolution(int &xR, int &yR) const;

    mitk::ScalarType GetVolume() override;
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
