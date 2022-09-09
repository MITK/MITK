/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// mitk headers
#include "mitkGrabCutOpenCVImageFilter.h"
#include "mitkPointSet.h"

#include <opencv2/imgproc.hpp>
#include <opencv2/imgproc/imgproc_c.h>

// This is a magic number defined in "grabcut.cpp" of OpenCV.
// GrabCut function crashes if less than this number of model
// points are given. There must be at least as much model points
// as components of the Gaussian Mixture Model.
#define GMM_COMPONENTS_COUNT 5

mitk::GrabCutOpenCVImageFilter::GrabCutOpenCVImageFilter()
  : m_ModelPointsDilationSize(0),
    m_UseOnlyRegionAroundModelPoints(false),
    m_CurrentProcessImageNum(0),
    m_InputImageId(AbstractOpenCVImageFilter::INVALID_IMAGE_ID),
    m_ResultImageId(AbstractOpenCVImageFilter::INVALID_IMAGE_ID),
    m_StopThread(false)
{
  m_Thread = std::thread(&GrabCutOpenCVImageFilter::SegmentationWorker, this);
}

mitk::GrabCutOpenCVImageFilter::~GrabCutOpenCVImageFilter()
{
  // terminate worker thread on destruction
  m_StopThread = true;
  m_WorkerBarrier.notify_all();
  if (m_Thread.joinable())
    m_Thread.detach();
}

bool mitk::GrabCutOpenCVImageFilter::OnFilterImage( cv::Mat& image )
{
  if ( image.empty() )
  {
    MITK_WARN << "Filtering empty image?";
    return false;
  }

  // make sure that the image is an rgb image as needed
  // by the GrabCut algorithm
  if (image.type() != CV_8UC3)
  {
    cv::Mat tmp = image.clone();
    cv::cvtColor(tmp, image, CV_GRAY2RGB);
  }

  // set image as the current input image, guarded by
  // a mutex as the worker thread reads this imagr
  m_ImageMutex.lock();
  m_InputImage = image.clone();
  m_InputImageId = this->GetCurrentImageId();
  m_ImageMutex.unlock();

  // wake up the worker thread if there was an image set
  // and foreground model points are available
  if ( ! m_ForegroundPoints.empty()) { m_WorkerBarrier.notify_all(); }

  return true;
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(ModelPointsList foregroundPoints)
{
  m_PointSetsMutex.lock();
  m_ForegroundPoints = foregroundPoints;
  m_PointSetsMutex.unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(ModelPointsList foregroundPoints, ModelPointsList backgroundPoints)
{
  m_PointSetsMutex.lock();
  m_BackgroundPoints = backgroundPoints;
  m_ForegroundPoints = foregroundPoints;
  m_PointSetsMutex.unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(cv::Mat foregroundMask)
{
  m_PointSetsMutex.lock();
  m_ForegroundPoints = this->ConvertMaskToModelPointsList(foregroundMask);
  m_PointSetsMutex.unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPoints(cv::Mat foregroundMask, cv::Mat backgroundMask)
{
  m_PointSetsMutex.lock();
  m_ForegroundPoints = this->ConvertMaskToModelPointsList(foregroundMask);
  m_BackgroundPoints = this->ConvertMaskToModelPointsList(backgroundMask);
  m_PointSetsMutex.unlock();
}

void mitk::GrabCutOpenCVImageFilter::SetModelPointsDilationSize(int modelPointsDilationSize)
{
  if ( modelPointsDilationSize < 0 )
  {
    MITK_ERROR("AbstractOpenCVImageFilter")("GrabCutOpenCVImageFilter")
            << "Model points dilation size must not be smaller then zero.";
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

cv::Rect mitk::GrabCutOpenCVImageFilter::GetRegionAroundModelPoints()
{
  return m_BoundingBox;
}

int mitk::GrabCutOpenCVImageFilter::GetResultImageId()
{
  return m_ResultImageId;
}

cv::Mat mitk::GrabCutOpenCVImageFilter::GetResultMask()
{
  cv::Mat result;

  m_ResultMutex.lock();
  result = m_ResultMask.clone();
  m_ResultMutex.unlock();

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

  // convert cvContours to vector of ModelPointsLists
  for ( unsigned int i = 0; i < cvContours.size(); ++i )
  {
    mitk::GrabCutOpenCVImageFilter::ModelPointsList curContourPoints;

    for ( auto it = cvContours[i].begin();
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
  if (mask.empty()) { return mitk::GrabCutOpenCVImageFilter::ModelPointsList(); }

  // return empty model point list if given pixel is outside the image borders
  if (pixelIndex.GetElement(0) < 0 || pixelIndex.GetElement(0) >= mask.size().height
    || pixelIndex.GetElement(1) < 0 || pixelIndex.GetElement(1) >= mask.size().width)
  {
    MITK_WARN("AbstractOpenCVImageFilter")("GrabCutOpenCVImageFilter")
        << "Given pixel index ("<< pixelIndex.GetElement(0) << ", " << pixelIndex.GetElement(1)
        << ") is outside the image (" << mask.size().height << ", " << mask.size().width << ").";

    return mitk::GrabCutOpenCVImageFilter::ModelPointsList();
  }

  // create a mask where the segmentation around the given pixel index is
  // set (done by flood filling the result mask using the pixel as seed)
  cv::floodFill(mask, cv::Point(pixelIndex.GetElement(0), pixelIndex.GetElement(1)), 5);

  cv::Mat foregroundMask;
  cv::compare(mask, 5, foregroundMask, cv::CMP_EQ);

  // find the contour on the flood filled image (there can be only one now)
  std::vector<std::vector<cv::Point> > cvContours;
  std::vector<cv::Vec4i> hierarchy;
  cv::findContours(foregroundMask, cvContours, hierarchy, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);

  ModelPointsList contourPoints;

  // convert cvContours to ModelPointsList
  for ( auto it = cvContours[0].begin();
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
  // initialize mask with values of propably background
  cv::Mat mask(m_InputImage.size().height, m_InputImage.size().width, CV_8UC1, cv::GC_PR_BGD);

  // get foreground and background points (guarded by mutex)
  m_PointSetsMutex.lock();
  ModelPointsList pointsLists[2] = {ModelPointsList(m_ForegroundPoints), ModelPointsList(m_BackgroundPoints)};
  m_PointSetsMutex.unlock();

  // define values for foreground and background pixels
  unsigned int pixelValues[2] = {cv::GC_FGD, cv::GC_BGD};

  for (unsigned int n = 0; n < 2; ++n)
  {
    for (auto it = pointsLists[n].begin();
         it != pointsLists[n].end(); ++it)
    {
      // set pixels around current pixel to the same value (size of this
      // area is specified by ModelPointsDilationSize)
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
    MITK_WARN("AbstractOpenCVImageFilter")("GrabCutOpenCVImageFilter")
        << "Cannot find any foreground points. Returning full image size as bounding rectangle.";
    return cv::Rect(0, 0, mask.rows, mask.cols);
  }

  // calculate bounding rect around the model points
  cv::Rect boundingRect = cv::boundingRect(modelPoints);

  // substract additional width to x and y value (and make sure that they aren't outside the image then)
  boundingRect.x = static_cast<unsigned int>(boundingRect.x) > m_AdditionalWidth ? boundingRect.x - m_AdditionalWidth : 0;
  boundingRect.y = static_cast<unsigned int>(boundingRect.y) > m_AdditionalWidth ? boundingRect.y - m_AdditionalWidth : 0;

  // add additional width to width of bounding rect (twice as x value was moved before)
  // and make sure that the bounding rect will stay inside the image borders)
  if ( static_cast<unsigned int>(boundingRect.x + boundingRect.width)
       + 2 * m_AdditionalWidth < static_cast<unsigned int>(mask.size().width) )
  {
    boundingRect.width += 2 * m_AdditionalWidth;
  }
  else
  {
    boundingRect.width = mask.size().width - boundingRect.x - 1;
  }

  // add additional width to height of bounding rect (twice as y value was moved before)
  // and make sure that the bounding rect will stay inside the image borders)
  if ( static_cast<unsigned int>(boundingRect.y + boundingRect.height)
       + 2 * m_AdditionalWidth < static_cast<unsigned int>(mask.size().height) )
  {
    boundingRect.height += 2 * m_AdditionalWidth;
  }
  else
  {
    boundingRect.height = mask.size().height - boundingRect.y - 1;
  }

  assert(boundingRect.x + boundingRect.width < mask.size().width);
  assert(boundingRect.y + boundingRect.height < mask.size().height);

  return boundingRect;
}

cv::Mat mitk::GrabCutOpenCVImageFilter::RunSegmentation(cv::Mat input, cv::Mat mask)
{
  // test if foreground and background models are large enough for GrabCut
  cv::Mat compareFgResult, compareBgResult;
  cv::compare(mask, cv::GC_FGD, compareFgResult, cv::CMP_EQ);
  cv::compare(mask, cv::GC_PR_BGD, compareBgResult, cv::CMP_EQ);
  if ( cv::countNonZero(compareFgResult) < GMM_COMPONENTS_COUNT
    || cv::countNonZero(compareBgResult) < GMM_COMPONENTS_COUNT)
  {
    // return result mask with no pixels set to foreground
    return cv::Mat::zeros(mask.size(), mask.type());
  }

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

void mitk::GrabCutOpenCVImageFilter::SegmentationWorker()
{
  std::mutex mutex;
  std::unique_lock<std::mutex> lock(mutex);

  while (true)
  {
    m_WorkerBarrier.wait(lock, [this] { return !m_StopThread; });

    m_ImageMutex.lock();
    cv::Mat image = m_InputImage.clone();
    int inputImageId = m_InputImageId;
    m_ImageMutex.unlock();

    cv::Mat mask = this->GetMaskFromPointSets();

    cv::Mat result;
    if (m_UseOnlyRegionAroundModelPoints)
    {
      result = cv::Mat(mask.rows, mask.cols, mask.type(), 0.0);
      m_BoundingBox = this->GetBoundingRectFromMask(mask);
      RunSegmentation(image(m_BoundingBox), mask(m_BoundingBox)).copyTo(result(m_BoundingBox));
    }
    else
    {
      result = this->RunSegmentation(image, mask);
    }

    // save result to member attribute
    m_ResultMutex.lock();
    m_ResultMask = result;
    m_ResultImageId = inputImageId;
    m_ResultMutex.unlock();
  }
}
