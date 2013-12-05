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

#ifndef MITKGRABCUTOPENCVIMAGEFILTER_H
#define MITKGRABCUTOPENCVIMAGEFILTER_H

#include "mitkAbstractOpenCVImageFilter.h"

//itk headers
#include "itkObjectFactory.h"

#include "cv.h"

namespace itk {
template<unsigned int T> class Index;
}

namespace mitk {

class PointSet;

class MITK_OPENCVVIDEOSUPPORT_EXPORT GrabCutOpenCVImageFilter : public AbstractOpenCVImageFilter
{
public:
  mitkClassMacro(GrabCutOpenCVImageFilter, AbstractOpenCVImageFilter)
  itkNewMacro(Self)

  GrabCutOpenCVImageFilter();
  virtual ~GrabCutOpenCVImageFilter();

  bool FilterImage( cv::Mat& image );

  void ClearPoints();
  void AddForegroundPoints(std::vector<itk::Index<2> > foregroundPoints);
  void AddBackgroundPoints(std::vector<itk::Index<2> > backgroundPoints);

  const cv::Mat& GetResultMask();

protected:
  bool                              m_PointSetsChanged;

  std::vector<itk::Index<2> >         m_ForegroundPoints;
  std::vector<itk::Index<2> >         m_BackgroundPoints;

  cv::Mat                           m_GrabCutMask;
  cv::Mat                           m_ResultMask;

  void UpdateMaskFromPointSets(cv::Size imageSize);
  void UpdateMaskWithPointSet(std::vector<itk::Index<2> > pointSet, int pixelValue);
};

} // namespace mitk

#endif // MITKGRABCUTOPENCVIMAGEFILTER_H
