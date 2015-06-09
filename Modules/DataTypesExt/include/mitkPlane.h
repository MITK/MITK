/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


#ifndef MITKPLANE_H_HEADER_INCLUDED
#define MITKPLANE_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
#include "MitkDataTypesExtExports.h"


class vtkPlaneSource;
class vtkDoubleArray;

namespace mitk {

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
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  void SetExtent( const double x, const double y );
  void GetExtent( double &x, double &y ) const;

  void SetResolution( const int xR, const int yR );
  void GetResolution( int &xR, int &yR ) const;


  virtual mitk::ScalarType GetVolume() override;
  virtual bool IsInside(const Point3D& p)  const override;

protected:
  Plane();
  virtual ~Plane();

  vtkPlaneSource *m_PlaneSource;

  vtkPolyData *m_Plane;

  vtkDoubleArray *m_PlaneNormal;


};

}
#endif /* MITKPLANE_H_HEADER_INCLUDED */
