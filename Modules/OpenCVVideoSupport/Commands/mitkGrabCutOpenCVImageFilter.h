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
#include "itkMutexLock.h"

#include "cv.h"

namespace itk {
template<unsigned int T> class Index;
template<class T> class SmartPointer;
class MultiThreader;
class ConditionVariable;
class FastMutexLock;
}

namespace mitk {

class PointSet;

class MITK_OPENCVVIDEOSUPPORT_EXPORT GrabCutOpenCVImageFilter : public AbstractOpenCVImageFilter
{
public:
  typedef std::vector<itk::Index<2> > ModelPointsList;

  mitkClassMacro(GrabCutOpenCVImageFilter, AbstractOpenCVImageFilter)
  itkNewMacro(Self)

  GrabCutOpenCVImageFilter();
  virtual ~GrabCutOpenCVImageFilter();

  bool OnFilterImage( cv::Mat& image );

  void SetModelPoints(ModelPointsList foregroundPoints);
  void SetModelPoints(ModelPointsList foregroundPoints, ModelPointsList backgroundPoints);

  void SetModelPoints(cv::Mat foregroundMask);
  void SetModelPoints(cv::Mat foregroundMask, cv::Mat backgroundMask);

  void SetModelPointsDilationSize(int modelPointsDilationSize);

  void SetUseOnlyRegionAroundModelPoints(unsigned int additionalWidth);
  void SetUseFullImage();

  int GetResultImageId();
  cv::Mat GetResultMask();
  std::vector<ModelPointsList> GetResultContours();

protected:
  cv::Mat GetMaskFromPointSets();
  cv::Rect GetBoundingRectFromMask(cv::Mat mask);
  cv::Mat RunSegmentation(cv::Mat input, cv::Mat mask);

  ModelPointsList ConvertMaskToModelPointsList(cv::Mat mask);

  bool                         m_PointSetsChanged;
  bool                         m_InputImageChanged;
  bool                         m_NewResultReady;

  int                          m_ModelPointsDilationSize;

  bool                         m_UseOnlyRegionAroundModelPoints;
  unsigned int                 m_AdditionalWidth;

  ModelPointsList              m_ForegroundPoints;
  ModelPointsList              m_BackgroundPoints;

  cv::Mat                      m_InputImage;
  cv::Mat                      m_GrabCutMask;
  cv::Mat                      m_ResultMask;

  unsigned int                 m_ProcessEveryNumImage;
  unsigned int                 m_CurrentProcessImageNum;

  int                          m_InputImageId;
  int                          m_ResultImageId;

private:
  static ITK_THREAD_RETURN_TYPE SegmentationWorker(void* pInfoStruct);

  int                                       m_ThreadId;
  bool                                      m_StopThread;

  itk::SmartPointer<itk::MultiThreader>     m_MultiThreader;
  itk::SmartPointer<itk::ConditionVariable> m_WorkerBarrier;
  itk::SmartPointer<itk::FastMutexLock>     m_ImageMutex;
  itk::SmartPointer<itk::FastMutexLock>     m_ResultMutex;
  itk::SmartPointer<itk::FastMutexLock>     m_PointSetsMutex;
};

} // namespace mitk

#endif // MITKGRABCUTOPENCVIMAGEFILTER_H
