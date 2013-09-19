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
    /**
     * @brief Default constructor has nothing to do.
     */
    Vector<TCoordRep, NVectorDimension>()
      : itk::Vector<TCoordRep, NVectorDimension>() {}

    /**
     * @brief Copy constructor.
     * Can convert nonidentical coordinate representations.
     * E.g. use this to convert from mitk::Vector<float, 3> to mitk::Vector<double,3>
     */
    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>(const Vector<TOtherCoordRep, NVectorDimension>& r)
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * @brief Constructor to convert from itk::Vector to mitk::Vector.
     * Can convert non-identical coordinate representations.
     * E.g. use this to convert from itk::Vector<double, 3> to mitk::Vector<float,3>
     */
    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>(const itk::Vector<TOtherCoordRep, NVectorDimension>& r)
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

//    /**
//     * @brief convert an array of the same data type to Vector.
//     * @param r the array. Attention: must have NVectorDimension valid arguments!
//     */
//    Vector<TCoordRep, NVectorDimension>(const TCoordRep r[NVectorDimension])
//      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * @brief convert an array of a different data type to Vector
     * @param r the array.
     * @attention must have NVectorDimension valid arguments!
     */
    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>(const TOtherCoordRep r[NVectorDimension])
      : itk::Vector<TCoordRep, NVectorDimension>()
    { // TODO SW: very strange: you could leave the following lines and gcc would still copy the array!
      for (int var = 0; var < 3; ++var) {
        this->SetElement(var, static_cast<TCoordRep>(r[var]));
      }
    }

    /**
     * Constructor to initialize entire vector to one value.
     */
    Vector<TCoordRep, NVectorDimension>(const TCoordRep & v)
        : itk::Vector<TCoordRep, NVectorDimension>(v) {}

    /**
     * @brief Constructor for vnl_vectors.
     * Can convert non-identical coordinate representations.
     * @attention vnlVector should have same size as NVectorDimension, otherwise
     * @attention 1. elements will be lost in case vnlVector is bigger.
     * @attention 2. elements will be set to 0 in case this is bigger.
     * @attention In both cases, MITK_WARN warnings will be issued.
     */
    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>(const vnl_vector<TOtherCoordRep>& vnlVector)
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
        this->SetElement(var, static_cast<TCoordRep>(vnlVector.get(var)));
      }
    }

    template <unsigned int NOtherVectorDimension>
    Vector<TCoordRep, NVectorDimension>& operator=(const Vector<TCoordRep, NOtherVectorDimension>& otherVector)
    {
      for (int var = 0; (var < NVectorDimension) && (var < NOtherVectorDimension); ++var) {
        this->SetElement(var, static_cast<TCoordRep>(otherVector[var]));
      }
      return *this;
    }


    /**
     * @brief Convert a vnl_vector_fixed to a mitk::Vector of the same size.
     * Can convert non-identical coordinate representations.
     */
    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>(const vnl_vector_fixed<TOtherCoordRep, NVectorDimension> vnlVectorFixed)
        : itk::Vector<TCoordRep, NVectorDimension>()
    {
      for (int var = 0; var < 3; ++var) {
        this->SetElement(var, static_cast<TOtherCoordRep>(vnlVectorFixed[var]));
      }
    };

    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>& operator=(const vnl_vector_fixed<TOtherCoordRep, NVectorDimension>& vnlVectorFixed)
    {
      for (int var = 0; (var < NVectorDimension); ++var) {
        this->SetElement(var, static_cast<TCoordRep>(vnlVectorFixed[var]));
      }
      return *this;
    }

//    /**
//     * @brief User defined conversion of mitk::Vector to vnl_vector_fixed
//     * mustn't implement when vnl_vector is implemented due to implicit conversion of mitk::Vector3D.
//     */
//    template <typename TOtherCoordRep>
//    operator vnl_vector_fixed<TOtherCoordRep, NVectorDimension> () const
//    {
//      Vector<TOtherCoordRep, NVectorDimension> convertedVector = *this;
//      vnl_vector_fixed<TOtherCoordRep, NVectorDimension> vnlVectorFixed(convertedVector.GetVnlVector());
//      return vnlVectorFixed;
//    }

    /**
     * @brief User defined conversion of mitk::Vector to vnl_vector
     */
    template <typename TOtherCoordRep>
    operator vnl_vector<TOtherCoordRep> () const
    {
      Vector<TOtherCoordRep, NVectorDimension> convertedVector = *this;
      vnl_vector<TOtherCoordRep> vnlVector(convertedVector.GetVnlVector());
      return vnlVector;
    }


    /**
     * @brief Copies the elements of this into the array r
     * @param[out] r    the array which will hold the elements.
     * @attention Must be of at least dimension NVectorDimension!
     */
    template <typename TOtherCoordRep>
    void CopyToArray(TOtherCoordRep r[NVectorDimension]) const
    {
      for (int i = 0; i < this->GetVectorDimension(); i++)
        {
          r[i] = static_cast<TOtherCoordRep>(this->GetElement(i));
        }
    }

  private:



  }; // end mitk::Vector

  typedef Vector<ScalarType,2> Vector2D;
  typedef Vector<ScalarType,3> Vector3D;
  typedef Vector<ScalarType,4> Vector4D;

} // end namespace mitk

#endif /* MITKVECTOR_H_ */
