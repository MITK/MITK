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

#include "mitkNumericConstants.h"
#include "mitkArray.h"
#include "mitkEqual.h"
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
    explicit Vector<TCoordRep, NVectorDimension>()
      : itk::Vector<TCoordRep, NVectorDimension>() {}

    /**
     * @brief Copy constructor.
     */
    explicit Vector<TCoordRep, NVectorDimension>(const mitk::Vector<TCoordRep, NVectorDimension>& r)
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

      for (unsigned int var = 0; (var < NVectorDimension) && (var < vnlVector.size()); ++var) {
        this->SetElement(var, vnlVector.get(var));
      }
    }

    /**
     * @brief Constructor for vnl_vector_fixed.
     */
    Vector<TCoordRep, NVectorDimension>(const vnl_vector_fixed<TCoordRep, NVectorDimension>& vnlVectorFixed)
        : itk::Vector<TCoordRep, NVectorDimension>()
    {
      for (unsigned int var = 0; var < NVectorDimension; ++var) {
        this->SetElement(var,  vnlVectorFixed[var]);
      }
    };

    /**
     * Copies the elements from array array to this.
     * Note that this method will assign doubles to floats without complaining!
     *
     * @param array the array whose values shall be copied. Must overload [] operator.
     */
    template <typename ArrayType >
    void FillVector(const ArrayType& array)
    {
      itk::FixedArray<TCoordRep, NVectorDimension>* thisP = dynamic_cast<itk::FixedArray<TCoordRep, NVectorDimension>* >(this);
      mitk::FillArray<ArrayType, TCoordRep, NVectorDimension>(*thisP, array);
    }

    /**
     * Copies the values stored in this vector into the array array.d
     *
     * @param array the array which should store the values of this.
     */
    template <typename ArrayType >
    void ToArray(ArrayType array) const
    {
      mitk::ToArray<ArrayType, TCoordRep, NVectorDimension>(array, *this);
    }


    /**
     * @brief User defined conversion of mitk::Vector to vnl_vector.
     * Note: the conversion to mitk::Vector to vnl_vector_fixed has not been implemented since this
     * would collide with the conversion vnl_vector to vnl_vector_fixed provided by vnl.
     */
    operator vnl_vector<TCoordRep> () const
    {
      return this->GetVnlVector();
    }


  }; // end mitk::Vector

  // convenience typedefs for often used mitk::Vector representations.

  typedef Vector<ScalarType,2> Vector2D;
  typedef Vector<ScalarType,3> Vector3D;
  typedef Vector<ScalarType,4> Vector4D;

  // other vector types used in MITK
  typedef vnl_vector<ScalarType> VnlVector;

  // The equal methods to compare vectors for equality are below:

  /**
   * @ingroup MITKTestingAPI
   *
   * @param vector1 Vector to compare.
   * @param vector2 Vector to compare.
   * @param eps Tolerance for floating point comparison.
   * @param verbose Flag indicating detailed console output.
   * @return True if vectors are equal.
   */
  template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Vector<TCoordRep, NPointDimension>& vector1, const itk::Vector<TCoordRep, NPointDimension>& vector2, TCoordRep eps=mitk::eps, bool verbose=false)
  {
    bool isEqual = true;
    typename itk::Vector<TCoordRep, NPointDimension>::VectorType diff = vector1-vector2;
    for (unsigned int i=0; i<NPointDimension; i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

    return isEqual;
  }

  /**
   * @ingroup MITKTestingAPI
   *
   * @param vector1 Vector to compare.
   * @param vector2 Vector to compare.
   * @param eps Tolerance for floating point comparison.
   * @param verbose Flag indicating detailed console output.
   * @return True if vectors are equal.
   */
  inline bool Equal(const mitk::VnlVector& vector1, const mitk::VnlVector& vector2, ScalarType eps=mitk::eps, bool verbose=false)
  {
    bool isEqual = true;
    mitk::VnlVector diff = vector1-vector2;
    for (unsigned int i=0; i<diff.size(); i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

    return isEqual;
  }


  /**
   * @ingroup MITKTestingAPI
   *
   * @param vector1 Vector to compare.
   * @param vector2 Vector to compare.
   * @param eps Tolerance for floating point comparison.
   * @param verbose Flag indicating detailed console output.
   * @return True if vectors are equal.
   */
  template <typename TCoordRep, unsigned int NPointDimension>
    inline bool Equal(const vnl_vector_fixed<TCoordRep, NPointDimension> & vector1, const vnl_vector_fixed<TCoordRep, NPointDimension>& vector2, TCoordRep eps=mitk::eps, bool verbose=false)
  {
    vnl_vector_fixed<TCoordRep, NPointDimension> diff = vector1-vector2;
    bool isEqual = true;
    for( unsigned int i=0; i<diff.size(); i++)
    {
      if (DifferenceBiggerOrEqualEps(diff[i], eps))
      {
        isEqual = false;
        break;
      }
    }

    ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

    return isEqual;
  }


} // end namespace mitk

#endif /* MITKVECTOR_H_ */
