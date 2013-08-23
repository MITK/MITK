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

  template<class TCoordRep, unsigned int NVectorDimension = 3>
  class Vector : public itk::Vector<TCoordRep, NVectorDimension>
  {
  public:
    /** Default constructor has nothing to do. */
    Vector<TCoordRep, NVectorDimension>() {}

    /** Pass-through constructors for the Array base class. */
    Vector<TCoordRep, NVectorDimension>(const Vector<TCoordRep, NVectorDimension>& r) : itk::Vector<TCoordRep, NVectorDimension>(r) {}
    Vector<TCoordRep, NVectorDimension>(const TCoordRep r[NVectorDimension]):itk::Vector<TCoordRep, NVectorDimension>(r) {}
    Vector<TCoordRep, NVectorDimension>(const TCoordRep & v):itk::Vector<TCoordRep, NVectorDimension>(v) {}
    Vector<TCoordRep, NVectorDimension>(const itk::Vector<TCoordRep, NVectorDimension> r) : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * Warning: Array must have same dimension as Vector
     */
    void CopyToArray(ScalarType array_p[NVectorDimension]) const
    {
      for (int i = 0; i < this->GetVectorDimension(); i++)
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
