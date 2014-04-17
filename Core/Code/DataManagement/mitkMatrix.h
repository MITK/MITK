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

#ifndef MITKMATRIX_H_
#define MITKMATRIX_H_

#include <itkMatrix.h>

#include "mitkConstants.h"
#include "mitkArray.h"
#include "mitkEqual.h"

namespace mitk
{

  template< class T, unsigned int NRows = 3, unsigned int NColumns = 3 >
  class Matrix : public itk::Matrix<T, NRows, NColumns>
  {
  public:

    /** Standard class typedefs. */
    typedef Matrix Self;

    typedef typename itk::Matrix<T, NRows, NColumns>::InternalMatrixType InternalMatrixType;

    /** Default constructor. */
    Matrix<T, NRows, NColumns>() :
        itk::Matrix<T, NRows, NColumns>() {}

    /** Copy constructor. */
    Matrix<T, NRows, NColumns>(const Matrix & matrix) :
            itk::Matrix<T, NRows, NColumns>(matrix) {}

    /** Copy constructor for itk compatibility */
    Matrix<T, NRows, NColumns>(const itk::Matrix<T, NRows, NColumns> & matrix) :
            itk::Matrix<T, NRows, NColumns>(matrix) {}

    /**For every operator=, there should be an equivalent copy constructor. */
    inline Matrix<T, NRows, NColumns>(const vnl_matrix< T > & matrix) :
        itk::Matrix<T, NRows, NColumns>(matrix) {}

    /**For every operator=, there should be an equivalent copy constructor. */
    inline explicit Matrix<T, NRows, NColumns>(InternalMatrixType & matrix) :
        itk::Matrix<T, NRows, NColumns>(matrix) {}

    /**
     * Necessary because otherwise operator= is default operator= from Matrix.
     */
    using itk::Matrix<T, NRows, NColumns>::operator=;



    /**
     * Warning: Array must have same dimension as Matrix
     */
    void CopyToArray(ScalarType const array_p[NRows][NColumns]) const
    {
      for (int i = 0; i < this->GetPointDimension(); i++)
      {
        array_p[i] = this->GetElement(i);
      }
    }
  };

  typedef Matrix<ScalarType,2,2> Matrix2D;
  typedef Matrix<ScalarType,3,3> Matrix3D;
  typedef Matrix<ScalarType,4,4> Matrix4D;



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

}


#endif /* MITKMATRIX_H_ */
