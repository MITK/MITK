/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKELLIPSEDATA_H_HEADER_INCLUDED
#define MITKELLIPSEDATA_H_HEADER_INCLUDED

#include "MitkDataTypesExtExports.h"
#include "mitkBoundingObject.h"

namespace mitk
{
  //##Documentation
  //## @brief Data class containing an ellipsoid
  //## @ingroup Data
  class MITKDATATYPESEXT_EXPORT Ellipsoid : public BoundingObject
  {
  public:
    mitkClassMacro(Ellipsoid, BoundingObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      mitk::ScalarType GetVolume() override;
    bool IsInside(const Point3D &p) const override;

  protected:
    Ellipsoid();
    ~Ellipsoid() override;
  };
}
#endif /* MITKELLIPSEDATA_H_HEADER_INCLUDED */
