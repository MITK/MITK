/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkTubeElement_h
#define mitkTubeElement_h

#include <MitkTubeGraphExports.h>

#include <mitkPoint.h>

namespace mitk
{
  /**
  * \brief Abstract class for elements which describes tubular structur.
  */
  class MITKTUBEGRAPH_EXPORT TubeElement
  {
  public:
    virtual ~TubeElement(){};

    /**
    * Set the 3D position of the element.
    */
    virtual void SetCoordinates(Point3D coordinates) = 0;

    /**
    * Returns the 3D position of the element.
    */
    virtual const Point3D &GetCoordinates() const = 0;

    /**
    * Comparison operation between this object and the given object.
    * @param right The object to compare with.
    * @return true, if the object is the same;false, if not.
    */
    virtual bool operator==(const TubeElement &right) const = 0;

  }; // class

} // namespace
#endif
