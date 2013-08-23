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

#ifndef MITKVECTOR_H_
#define MITKVECTOR_H_


#include <itkVector.h>

#include "mitkTypeBasics.h"

namespace mitk
{

  template<class TCoordRep, unsigned int NPointDimension = 3>
  class Vector : public itk::Vector<TCoordRep, NPointDimension>
  {
  public:
    /** Default constructor has nothing to do. */
    Vector<TCoordRep, NPointDimension>() {}

    /** Pass-through constructors for the Array base class. */
    Vector<TCoordRep, NPointDimension>(const Vector<TCoordRep, NPointDimension>& r) : itk::Vector<TCoordRep, NPointDimension>(r) {}
    Vector<TCoordRep, NPointDimension>(const TCoordRep r[NPointDimension]):itk::Vector<TCoordRep, NPointDimension>(r) {}
    Vector<TCoordRep, NPointDimension>(const TCoordRep & v):itk::Vector<TCoordRep, NPointDimension>(v) {}
    Vector<TCoordRep, NPointDimension>(const itk::Vector<TCoordRep, NPointDimension> r) : itk::Vector<TCoordRep, NPointDimension>(r) {}
    /**
     * Warning: Array must have same dimension as Point
     */
    void CopyToArray(ScalarType* const array_p) const
    {
      for (int i = 0; i < this->GetPointDimension(); i++)
        {
          array_p[i] = this->GetElement(i);
        }
    }
  };

  typedef Vector<ScalarType,2> Vector2D;
  typedef Vector<ScalarType,3> Vector3D;
  typedef Vector<ScalarType,4> Vector4D;

}

#endif /* MITKVECTOR_H_ */
