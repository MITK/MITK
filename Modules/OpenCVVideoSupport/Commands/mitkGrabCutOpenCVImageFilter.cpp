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
    m_InputImageId(AbstractOpenCVImageFilter::INVALID_IMAGE_ID),
    m_ResultImageId(AbstractOpenCVImageFilter::INVALID_IMAGE_ID),
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

bool mitk::GrabCutOpenCVImageFilter::OnFilterImage( cv::Mat& image )
{
  if ( image.empty() )
  {
    MITK_WARN << "Filtering empty image?";
    return false;
  }

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

  // make sure that the image is an rgb image
  if (image.type() != CV_8UC3)
  {
    cv::Mat test = image.clone();
    cv::cvtColor(test, image, CV_GRAY2RGB);
  }

  m_InputImage = image.clone();
  m_InputImageId = this->GetCurrentImageId();
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

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(cv::Mat foregroundMask)
{
  m_PointSetsMutex->Lock();

  m_ForegroundPoints = this->ConvertMaskToModelPointsList(foregroundMask);
  m_PointSetsChanged = true;

  m_PointSetsMutex->Unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(cv::Mat foregroundMask, cv::Mat backgroundMask)
{
  m_PointSetsMutex->Lock();

  m_ForegroundPoints = this->ConvertMaskToModelPointsList(foregroundMask);
  m_BackgroundPoints = this->ConvertMaskToModelPointsList(backgroundMask);
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

int mitk::GrabCutOpenCVImageFilter::GetResultImageId()
{
  return m_ResultImageId;
}

cv::Mat mitk::GrabCutOpenCVImageFilter::GetResultMask()
{
  cv::Mat result;

  m_ResultMutex->Lock();
  result = m_ResultMask.clone();
  m_ResultMutex->Unlock();

  return result;
}

std::vector<mitk::GrabCutOpenCVImageFilter::ModelPointsList> mitk::GrabCutOpenCVImageFilter::GetResultContours()
{
  std::vector<std::vector<cv::Point> > cvContours;
  std::vector<cv::Vec4i> hierarchy;
  std::vector<mitk::GrabCutOpenCVImageFilter::ModelPointsList> contourPoints;

  cv::Mat resultMask = this->GetResultMask();
  if (resultMask.empty()) { return contourPoints; }

  cv::findContours(resultMask, cvContours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

  for ( unsigned int i = 0; i < cvContours.size(); ++i )
  {
    mitk::GrabCutOpenCVImageFilter::ModelPointsList curContourPoints;

    for ( std::vector<cv::Point>::iterator it = cvContours[i].begin();
          it != cvContours[i].end(); ++it)
    {
      itk::Index<2> index;
      index.SetElement(0, it->x);
      index.SetElement(1, it->y);
      curContourPoints.push_back(index);
    }

    contourPoints.push_back(curContourPoints);
  }

  return contourPoints;
}

mitk::GrabCutOpenCVImageFilter::ModelPointsList mitk::GrabCutOpenCVImageFilter::GetResultContourWithPixel(itk::Index<2> pixelIndex)
{
  cv::Mat mask = this->GetResultMask();

  cv::floodFill(mask, cv::Point(pixelIndex.GetElement(0), pixelIndex.GetElement(1)), 5);

  cv::Mat foregroundMask;
  cv::compare(mask, 5, foregroundMask, cv::CMP_EQ);

  std::vector<std::vector<cv::Point> > cvContours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(foregroundMask, cvContours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

  ModelPointsList contourPoints;

  for ( std::vector<cv::Point>::iterator it = cvContours[0].begin();
        it != cvContours[0].end(); ++it)
  {
    itk::Index<2> index;
    index.SetElement(0, it->x);
    index.SetElement(1, it->y);
    contourPoints.push_back(index);
  }

  return contourPoints;
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

mitk::GrabCutOpenCVImageFilter::ModelPointsList mitk::GrabCutOpenCVImageFilter::ConvertMaskToModelPointsList(cv::Mat mask)
{
  cv::Mat points;
  cv::findNonZero(mask, points);

  // push extracted points into a vector of itk indices
  ModelPointsList pointsVector;
  for ( size_t n = 0; n < points.total(); ++n)
  {
    itk::Index<2> index;
    index.SetElement(0, points.at<cv::Point>(n).x);
    index.SetElement(1, points.at<cv::Point>(n).y);
    pointsVector.push_back(index);
  }

  return pointsVector;
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
    int inputImageId = thisObject->m_InputImageId;
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
    thisObject->m_ResultImageId = inputImageId;
    thisObject->m_ResultMutex->Unlock();
  }

  mutex.Unlock();

  return ITK_THREAD_RETURN_VALUE;
}
