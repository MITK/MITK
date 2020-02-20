/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkStringFromCvMat_h
#define mitkStringFromCvMat_h

#include <string>
#include <mitkObservable.h>
#include <mitkAlgorithm.h>
#include <mitkEndoMacros.h>
#include <sstream>

#include "opencv2/core.hpp"

namespace mitk
{
  ///
  /// create a vnl_matrix from a cv mat
  ///
  class StringFromCvMat:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    StringFromCvMat(
      const cv::Mat* _CvMat,
      std::string* _String):
        m_CvMat(_CvMat),
        m_String(_String)
    {
    }


    ///
    /// cv mat to vnl matrix with known cv type
    ///
    template <typename TCvMat, char T_Delim>
    void ToString( std::string& string, const cv::Mat& mat )
    {
      std::ostringstream s;


      for(int i=0; i<mat.rows; ++i)
      {
        for(int j=0; j<mat.cols; ++j)
        {
          s << mat.at<TCvMat>(i,j) << " ";
        }
        s << T_Delim;
      }
      string = s.str();
    }

    void SetMatrix(
      const cv::Mat* _CvMat )
    {
      m_CvMat = _CvMat;
    }

    ///
    /// executes the Algorithm
    ///
    void Update() override
    {
      endoAccessCvMat( ToString, '\n', (*m_String), (*m_CvMat) );
    }
  private:
    ///
    /// StringFromCvMat input member variable
    ///
    const cv::Mat* m_CvMat;
    ///
    /// StringFromCvMat output member variable
    ///
    std::string* m_String;
  };
} // namespace mitk

#endif // mitkStringFromCvMat_h
