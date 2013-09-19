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
     * Can convert non-identical coordinate representations.
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


    /**
     * @brief Constructor to convert an array to mitk::Vector
     * Can convert non-identical coordinate representations.
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
     * @throws mitk::Exception if vnl_vector.size() != NVectorDimension.
     */
    template <typename TOtherCoordRep>
    Vector<TCoordRep, NVectorDimension>(const vnl_vector<TOtherCoordRep>& vnlVector)
            : itk::Vector<TCoordRep, NVectorDimension>()
    {
      if (vnlVector.size() != NVectorDimension)
        mitkThrow() << "when constructing mitk::Vector from vnl_vector: sizes didn't match: mitk::Vector " << NVectorDimension << "vnl_vector " << vnlVector.size();

      for (int var = 0; (var < NVectorDimension) && (var < vnlVector.size()); ++var) {
        this->SetElement(var, static_cast<TCoordRep>(vnlVector.get(var)));
      }
    }

    /**
     * @brief Constructor for vnl_vecto_fixed.
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


    /**
     * @brief User defined conversion of mitk::Vector to vnl_vector.
     * Note: the conversion to mitk::Vector to vnl_vector_fixed has not been implemented since this
     * would collide with the conversion vnl_vector to vnl_vector_fixed provided by vnl.
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

  }; // end mitk::Vector

  // convenience typedefs for often used mitk::Vector representations.
  typedef Vector<ScalarType,2> Vector2D;
  typedef Vector<ScalarType,3> Vector3D;
  typedef Vector<ScalarType,4> Vector4D;

} // end namespace mitk

#endif /* MITKVECTOR_H_ */
