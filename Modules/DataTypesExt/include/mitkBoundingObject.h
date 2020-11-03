/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef BOUNDINGOBJECT_H_HEADER_INCLUDED
#define BOUNDINGOBJECT_H_HEADER_INCLUDED

#include "MitkDataTypesExtExports.h"
#include <mitkSurface.h>

namespace mitk
{
  //##Documentation
  //## @brief superclass of all bounding objects (cylinder, cuboid,...)
  //##
  //## Manages generic functions and provides an interface for IsInside()
  //## calculates a generic bounding box
  //## @ingroup Data
  class MITKDATATYPESEXT_EXPORT BoundingObject : public mitk::Surface // BaseData
  {
  public:
    mitkClassMacro(BoundingObject, mitk::Surface);

    virtual bool IsInside(const mitk::Point3D &p) const = 0;

    virtual mitk::ScalarType GetVolume();
    itkGetMacro(Positive, bool);
    itkSetMacro(Positive, bool);
    itkBooleanMacro(Positive);

    //##Documentation
    //## @brief Sets the Geometry3D of the bounding object to fit the given
    //## geometry.
    //##
    //## The fit is done once, so if the given geometry changes it will
    //## \em not effect the bounding object.
    virtual void FitGeometry(BaseGeometry *aGeometry3D);

  protected:
    BoundingObject();
    ~BoundingObject() override;

    bool WriteXMLData(XMLWriter &xmlWriter);

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
#endif /* BOUNDINGOBJECT_H_HEADER_INCLUDED */
