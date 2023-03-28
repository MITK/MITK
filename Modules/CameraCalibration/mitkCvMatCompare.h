/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkCvMatCompare_h
#define mitkCvMatCompare_h

#include <mitkEpsilon.h>
#include <mitkObservable.h>
#include <mitkAlgorithm.h>
#include <mitkEndoDebug.h>

namespace mitk
{
  ///
  /// compare two matrices with a given epsilon
  ///
  class CvMatCompare:
    virtual public Algorithm
  {
  public:
    ///
    /// init default values and save references
    ///
    CvMatCompare(
      const Epsilon* _Epsilon,
      bool* _Equals):
        m_CvMat1(nullptr),
        m_CvMat2(nullptr),
        m_Epsilon(_Epsilon),
        m_Equals(_Equals)
    {
    }
    ///
    /// init default values and save references
    ///
    CvMatCompare(
      const cv::Mat* _CvMat1,
      const cv::Mat* _CvMat2,
      const Epsilon* _Epsilon,
      bool* _Equals):
        m_CvMat1(_CvMat1),
        m_CvMat2(_CvMat2),
        m_Epsilon(_Epsilon),
        m_Equals(_Equals)
    {
    }

    void SetMatrices( const cv::Mat* _CvMat1, const cv::Mat* _CvMat2 )
    {
      m_CvMat1 = _CvMat1;
      m_CvMat2 = _CvMat2;
    }

    void SetMatrix1( const cv::Mat* _CvMat1 )
    {
      m_CvMat1 = _CvMat1;
    }
    void SetMatrix2( const cv::Mat* _CvMat2 )
    {
      m_CvMat2 = _CvMat2;
    }

    ///
    /// executes the Algorithm
    ///
    void Update() override
    {
      assert( m_CvMat1 && m_CvMat2 );
      cv::Mat dst;
      cv::absdiff( (*m_CvMat1), (*m_CvMat2), dst);
      double max = 0;
      cv::minMaxLoc( dst, nullptr, &max );

      endodebugvar( max )

      (*m_Equals) = max <= *m_Epsilon;
    }
  private:
    ///
    /// CvMatCompare input member variable
    ///
    const cv::Mat* m_CvMat1;
    ///
    /// CvMatCompare input member variable
    ///
    const cv::Mat* m_CvMat2;
    ///
    /// CvMatCompare input member variable
    ///
    const Epsilon* m_Epsilon;
    ///
    /// CvMatCompare output member variable
    ///
    bool* m_Equals;
  };
} // namespace mitk

#endif
