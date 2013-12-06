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
    m_ProcessEveryNumImage(1), m_CurrentProcessImageNum(0),
    m_ResultCount(0),
    m_ThreadId(-1),
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
  if ( m_ThreadId >= 0) { m_MultiThreader->TerminateThread(m_ThreadId); }
}

bool mitk::GrabCutOpenCVImageFilter::FilterImage( cv::Mat& image )
{
  // update just every x images (as configured)
  ++m_CurrentProcessImageNum %= m_ProcessEveryNumImage;
  if (m_CurrentProcessImageNum != 0) { return true; }

  m_ImageMutex->Lock();
  m_InputImage = image.clone();
  m_ImageMutex->Unlock();

  if ( ! m_ForegroundPoints.empty()) { m_WorkerBarrier->Broadcast(); }

  /*// nothing to update if the point sets haven't changed
  if ( ! m_PointSetsChanged ) { return true; }

  if ( m_ForegroundPoints.empty() )
  {
    MITK_WARN("GrabCutOpenCVImageFilter")("AbstractOpenCVImageFilter")
        << "Cannot filter image without foreground points set.";
    return false;
  }

  this->UpdateMaskFromPointSets();

  cv::Mat mask = m_GrabCutMask.clone();

  cv::Mat bgdModel, fgdModel;
  cv::grabCut(m_InputImage, mask, cv::Rect(), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);

  cv::compare(mask, cv::GC_PR_FGD, m_ResultMask, cv::CMP_EQ);

  cv::Mat foregroundMat;
  cv::compare(mask, cv::GC_FGD, foregroundMat, cv::CMP_EQ);

  foregroundMat.copyTo(m_ResultMask, foregroundMat);

  m_InputImageChanged = false;*/

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

const cv::Mat& mitk::GrabCutOpenCVImageFilter::GetResultMask()
{
  //this->UpdateMaskFromPointSets();

  return m_ResultMask;
}

/*bool mitk::GrabCutOpenCVImageFilter::UpdateResultMask()
{
  // nothing to update if the point sets haven't changed
  if ( ! m_PointSetsChanged ) { return true; }

  if ( m_InputImage.empty() )
  {
    MITK_WARN("GrabCutOpenCVImageFilter")("AbstractOpenCVImageFilter")
        << "Cannot update result mask without input image being set.";
    return false;
  }

  if ( m_ForegroundPoints.empty() )
  {
    MITK_WARN("GrabCutOpenCVImageFilter")("AbstractOpenCVImageFilter")
        << "Cannot filter image without foreground points set.";
    return false;
  }

  this->UpdateMaskFromPointSets();

  cv::Mat mask = m_GrabCutMask.clone();

  cv::Mat bgdModel, fgdModel;
  cv::grabCut(m_InputImage, mask, cv::Rect(), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);

  cv::compare(mask, cv::GC_PR_FGD, m_ResultMask, cv::CMP_EQ);

  cv::Mat foregroundMat;
  cv::compare(mask, cv::GC_FGD, foregroundMat, cv::CMP_EQ);

  foregroundMat.copyTo(m_ResultMask, foregroundMat);

  m_InputImageChanged = false;

  return true;
}*/

/*void mitk::GrabCutOpenCVImageFilter::UpdateMaskFromPointSets()
{
  if ( m_PointSetsChanged )
  {
    m_PointSetsMutex->Lock();

    m_GrabCutMask = cv::Mat(m_InputImage.size().height, m_InputImage.size().width, CV_8UC1, cv::GC_PR_BGD);

    this->UpdateMaskWithPointSet(m_ForegroundPoints, cv::GC_FGD);
    this->UpdateMaskWithPointSet(m_BackgroundPoints, cv::GC_BGD);

    m_PointSetsChanged = false;

    m_PointSetsMutex->Unlock();
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
}*/

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
  cv::Mat bgdModel, fgdModel;
  cv::grabCut(input, mask, cv::Rect(), bgdModel, fgdModel, 1, cv::GC_INIT_WITH_MASK);

  cv::compare(mask, cv::GC_PR_FGD, m_ResultMask, cv::CMP_EQ);

  cv::Mat foregroundMat;
  cv::compare(mask, cv::GC_FGD, foregroundMat, cv::CMP_EQ);

  cv::Mat result;
  foregroundMat.copyTo(result, foregroundMat);
  return result;
}

ITK_THREAD_RETURN_TYPE mitk::GrabCutOpenCVImageFilter::SegmentationWorker(void* pInfoStruct)
{
  /* extract this pointer from Thread Info structure */
  struct itk::MultiThreader::ThreadInfoStruct * pInfo = (struct itk::MultiThreader::ThreadInfoStruct*)pInfoStruct;
  mitk::GrabCutOpenCVImageFilter* thisObject = static_cast<mitk::GrabCutOpenCVImageFilter*>(pInfo->UserData);

  while (true)
  {
    itk::SimpleMutexLock* mutex = &(thisObject->m_WorkerMutex);
    mutex->Lock();
    thisObject->m_WorkerBarrier->Wait(mutex);

    thisObject->m_ImageMutex->Lock();
    cv::Mat image = thisObject->m_InputImage.clone();
    thisObject->m_ImageMutex->Unlock();

    cv::Mat mask = thisObject->GetMaskFromPointSets();

    cv::Mat result = thisObject->RunSegmentation(image, mask);
    thisObject->m_ResultMutex->Lock();
    thisObject->m_ResultMask = result;
    thisObject->m_ResultCount++;
    thisObject->m_ResultMutex->Unlock();
  }

  return ITK_THREAD_RETURN_VALUE;
}
