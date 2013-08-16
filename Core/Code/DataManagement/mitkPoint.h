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


#ifndef MITKPOINT_H
#define MITKPOINT_H



// this is needed for memcopy in ITK
// can be removed when fixed in ITK
#include <cstring>

#include <itkPoint.h>
#include <float.h>
#include <itkIndex.h>
#include <itkContinuousIndex.h>
#include <itkVector.h>
#include <itkMatrix.h>
#include <itkTransform.h>
#include <vnl/vnl_quaternion.h>
#include <MitkExports.h>
#include "mitkDataTypeBasics.h"


namespace mitk {

template<class TCoordRep, unsigned int NPointDimension = 3>
class Point : public itk::Point<TCoordRep, NPointDimension>
{

public:
     /** Default constructor has nothing to do. */
  mitk::Point<TCoordRep, NPointDimension>() {}

  /** Pass-through constructors for the Array base class. */
  mitk::Point<TCoordRep, NPointDimension>(const mitk::Point<TCoordRep, NPointDimension>& r) : itk::Point<TCoordRep, NPointDimension>(r) {}
  mitk::Point<TCoordRep, NPointDimension>(const ValueType r[NPointDimension]):itk::Point<TCoordRep, NPointDimension>(r) {}
  mitk::Point<TCoordRep, NPointDimension>(const ValueType & v):itk::Point<TCoordRep, NPointDimension>(v) {}
  mitk::Point<TCoordRep, NPointDimension>(const itk::Point<TCoordRep, NPointDimension> r) : itk::Point<TCoordRep, NPointDimension>(r) {}

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

typedef mitk::Point<ScalarType,2> Point2D;
typedef mitk::Point<ScalarType,3> Point3D;
typedef mitk::Point<ScalarType,4> Point4D;

typedef mitk::Point<int,2> Point2I;
typedef mitk::Point<int,3> Point3I;
typedef mitk::Point<int,4> Point4I;

} // namespace mitk


#endif /* MITKPOINT_H */
