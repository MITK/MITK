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

#include "itkMultiThreader.h"
#include "itkFastMutexLock.h"
#include "itkConditionVariable.h"

mitk::GrabCutOpenCVImageFilter::GrabCutOpenCVImageFilter()
  : m_PointSetsChanged(false), m_InputImageChanged(false),
    m_NewResultReady(false),
    m_ModelPointsDilationSize(0),
    m_UseOnlyRegionAroundModelPoints(false),
    m_ProcessEveryNumImage(1), m_CurrentProcessImageNum(0),
    m_ResultCount(0),
    m_ThreadId(-1), m_StopThread(false),
    m_MultiThreader(itk::MultiThreader::New()),
    m_WorkerBarrier(itk::ConditionVariable::New()),
    m_ImageMutex(itk::FastMutexLock::New()),
    m_ResultMutex(itk::FastMutexLock::New()),
    m_PointSetsMutex(itk::FastMutexLock::New())
{
  m_ThreadId = m_MultiThreader->SpawnThread(this->SegmentationWorker, this);
}

mitk::GrabCutOpenCVImageFilter::~GrabCutOpenCVImageFilter()
{
  m_StopThread = true;
  m_WorkerBarrier->Broadcast();
  if ( m_ThreadId >= 0) { m_MultiThreader->TerminateThread(m_ThreadId); }
}

bool mitk::GrabCutOpenCVImageFilter::FilterImage( cv::Mat& image )
{
  if ( m_NewResultReady )
  {
    m_ResultMutex->Lock();
    m_NewResultReady = false;
    m_ResultMutex->Unlock();
  }

  // update just every x images (as configured)
  ++m_CurrentProcessImageNum %= m_ProcessEveryNumImage;
  if (m_CurrentProcessImageNum != 0) { return true; }

  m_ImageMutex->Lock();
  m_InputImage = image.clone();
  m_ImageMutex->Unlock();

  if ( ! m_ForegroundPoints.empty()) { m_WorkerBarrier->Broadcast(); }

  return true;
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(ModelPointsList foregroundPoints)
{
  m_PointSetsMutex->Lock();

  m_ForegroundPoints = foregroundPoints;
  m_PointSetsChanged = true;

  m_PointSetsMutex->Unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(ModelPointsList foregroundPoints, ModelPointsList backgroundPoints)
{
  m_PointSetsMutex->Lock();

  m_BackgroundPoints = backgroundPoints;
  m_ForegroundPoints = foregroundPoints;
  m_PointSetsChanged = true;

  m_PointSetsMutex->Unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPointsDilationSize(int modelPointsDilationSize)
{
  if ( modelPointsDilationSize < 0 )
  {
    MITK_ERROR << "Model points dilation size must not be smaller then zero.";
    mitkThrow() << "Model points dilation size must not be smaller then zero.";
  }
  m_ModelPointsDilationSize = modelPointsDilationSize;
}

void mitk::GrabCutOpenCVImageFilter::SetUseOnlyRegionAroundModelPoints(unsigned int additionalWidth)
{
  m_UseOnlyRegionAroundModelPoints = true;
  m_AdditionalWidth = additionalWidth;
}

void mitk::GrabCutOpenCVImageFilter::SetUseFullImage()
{
  m_UseOnlyRegionAroundModelPoints = false;
}

unsigned int mitk::GrabCutOpenCVImageFilter::GetResultCount()
{
  return m_ResultCount;
}

cv::Mat mitk::GrabCutOpenCVImageFilter::GetResultMask()
{
  cv::Mat result;

  m_ResultMutex->Lock();
  result = m_ResultMask.clone();
  m_ResultMutex->Unlock();

  return result;
}

cv::Mat mitk::GrabCutOpenCVImageFilter::GetMaskFromPointSets()
{
  cv::Mat mask(m_InputImage.size().height, m_InputImage.size().width, CV_8UC1, cv::GC_PR_BGD);

  m_PointSetsMutex->Lock();
  ModelPointsList pointsLists[2] = {ModelPointsList(m_ForegroundPoints), ModelPointsList(m_BackgroundPoints)};
  m_PointSetsMutex->Unlock();

  unsigned int pixelValues[2] = {cv::GC_FGD, cv::GC_BGD};

  for (unsigned int n = 0; n < 2; ++n)
  {
    for (ModelPointsList::iterator it = pointsLists[n].begin();
         it != pointsLists[n].end(); ++it)
    {
      for ( int i = -m_ModelPointsDilationSize; i <= m_ModelPointsDilationSize; ++i )
      {
        for ( int j = -m_ModelPointsDilationSize; j <= m_ModelPointsDilationSize; ++j)
        {
          int x = it->GetElement(1) + i; int y = it->GetElement(0) + j;
          if ( x >= 0 && y >= 0 && x < mask.cols && y < mask.rows)
          {
            mask.at<unsigned char>(x, y) = pixelValues[n];
          }
        }
      }
    }
  }

  return mask;
}

cv::Rect mitk::GrabCutOpenCVImageFilter::GetBoundingRectFromMask(cv::Mat mask)
{
  cv::Mat nonPropablyBackgroundMask, modelPoints;
  cv::compare(mask, cv::GC_PR_BGD, nonPropablyBackgroundMask, cv::CMP_NE);
  cv::findNonZero(nonPropablyBackgroundMask, modelPoints);

  if (modelPoints.empty())
  {
    MITK_WARN << "Cannot find any foreground points. Returning full image size as bounding rectangle.";
    return cv::Rect(0, 0, mask.rows, mask.cols);
  }

  cv::Rect boundingRect = cv::boundingRect(modelPoints);

  boundingRect.x = boundingRect.x > m_AdditionalWidth ? boundingRect.x - m_AdditionalWidth : 0;
  boundingRect.y = boundingRect.y > m_AdditionalWidth ? boundingRect.y - m_AdditionalWidth : 0;

  if ( boundingRect.x + boundingRect.width + m_AdditionalWidth < mask.size().width )
  {
    boundingRect.width += m_AdditionalWidth;
  }
  else
  {
    boundingRect.width = mask.size().width - boundingRect.x - 1;
  }

  if ( boundingRect.y + boundingRect.height + m_AdditionalWidth < mask.size().height )
  {
    boundingRect.height += m_AdditionalWidth;
  }
  else
  {
    boundingRect.height = mask.size().height - boundingRect.y - 1;
  }

  return boundingRect;
}

cv::Mat mitk::GrabCutOpenCVImageFilter::RunSegmentation(cv::Mat input, cv::Mat mask)
{
  // do the actual grab cut segmentation (initialized with the mask)
  cv::Mat bgdModel, fgdModel;
  cv::grabCut(input, mask, cv::Rect(), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);

  // set propably foreground pixels to white on result mask
  cv::Mat result;
  cv::compare(mask, cv::GC_PR_FGD, result, cv::CMP_EQ);

  // set foreground pixels to white on result mask
  cv::Mat foregroundMat;
  cv::compare(mask, cv::GC_FGD, foregroundMat, cv::CMP_EQ);
  foregroundMat.copyTo(result, foregroundMat);

  return result; // now the result mask can be returned
}

ITK_THREAD_RETURN_TYPE mitk::GrabCutOpenCVImageFilter::SegmentationWorker(void* pInfoStruct)
{
  // extract this pointer from thread info structure
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  mitk::GrabCutOpenCVImageFilter* thisObject = static_cast<mitk::GrabCutOpenCVImageFilter*>(pInfo->UserData);

  itk::SimpleMutexLock mutex;
  mutex.Lock();

  while (true)
  {
    if (thisObject->m_StopThread) { break; }

    thisObject->m_WorkerBarrier->Wait(&mutex);

    if (thisObject->m_StopThread) { break; }

    thisObject->m_ImageMutex->Lock();
    cv::Mat image = thisObject->m_InputImage.clone();
    thisObject->m_ImageMutex->Unlock();

    cv::Mat mask = thisObject->GetMaskFromPointSets();

    cv::Mat result;
    if (thisObject->m_UseOnlyRegionAroundModelPoints)
    {
      result = cv::Mat(mask.rows, mask.cols, mask.type(), 0.0);
      cv::Rect boundingBox = thisObject->GetBoundingRectFromMask(mask);
      thisObject->RunSegmentation(image(boundingBox), mask(boundingBox)).copyTo(result(boundingBox));
    }
    else
    {
      result = thisObject->RunSegmentation(image, mask);
    }

    thisObject->m_ResultMutex->Lock();
    thisObject->m_ResultMask = result;
    thisObject->m_NewResultReady = true;
    thisObject->m_ResultCount++;
    thisObject->m_ResultMutex->Unlock();
  }

  mutex.Unlock();

  return ITK_THREAD_RETURN_VALUE;
}
