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
#ifndef mitkVnlVectorFromCvMat_h
#define mitkVnlVectorFromCvMat_h

#include <mitkVnlMatrixFromCvMat.h>
#include <vnl/vnl_vector.h>

namespace mitk
{
  ///
  /// create a vnl_vector from a cv mat
  ///
  template <class T>
  class VnlVectorFromCvMat:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    VnlVectorFromCvMat(
      const cv::Mat* _CvMat,
      vnl_vector<T>* _VnlVector):
        m_VnlMatrixFromCvMat( _CvMat, &m_VnlMatrix ),
        m_VnlVector(_VnlVector)
    {
    }

    ///
    /// executes the Algorithm
    ///
    void Update()
    {
      m_VnlMatrixFromCvMat.Update();

      if( m_VnlMatrix.rows() == 1 )
        *m_VnlVector = m_VnlMatrix.get_row(0);
      else if( m_VnlMatrix.cols() == 1 )
        *m_VnlVector = m_VnlMatrix.get_column(0);

    }
  private:
    ///
    /// VnlVectorFromCvMat input member variable
    ///
    vnl_matrix<T> m_VnlMatrix;
    ///
    /// internal algorithm to get a vnl matrix from a cv mat
    ///
    VnlMatrixFromCvMat<T> m_VnlMatrixFromCvMat;
    ///
    /// VnlVectorFromCvMat output member variable
    ///
    vnl_vector<T>* m_VnlVector;
  };
} // namespace mitk

#endif // mitkVnlVectorFromCvMat_h
