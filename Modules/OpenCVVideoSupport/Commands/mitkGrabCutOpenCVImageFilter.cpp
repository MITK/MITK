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

#include "mitkGrabCutOpenCVImageFilter.h"

#include "mitkPointSet.h"

mitk::GrabCutOpenCVImageFilter::GrabCutOpenCVImageFilter()
  : m_PointSetsChanged(false)
{
}

mitk::GrabCutOpenCVImageFilter::~GrabCutOpenCVImageFilter()
{
}

bool mitk::GrabCutOpenCVImageFilter::FilterImage( cv::Mat& image )
{
  if ( m_ForegroundPoints.empty() )
  {
    MITK_WARN("GrabCutOpenCVImageFilter")("AbstractOpenCVImageFilter")
        << "Cannot filter image without foreground points set.";
    return false;
  }

  this->UpdateMaskFromPointSets(image.size());

  cv::Mat mask = m_GrabCutMask.clone();

  cv::Mat bgdModel, fgdModel;
  cv::grabCut(image, mask, cv::Rect(), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);

  cv::compare(mask, cv::GC_PR_FGD, m_ResultMask, cv::CMP_EQ);

  cv::Mat foregroundMat;
  cv::compare(mask, cv::GC_FGD, foregroundMat, cv::CMP_EQ);

  foregroundMat.copyTo(m_ResultMask, foregroundMat);

  return true;
}

void mitk::GrabCutOpenCVImageFilter::ClearPoints()
{
  if ( ! m_ForegroundPoints.empty() || ! m_BackgroundPoints.empty() )
  {
    m_ForegroundPoints.clear();
    m_BackgroundPoints.clear();
    m_PointSetsChanged = true;
  }
}

void mitk::GrabCutOpenCVImageFilter::AddForegroundPoints(std::vector<itk::Index<2> > foregroundPoints)
{
  m_ForegroundPoints.insert(m_ForegroundPoints.end(), foregroundPoints.begin(), foregroundPoints.end());

  m_PointSetsChanged = true;
}

void mitk::GrabCutOpenCVImageFilter::AddBackgroundPoints(std::vector<itk::Index<2> > backgroundPoints)
{
  m_BackgroundPoints.insert(m_BackgroundPoints.end(), backgroundPoints.begin(), backgroundPoints.end());

  m_PointSetsChanged = true;
}

const cv::Mat& mitk::GrabCutOpenCVImageFilter::GetResultMask()
{
  return m_ResultMask;
}

void mitk::GrabCutOpenCVImageFilter::UpdateMaskFromPointSets(cv::Size imageSize)
{
  if ( m_PointSetsChanged )
  {
    m_GrabCutMask = cv::Mat(imageSize.height, imageSize.width, CV_8UC1, cv::GC_PR_BGD);

    this->UpdateMaskWithPointSet(m_ForegroundPoints, cv::GC_FGD);
    this->UpdateMaskWithPointSet(m_BackgroundPoints, cv::GC_BGD);

    m_PointSetsChanged = false;
  }
}

void mitk::GrabCutOpenCVImageFilter::UpdateMaskWithPointSet(std::vector<itk::Index<2> > pointSet, int pixelValue)
{
  for (std::vector<itk::Index<2> >::iterator it = pointSet.begin();
       it != pointSet.end(); ++it)
  {
    for ( int i = -2; i <= 2; ++i )
    {
      for ( int j = -2; j <= 2; ++j)
      {
        int x = it->GetElement(1) + i; int y = it->GetElement(0) + j;
        if ( x >= 0 && y >= 0 && x < m_GrabCutMask.cols && y < m_GrabCutMask.rows)
        {
          m_GrabCutMask.at<unsigned char>(x, y) = pixelValue;
        }
      }
    }
  }
}
