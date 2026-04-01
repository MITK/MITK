/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMatrix_h
#define mitkMatrix_h

#include <itkMatrix.h>

#include <mitkArray.h>
#include <mitkEqual.h>
#include <mitkNumericConstants.h>

namespace mitk
{
  /** \brief MITK matrix type extending itk::Matrix with additional constructors and utility methods.
   *
   * \tparam T Element type (e.g. double, float).
   * \tparam NRows Number of rows (default: 3).
   * \tparam NColumns Number of columns (default: 3).
   */
  template <class T, unsigned int NRows = 3, unsigned int NColumns = 3>
  class Matrix : public itk::Matrix<T, NRows, NColumns>
  {
  public:
    /** \brief Standard class typedefs. */
    typedef Matrix Self;

    typedef typename itk::Matrix<T, NRows, NColumns>::InternalMatrixType InternalMatrixType;

    /** \brief Default constructor. */
    explicit Matrix() : itk::Matrix<T, NRows, NColumns>() {}
    /** \brief Copy constructor. */
    explicit Matrix(const Matrix &matrix) : itk::Matrix<T, NRows, NColumns>(matrix) {}
    /** \brief Copy constructor for itk compatibility. */
    Matrix(const itk::Matrix<T, NRows, NColumns> &matrix) : itk::Matrix<T, NRows, NColumns>(matrix)
    {
    }
    /** \brief Constructor from vnl_matrix. */
    inline Matrix(const vnl_matrix<T> &matrix) : itk::Matrix<T, NRows, NColumns>(matrix) {}
    /** \brief Constructor from internal matrix type. */
    inline explicit Matrix(InternalMatrixType &matrix) : itk::Matrix<T, NRows, NColumns>(matrix) {}
    /**
     * Necessary because otherwise operator= is default operator= from Matrix.
     */
    using itk::Matrix<T, NRows, NColumns>::operator=;

    /**
     * \brief Copy elements from a 2D array into this matrix.
     *
     * Note that this method will assign doubles to floats without complaining!
     *
     * \param[in] array The array whose values shall be copied. Must overload [][] operator.
     */
    template <typename ArrayType>
    void FillMatrix(const ArrayType &array)
    {
      for (unsigned i = 0; i < NRows; i++)
      {
        for (unsigned j = 0; j < NColumns; j++)
        {
          (*this)[i][j] = array[i][j];
        }
      }
    };

    /**
     * \brief Copy the elements of this matrix into a 2D array.
     *
     * \warning The target matrix must have the same dimensions as this Matrix.
     */
    template <typename MatrixType>
    void ToArray(MatrixType matrix) const
    {
      for (unsigned i = 0; i < NRows; i++)
      {
        for (unsigned j = 0; j < NColumns; j++)
        {
          matrix[i][j] = (*this)[i][j];
        }
      }
    }
  };

  typedef Matrix<ScalarType, 2, 2> Matrix2D;
  typedef Matrix<ScalarType, 3, 3> Matrix3D;
  typedef Matrix<ScalarType, 4, 4> Matrix4D;

  /**
   * \brief Check for matrix equality using root mean squared error (RMS) of all elements.
   *
   * \param[in] matrix1 First vnl matrix.
   * \param[in] matrix2 Second vnl matrix.
   * \param[in] epsilon User-defined accuracy bounds.
   * \return True if the RMS of element differences is below epsilon.
   */
  template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
  inline bool MatrixEqualRMS(const vnl_matrix_fixed<TCoordRep, NRows, NCols> &matrix1,
                             const vnl_matrix_fixed<TCoordRep, NRows, NCols> &matrix2,
                             mitk::ScalarType epsilon = mitk::eps)
  {
    if ((matrix1.rows() == matrix2.rows()) && (matrix1.cols() == matrix2.cols()))
    {
      vnl_matrix_fixed<TCoordRep, NRows, NCols> differenceMatrix = matrix1 - matrix2;
      if (differenceMatrix.rms() < epsilon)
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

  /**
   * \brief Check for matrix equality using root mean squared error (RMS) of all elements.
   *
   * Overload for itk::Matrix types.
   *
   * \param[in] matrix1 First itk matrix.
   * \param[in] matrix2 Second itk matrix.
   * \param[in] epsilon User-defined accuracy bounds.
   * \return True if the RMS of element differences is below epsilon.
   */
  template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
  inline bool MatrixEqualRMS(const itk::Matrix<TCoordRep, NRows, NCols> &matrix1,
                             const itk::Matrix<TCoordRep, NRows, NCols> &matrix2,
                             mitk::ScalarType epsilon = mitk::eps)
  {
    return mitk::MatrixEqualRMS(matrix1.GetVnlMatrix(), matrix2.GetVnlMatrix(), epsilon);
  }

  /**
   * \brief Check for element-wise matrix equality with a user-defined accuracy.
   *
   * \param[in] matrix1 First vnl matrix.
   * \param[in] matrix2 Second vnl matrix.
   * \param[in] epsilon User-defined accuracy bounds.
   * \return True if all element-wise differences are below epsilon.
   */
  template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
  inline bool MatrixEqualElementWise(const vnl_matrix_fixed<TCoordRep, NRows, NCols> &matrix1,
                                     const vnl_matrix_fixed<TCoordRep, NRows, NCols> &matrix2,
                                     mitk::ScalarType epsilon = mitk::eps)
  {
    if ((matrix1.rows() == matrix2.rows()) && (matrix1.cols() == matrix2.cols()))
    {
      for (unsigned int r = 0; r < NRows; r++)
      {
        for (unsigned int c = 0; c < NCols; c++)
        {
          TCoordRep difference = matrix1(r, c) - matrix2(r, c);
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

  /**
   * \brief Check for element-wise matrix equality with a user-defined accuracy.
   *
   * Overload for itk::Matrix types.
   *
   * \param[in] matrix1 First itk matrix.
   * \param[in] matrix2 Second itk matrix.
   * \param[in] epsilon User-defined accuracy bounds.
   * \return True if all element-wise differences are below epsilon.
   */
  template <typename TCoordRep, unsigned int NRows, unsigned int NCols>
  inline bool MatrixEqualElementWise(const itk::Matrix<TCoordRep, NRows, NCols> &matrix1,
                                     const itk::Matrix<TCoordRep, NRows, NCols> &matrix2,
                                     mitk::ScalarType epsilon = mitk::eps)
  {
    return mitk::MatrixEqualElementWise(matrix1.GetVnlMatrix(), matrix2.GetVnlMatrix(), epsilon);
  }
}

#endif
