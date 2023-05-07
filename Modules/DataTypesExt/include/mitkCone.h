/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCone_h
#define mitkCone_h

#include "MitkDataTypesExtExports.h"
#include "mitkBoundingObject.h"

namespace mitk
{
  //##Documentation
  //## @brief Data class containing an cylinder
  //## @ingroup Data
  class MITKDATATYPESEXT_EXPORT Cone : public BoundingObject
  {
  public:
    mitkClassMacro(Cone, BoundingObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      mitk::ScalarType GetVolume() override;
    bool IsInside(const Point3D &p) const override;
    // virtual void UpdateOutputInformation();
  protected:
    Cone();
    ~Cone() override;
  };
}
#endif
