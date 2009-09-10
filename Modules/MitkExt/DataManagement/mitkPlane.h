/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-13 18:06:46 +0200 (Mi, 13 Mai 2009) $
Version:   $Revision: 17258 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKPLANE_H_HEADER_INCLUDED
#define MITKPLANE_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"


class vtkPlaneSource;

namespace mitk {

/**
 * \brief Plane surface representation
 *
 * Encapsulates vtkPlaneSource.
 *
 * \ingroup Data
 */
class MITKEXT_CORE_EXPORT Plane : public BoundingObject
{
public:
  mitkClassMacro(Plane, BoundingObject);  
  itkNewMacro(Self);

  void SetExtent( const double x, const double y );
  void GetExtent( double &x, double &y ) const;

  void SetResolution( const int xR, const int yR );
  void GetResolution( int &xR, int &yR ) const;

  
  virtual mitk::ScalarType GetVolume();
  virtual bool IsInside(const Point3D& p)  const;

protected:
  Plane();
  virtual ~Plane();

  vtkPlaneSource *m_Plane;

};

}
#endif /* MITKPLANE_H_HEADER_INCLUDED */


