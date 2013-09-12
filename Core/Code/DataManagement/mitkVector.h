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
#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_vector.h>

#include "mitkTypeBasics.h"
#include "mitkLogMacros.h"

namespace mitk
{

  template<class TCoordRep, unsigned int NVectorDimension = 3>
  class Vector : public itk::Vector<TCoordRep, NVectorDimension>
  {
  public:
    /** Default constructor has nothing to do. */
    Vector<TCoordRep, NVectorDimension>()
      : itk::Vector<TCoordRep, NVectorDimension>() {}

    /**
     * Constructor to convert from mitk::Vector to mitk::Vector.
     * Can convert nonidentical types.
     * E.g. use this to convert from mitk::Vector<float, 3> to mitk::Vector<double,3>
     */
    template <typename TOtherTypeRepresentation>
    Vector<TCoordRep, NVectorDimension>(const Vector<TOtherTypeRepresentation, NVectorDimension>& r)
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * Constructor to convert from itk::Vector to mitk::Vector.
     * Can convert nonidentical types.
     * E.g. use this to convert from itk::Vector<double, 3> to mitk::Vector<float,3>
     */
    template <typename TOtherTypeRepresentation>
    Vector<TCoordRep, NVectorDimension>(const itk::Vector<TOtherTypeRepresentation, NVectorDimension>& r)
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    Vector<TCoordRep, NVectorDimension>(const TCoordRep r[NVectorDimension])
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * Constructor to initialize entire vector to one value.
     */
    Vector<TCoordRep, NVectorDimension>(const TCoordRep & v)
        : itk::Vector<TCoordRep, NVectorDimension>(v) {}

    /**
     * Attention: vnlVector should have same size as NVectorDimension, otherwise
     * 1. elements will be lost in case vnlVector is bigger.
     * 2. elements will be set to 0 in case this is bigger.
     * In both cases, MITK_WARN warnings will be issued.
     */
    Vector<TCoordRep, NVectorDimension>(const vnl_vector<ScalarType>& vnlVector)
            : itk::Vector<TCoordRep, NVectorDimension>()
    {
      this->Fill(0);

      if (vnlVector.size() < NVectorDimension)
        MITK_WARN << "Attention: conversion of vnl_vector to mitk::Vector:"
                  << "the mitk::Vector has more elements than the vnl_vector. Remaining elements will be set to 0.";

      if (NVectorDimension < vnlVector.size())
        MITK_WARN << "Attention: conversion of vnl_vector to mitk::Vector:"
                  << "the vnl_vector has more elements than the mitk::Vector. vnlVector[NVectorDimension .. vnlVector.size()] will be lost.";

      for (int var = 0; (var < NVectorDimension) && (var < vnlVector.size()); ++var) {
        this->SetElement(var, vnlVector.get(var));
      }
    };

    /**
     * Convert a vnl_vector_fixed to a mitk::Vector of the same size and coordinate representation.
     * TODO SW: allow different coordinate representations.
     */
    Vector<TCoordRep, NVectorDimension>(const vnl_vector_fixed<TCoordRep, NVectorDimension> vnlVectorFixed)
        : itk::Vector<TCoordRep, NVectorDimension>(vnlVectorFixed.data_block()){};

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

    /**
     * User defined conversion of mitk::Vector to vnl_vector_fixed
     */
    operator vnl_vector_fixed<TCoordRep, NVectorDimension>& () const
    {
      vnl_vector_fixed<TCoordRep, NVectorDimension> vnlVectorFixed(this->GetVnlVector());
      return vnlVectorFixed;
    }

  }; // end mitk::Vector

  typedef Vector<ScalarType,2> Vector2D;
  typedef Vector<ScalarType,3> Vector3D;
  typedef Vector<ScalarType,4> Vector4D;

} // end namespace mitk

#endif /* MITKVECTOR_H_ */
