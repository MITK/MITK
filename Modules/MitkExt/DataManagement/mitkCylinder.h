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


#ifndef MITKCYLINDER_H_HEADER_INCLUDED
#define MITKCYLINDER_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
#include "MitkExtExports.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an cylinder
//## @ingroup Data
class MitkExt_EXPORT Cylinder : public BoundingObject
{
public:
  mitkClassMacro(Cylinder, BoundingObject);
  itkNewMacro(Self);

  virtual mitk::ScalarType GetVolume();
  virtual bool IsInside(const Point3D& p) const;

protected:
  Cylinder();
  virtual ~Cylinder();
};

}
#endif /* MITKCYLINDER_H_HEADER_INCLUDED */


