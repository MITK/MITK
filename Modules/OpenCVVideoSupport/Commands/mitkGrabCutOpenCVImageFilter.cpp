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

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(std::vector<itk::Index<2> > foregroundPoints)
{
  m_PointSetsMutex->Lock();

  m_ForegroundPoints = foregroundPoints;
  m_PointSetsChanged = true;

  m_PointSetsMutex->Unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(std::vector<itk::Index<2> > foregroundPoints, std::vector<itk::Index<2> > backgroundPoints)
{
  m_PointSetsMutex->Lock();

  m_BackgroundPoints = backgroundPoints;
  m_ForegroundPoints = foregroundPoints;
  m_PointSetsChanged = true;

  m_PointSetsMutex->Unlock();
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
      for ( int i = -2; i <= 2; ++i )
      {
        for ( int j = -2; j <= 2; ++j)
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
    thisObject->m_WorkerBarrier->Wait(&mutex);

    if (thisObject->m_StopThread) { break; }

    thisObject->m_ImageMutex->Lock();
    cv::Mat image = thisObject->m_InputImage.clone();
    thisObject->m_ImageMutex->Unlock();

    cv::Mat mask = thisObject->GetMaskFromPointSets();

    cv::Mat result = thisObject->RunSegmentation(image, mask);
    thisObject->m_ResultMutex->Lock();
    thisObject->m_ResultMask = result;
    thisObject->m_NewResultReady = true;
    thisObject->m_ResultCount++;
    thisObject->m_ResultMutex->Unlock();
  }

  mutex.Unlock();

  return ITK_THREAD_RETURN_VALUE;
}
