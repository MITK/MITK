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

#include "mitkTypeBasics.h"
#include <itkMatrix.h>

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

}


#endif /* MITKMATRIX_H_ */
