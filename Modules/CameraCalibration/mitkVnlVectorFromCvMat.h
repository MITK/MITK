/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
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
    void Update() override
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

#endif
