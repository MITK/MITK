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
#ifndef mitkCvMatFromVnlVector_h
#define mitkCvMatFromVnlVector_h

#include <mitkEndoMacros.h>
#include <vnl/vnl_vector.h>
#include <mitkCvMatFromVnlMatrix.h>

namespace mitk
{
  ///
  /// casts a random cv::Mat to a vnl vector
  /// if the columns of the first row are > 1
  /// then the vector is filled with the values from this row
  /// otherwise the first elements of each row are used
  /// to fill the vector
  ///
  template <class T>
  class CvMatFromVnlVector:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    CvMatFromVnlVector(
      const vnl_vector<T>* _VnlVector,
      cv::Mat* _CvMat):
        m_CvMatFromVnlMatrix(&m_VnlMatrix, _CvMat),
        m_VnlVector(_VnlVector)
    {
    }

    ///
    /// executes the Algorithm
    ///
    void Update()
    {
      m_VnlMatrix.set_size( m_VnlVector->size(), 1 );
      m_VnlMatrix.set_column(0, *m_VnlVector);
      m_CvMatFromVnlMatrix.Update();
    }
  private:
    ///
    /// CvMatFromVnlVector input member variable
    ///
    vnl_matrix<T> m_VnlMatrix;
    ///
    /// internal algorithm to create a vnl matrix from a random cv mat
    ///
    mitk::CvMatFromVnlMatrix<T> m_CvMatFromVnlMatrix;
    ///
    /// CvMatFromVnlVector input member variable
    ///
    const vnl_vector<T>* m_VnlVector;
  };
} // namespace mitk

#endif // mitkCvMatFromVnlVector_h
