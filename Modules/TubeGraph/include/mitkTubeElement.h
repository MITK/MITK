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

#ifndef _MITK_TubeElement_H
#define _MITK_TubeElement_H

#include <MitkTubeGraphExports.h>

#include <mitkPoint.h>

namespace mitk {
  /**
  * \brief Abstract class for elements which describes tubular structur.
  */
  class MITKTUBEGRAPH_EXPORT TubeElement {

  public:

    virtual ~TubeElement(){};

    /**
    * Set the 3D position of the element.
    */
    virtual void SetCoordinates(Point3D coordinates) = 0;

    /**
    * Returns the 3D position of the element.
    */
    virtual const Point3D& GetCoordinates() const = 0;

    /**
    * Comparison operation between this object and the given object.
    * @param right The object to compare with.
    * @return true, if the object is the same;false, if not.
    */
    virtual bool operator==(const TubeElement& right) const = 0;

  };//class

}//namespace
#endif
