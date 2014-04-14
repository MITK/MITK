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

#include "mitkConstants.h"
#include "mitkQuaternion.h"
#include "mitkPoint.h" // TODO SW: should not be included here, maybe generate one "general datatype include" like mitkPrimitives.h
#include "mitkVector.h"
#include "mitkMatrix.h"
#include "mitkEqual.h"

#include "mitkOldTypeConversions.h"


#ifndef DOXYGEN_SKIP

namespace mitk {



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
        if (DifferenceBiggerOrEqualEps(difference, epsilon))
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
    if (DifferenceBiggerOrEqualEps(diff[i], eps))
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
