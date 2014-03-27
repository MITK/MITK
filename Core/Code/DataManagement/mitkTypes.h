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


#ifndef MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD
#define MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD


#include <mitkLogMacros.h>

#include <itkPoint.h>
#include <float.h>
#include <itkIndex.h>
#include <itkContinuousIndex.h>
#include <itkVector.h>
#include <itkMatrix.h>
#include <itkTransform.h>
#include <vnl/vnl_quaternion.h>
#include <iomanip>

#include "mitkTypeBasics.h"
#include "mitkTypedefs.h"
#include "mitkPoint.h" // TODO SW: should not be included here, maybe generate one "general datatype include" like mitkPrimitives.h
#include "mitkVector.h"
#include "mitkMatrix.h"

#include "mitkOldTypeConversions.h"


#ifndef DOXYGEN_SKIP

namespace mitk {


template <class T, unsigned int NVectorDimension>
  itk::Vector<T, NVectorDimension> operator+(const itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  itk::Vector<T, NVectorDimension> sub;
  for( unsigned int i=0; i<NVectorDimension; i++)
  {
    sub[i] = vector[i]+point[i];
  }
  return sub;
}

template <class T, unsigned int NVectorDimension>
  inline itk::Vector<T, NVectorDimension>& operator+=(itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  for( unsigned int i=0; i<NVectorDimension; i++)
  {
    vector[i] += point[i];
  }
  return vector;
}

template <class T, unsigned int NVectorDimension>
  itk::Vector<T, NVectorDimension> operator-(const itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  itk::Vector<T, NVectorDimension> sub;
  for( unsigned int i=0; i<NVectorDimension; i++)
  {
    sub[i] = vector[i]-point[i];
  }
  return sub;
}

template <class T, unsigned int NVectorDimension>
  inline itk::Vector<T, NVectorDimension>& operator-=(itk::Vector<T, NVectorDimension> &vector, const itk::Point<T, NVectorDimension> &point)
{
  for( unsigned int i=0; i<NVectorDimension; i++)
  {
    vector[i] -= point[i];
  }
  return vector;
}

/**
 * Helper method to check if the difference is bigger than a given epsilon
 *
 * @param diff the difference to be checked against the epsilon
 * @param the epsilon. The absolute difference needs to be smaller than this.
 * @return true if abs(diff) > eps
 */
template <typename DifferenceType>
static inline bool DifferenceBiggerEps(DifferenceType diff, mitk::ScalarType epsilon = mitk::eps)
{
  return fabs(diff) > epsilon;
}

/**
 * outputs elem1, elem2 and eps in case verbose and !isEqual.
 * Elem can e.g. be a mitk::Vector or an mitk::Point.
 *
 * @param elem1 first element to be output
 * @param elem2 second
 * @param eps the epsilon which their difference was bigger than
 * @param verbose tells the function if something shall be output
 * @param isEqual function will only output something if the two elements are not equal
 */
template <typename ElementToOutput1, typename ElementToOutput2>
static inline void ConditionalOutputOfDifference(ElementToOutput1 elem1, ElementToOutput2 elem2, mitk::ScalarType eps, bool verbose, bool isEqual)
{
  if(verbose && !isEqual)
  {
    MITK_INFO << typeid(ElementToOutput1).name() << " and " << typeid(ElementToOutput2).name() << " not equal. Lefthandside " << std::setprecision(12) << elem1 << " - Righthandside " << elem2 << " - epsilon " << eps;
  }
}

/*!
\brief Check for matrix equality with a user defined accuracy. As an equality metric the root mean squared error (RMS) of all elements is calculated.
\param matrix1 first vnl matrix
\param matrix2 second vnl matrix
\param epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualRMS(const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix1,const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  if ( (matrix1.rows() == matrix2.rows()) && (matrix1.cols() == matrix2.cols()) )
  {
    vnl_matrix_fixed<TCoordRep,NRows,NCols> differenceMatrix = matrix1-matrix2;
    if (differenceMatrix.rms()<epsilon)
    {
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

/*!
\brief Check for matrix equality with a user defined accuracy. As an equality metric the root mean squared error (RMS) of all elements is calculated.
\param matrix1 first itk matrix
\param matrix2 second itk matrix
\param epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualRMS(const itk::Matrix<TCoordRep, NRows, NCols>& matrix1,const itk::Matrix<TCoordRep, NRows, NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  return mitk::MatrixEqualRMS(matrix1.GetVnlMatrix(),matrix2.GetVnlMatrix(),epsilon);
}

/*!
\brief Check for element-wise matrix equality with a user defined accuracy.
\param matrix1 first vnl matrix
\param matrix2 second vnl matrix
\param epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualElementWise(const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix1,const vnl_matrix_fixed<TCoordRep,NRows,NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  if ( (matrix1.rows() == matrix2.rows()) && (matrix1.cols() == matrix2.cols()) )
  {
    for( unsigned int r=0; r<NRows; r++)
    {
      for( unsigned int c=0; c<NCols; c++ )
      {
        TCoordRep difference =  matrix1(r,c)-matrix2(r,c);
        if (DifferenceBiggerEps(difference, epsilon))
        {
          return false;
        }
      }
    }
    return true;
  }
  else
  {
    return false;
  }
}

/*!
\brief Check for element-wise matrix equality with a user defined accuracy.
\param matrix1 first itk matrix
\param matrix2 second itk matrix
\param epsilon user defined accuracy bounds
*/
template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
inline bool MatrixEqualElementWise(const itk::Matrix<TCoordRep, NRows, NCols>& matrix1,const itk::Matrix<TCoordRep, NRows, NCols>& matrix2,mitk::ScalarType epsilon=mitk::eps)
{
  return mitk::MatrixEqualElementWise(matrix1.GetVnlMatrix(),matrix2.GetVnlMatrix(),epsilon);
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
inline bool Equal(const itk::Vector<TCoordRep, NPointDimension>& vector1, const itk::Vector<TCoordRep, NPointDimension>& vector2, TCoordRep eps=mitk::eps, bool verbose=false)
{
  bool isEqual = true;
  typename itk::Vector<TCoordRep, NPointDimension>::VectorType diff = vector1-vector2;
  for (unsigned int i=0; i<NPointDimension; i++)
  {
    if (DifferenceBiggerEps(diff[i], eps))
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
 * @param point1 Point to compare.
 * @param point2 Point to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if points are equal.
 */
template <typename TCoordRep, unsigned int NPointDimension>
  inline bool Equal(const itk::Point<TCoordRep, NPointDimension>& point1, const itk::Point<TCoordRep, NPointDimension>& point2, TCoordRep eps=mitk::eps, bool verbose=false)
{
  bool isEqual = true;
  typename itk::Point<TCoordRep, NPointDimension>::VectorType diff = point1-point2;
  for (unsigned int i=0; i<NPointDimension; i++)
  {
    if (DifferenceBiggerEps(diff[i], eps))
    {
      isEqual = false;
      break;
    }
  }

  ConditionalOutputOfDifference(point1, point2, eps, verbose, isEqual);

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
    if (DifferenceBiggerEps(diff[i], eps))
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
 * @param scalar1 Scalar value to compare.
 * @param scalar2 Scalar value to compare.
 * @param eps Tolerance for floating point comparison.
 * @param verbose Flag indicating detailed console output.
 * @return True if scalars are equal.
 */
inline bool Equal(ScalarType scalar1, ScalarType scalar2, ScalarType eps=mitk::eps, bool verbose=false)
{
  bool isEqual( !DifferenceBiggerEps(scalar1-scalar2, eps));

  ConditionalOutputOfDifference(scalar1, scalar2, eps, verbose, isEqual);

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
    if (DifferenceBiggerEps(diff[i], eps))
    {
      isEqual = false;
      break;
    }
  }

  ConditionalOutputOfDifference(vector1, vector2, eps, verbose, isEqual);

  return isEqual;
}

/**
 * Compares two ArrayTypes of size size.
 * ArrayTypes are all Objects/Types who have a [] operator. Pay attention not to set size higher
 * than the actual size of the ArrayType. For POD arrays like float array[4] this will memory corruption.
 */
template <typename TArrayType1, typename TArrayType2>
  inline bool EqualArray(TArrayType1& arrayType1, TArrayType2& arrayType2, int size, ScalarType eps = mitk::eps, bool verbose = false)
{
  bool isEqual = true;
  for (int var = 0; var < size; ++var)
  {
    isEqual = isEqual && Equal(arrayType1[var], arrayType2[var], eps);
  }

  ConditionalOutputOfDifference(arrayType1, arrayType2, eps, verbose, isEqual);

  return isEqual;
}





} // namespace mitk

#endif //DOXYGEN_SKIP

/*
 * This part of the code has been shifted here to avoid compiler clashes
 * caused by including <itkAffineGeometryFrame.h> before the declaration of
 * the Equal() methods above. This problem occurs when using MSVC and is
 * probably related to a compiler bug.
 */

#include <itkAffineGeometryFrame.h>

namespace mitk
{
  typedef itk::AffineGeometryFrame<ScalarType, 3>::TransformType AffineTransform3D;
}


#define mitkSetConstReferenceMacro(name,type) \
  virtual void Set##name (const type & _arg) \
  { \
    itkDebugMacro("setting " << #name " to " << _arg ); \
    if (this->m_##name != _arg) \
      { \
      this->m_##name = _arg; \
      this->Modified(); \
      } \
  }

#define mitkSetVectorMacro(name,type) \
  mitkSetConstReferenceMacro(name,type)

#define mitkGetVectorMacro(name,type) \
  itkGetConstReferenceMacro(name,type)

#endif /* MITKVECTOR_H_HEADER_INCLUDED_C1EBD0AD */
