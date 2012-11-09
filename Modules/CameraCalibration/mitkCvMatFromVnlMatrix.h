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
#ifndef mitkCvMatFromVnlMatrix_h
#define mitkCvMatFromVnlMatrix_h

#include <mitkEndoMacros.h>
#include <mitkEndoDebug.h>
#include <cv.h>
#include <vnl/vnl_matrix.h>
#include <mitkAlgorithm.h>

namespace mitk
{
  ///
  /// casts a random cv::Mat to a vnl Matrix
  /// if the columns of the first row are > 1
  /// then the Matrix is filled with the values from this row
  /// otherwise the first elements of each row are used
  /// to fill the Matrix
  ///
  template <class T>
  class CvMatFromVnlMatrix:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    CvMatFromVnlMatrix(
      const vnl_matrix<T>* _VnlMatrix,
      cv::Mat* _CvMat):
        m_VnlMatrix(_VnlMatrix),
        m_CvMat(_CvMat)
    {
    }

    ///
    /// templated function for multiplexed cv::Mat
    ///
    template <class T_CvType, class T_VnlType>
    static void Cast( const vnl_matrix<T_VnlType>& vnlMat, cv::Mat& cvMat )
    {
      endodebugvar( vnlMat )
      cvMat = cv::Mat(vnlMat.rows(), vnlMat.cols(), cv::DataType<T_CvType>::type);
      for(unsigned int i=0; i<vnlMat.rows(); ++i)
      {
        for(unsigned int j=0; j<vnlMat.cols(); ++j)
        {
          cvMat.at<T_CvType>(i,j) = static_cast<T_CvType>( vnlMat(i,j) );
//          endodebugvar( vnlMat(i,j) )
//          endodebugvar( cvMat.at<T_CvType>(i,j) )
        }
      }
    }

    ///
    /// executes the Algorithm
    ///
    void Update()
    {
      Cast<T, T>( *m_VnlMatrix, *m_CvMat );
    }
  private:
    ///
    /// CvMatFromVnlMatrix input member variable
    ///
    const vnl_matrix<T>* m_VnlMatrix;
    ///
    /// CvMatFromVnlMatrix output member variable
    ///
    cv::Mat* m_CvMat;
  };
} // namespace mitk

#endif // mitkCvMatFromVnlMatrix_h
