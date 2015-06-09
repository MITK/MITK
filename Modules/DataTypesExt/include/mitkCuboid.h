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


#ifndef MITCUBOID_H_HEADER_INCLUDED
#define MITCUBOID_H_HEADER_INCLUDED

#include "mitkBoundingObject.h"
#include "MitkDataTypesExtExports.h"

namespace mitk {

//##Documentation
//## @brief Data class containing an cuboid
//## @ingroup Data
class MITKDATATYPESEXT_EXPORT Cuboid : public BoundingObject
{
public:
  mitkClassMacro(Cuboid, BoundingObject);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual mitk::ScalarType GetVolume() override;
  virtual bool IsInside(const Point3D& p)  const override;

protected:
  Cuboid();
  virtual ~Cuboid();
};

}
#endif /* MITCUBOID_H_HEADER_INCLUDED */
