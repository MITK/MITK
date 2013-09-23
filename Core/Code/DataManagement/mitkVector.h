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
#include "mitkExceptionMacro.h"

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
     */
    Vector<TCoordRep, NVectorDimension>(const Vector<TCoordRep, NVectorDimension>& r)
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * @brief Constructor to convert from itk::Vector to mitk::Vector.
     */
    Vector<TCoordRep, NVectorDimension>(const itk::Vector<TCoordRep, NVectorDimension>& r)
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}


    /**
     * @brief Constructor to convert an array to mitk::Vector
     * @param r the array.
     * @attention must have NVectorDimension valid arguments!
     */
    Vector<TCoordRep, NVectorDimension>(const TCoordRep r[NVectorDimension])
      : itk::Vector<TCoordRep, NVectorDimension>(r) {}

    /**
     * Constructor to initialize entire vector to one value.
     */
    Vector<TCoordRep, NVectorDimension>(const TCoordRep & v)
        : itk::Vector<TCoordRep, NVectorDimension>(v) {}

    /**
     * @brief Constructor for vnl_vectors.
     * @throws mitk::Exception if vnl_vector.size() != NVectorDimension.
     */
    Vector<TCoordRep, NVectorDimension>(const vnl_vector<TCoordRep>& vnlVector)
            : itk::Vector<TCoordRep, NVectorDimension>()
    {
      if (vnlVector.size() != NVectorDimension)
        mitkThrow() << "when constructing mitk::Vector from vnl_vector: sizes didn't match: mitk::Vector " << NVectorDimension << "; vnl_vector " << vnlVector.size();

      for (int var = 0; (var < NVectorDimension) && (var < vnlVector.size()); ++var) {
        this->SetElement(var, vnlVector.get(var));
      }
    }

    /**
     * @brief Constructor for vnl_vector_fixed.
     */
    Vector<TCoordRep, NVectorDimension>(const vnl_vector_fixed<TCoordRep, NVectorDimension> vnlVectorFixed)
        : itk::Vector<TCoordRep, NVectorDimension>()
    {
      for (int var = 0; var < 3; ++var) {
        this->SetElement(var,  vnlVectorFixed[var]);
      }
    };


    /**
     * @brief User defined conversion of mitk::Vector to vnl_vector.
     * Note: the conversion to mitk::Vector to vnl_vector_fixed has not been implemented since this
     * would collide with the conversion vnl_vector to vnl_vector_fixed provided by vnl.
     */
    operator vnl_vector<TCoordRep> () const
    {
      vnl_vector<TCoordRep> vnlVector(this->GetVnlVector());
      return vnlVector;
    }


    /**
     * @brief Copies the elements of this into an ArrayType T
     * @param[out] array    the array which will hold the elements. Must be of a type which overrides the [] operator.
     * @attention array must be of dimension NVectorDimension!
     */
    template <typename ArrayType>
    void ToArray(ArrayType& array) const
    {
      for (int i = 0; i < NVectorDimension; i++)
        {
          array[i] = this->GetElement(i);
        }
    }

    /**
     * @brief Copies elements of an ArrayType T to this Vector
     * @param array     the array whose values will be copied into the Vector. Must be of a type which overrides the [] operator
     * @attention array must be of dimension NVectorDimension!
     */
    template <typename ArrayType>
    void FromArray(const ArrayType& array)
    {
      for (unsigned short int var = 0; var < NVectorDimension; ++var) {
        this->SetElement(var, array[var]);
      }
    }

  }; // end mitk::Vector

  // convenience typedefs for often used mitk::Vector representations.
  typedef Vector<ScalarType,2> Vector2D;
  typedef Vector<ScalarType,3> Vector3D;
  typedef Vector<ScalarType,4> Vector4D;

} // end namespace mitk

#endif /* MITKVECTOR_H_ */
