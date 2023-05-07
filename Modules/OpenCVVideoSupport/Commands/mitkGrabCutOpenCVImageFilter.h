/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGrabCutOpenCVImageFilter_h
#define mitkGrabCutOpenCVImageFilter_h

// mitk headers
#include "mitkAbstractOpenCVImageFilter.h"
#include "mitkVector.h"

// itk headers
#include "itkObjectFactory.h"
#include <itkThreadSupport.h>

// opencv headers
#include <opencv2/core.hpp>

#include <condition_variable>
#include <mutex>
#include <thread>

namespace itk {
template<unsigned int T> class Index;
template<class T> class SmartPointer;
class MultiThreader;
class ConditionVariable;
class FastMutexLock;
}

namespace mitk {
class PointSet;

/**
 * \brief Makes the OpenCV GrabCut filter available as OpenCVImageFilter.
 *
 * Image filtering is done asynchronly by using a worker thread as GrabCut segmentation
 * can take up to some seconds. Calling the mitk::GrabCutOpenCVImageFilter::OnFilterImage()
 * method sets just the input image and wakes up the worker thread. It is not guaranteed
 * that every image gets segmented. If multiple new images where set before a segmentation
 * was finished, only the last new image gets segmented afterwards.
 *
 * At least foreground model points have to be set by
 * mitk::GrabCutOpenCVImageFilter::SetModelPoints() before a segmentation can be performed.
 * The worder thread will not be waken up before any model points were set.
 *
 * When a new segmentation is ready, mitk::GrabCutOpenCVImageFilter::GetCurrentImageId()
 * returns a new image id. The segmentation can be got then by calling
 * mitk::GrabCutOpenCVImageFilter::GetResultMask(),
 * mitk::GrabCutOpenCVImageFilter::GetResultContours() or
 * mitk::GrabCutOpenCVImageFilter::GetResultContourWithPixel().
 */
class MITKOPENCVVIDEOSUPPORT_EXPORT GrabCutOpenCVImageFilter : public AbstractOpenCVImageFilter
{
public:
  /** \brief List holding image indices of the model points. */
  typedef std::vector<itk::Index<2> > ModelPointsList;

  mitkClassMacro(GrabCutOpenCVImageFilter, AbstractOpenCVImageFilter);

  itkFactorylessNewMacro(Self);

  itkCloneMacro(Self);

  GrabCutOpenCVImageFilter();
  ~GrabCutOpenCVImageFilter() override;

  /**
   * \brief Implementation of the virtual image filtering method.
   * The input image is copied to a member attribute, but the actual filtering is not done
   * in this method. Instead a worker thread is waken up every time this method is called,
   * if foreground model points were set before.
   *
   * \param image OpenCV image to be segmentated
   * \return false if an empty image was set, true otherwise
   */
  bool OnFilterImage( cv::Mat& image ) override;

  /**
   * \brief Sets a list of image indices as foreground model points.
   * \param foregroundPoints List of image indices which definitely belong to the foreground.
   */
  void SetModelPoints(ModelPointsList foregroundPoints);

  /**
   * \brief Sets a list of image indices as foreground and another list as background model points.
   * \param foregroundPoints List of image indices which definitely belong to the foreground.
   * \param backgroundPoints List of image indices which definitely belong to the background.
   */
  void SetModelPoints(ModelPointsList foregroundPoints, ModelPointsList backgroundPoints);

  /**
   * \brief Sets a mask where every non-zero pixel is treated as a foreground model pixel.
   */
  void SetModelPoints(cv::Mat foregroundMask);

  /**
   * \brief Sets masks specifying foreground and background points.
   * \param foregroundMask every non-zero pixel is treated as a foreground model pixel
   * \param backgroundMask every non-zero pixel is treated as a background model pixel
   */
  void SetModelPoints(cv::Mat foregroundMask, cv::Mat backgroundMask);

  /**
   * \brief Set a size of which each model point is dilated before image filtering.
   * The more color information of the foreground object the GrabCut filter gets the better
   * the result will be. Therefore the foreground pixels can be dilated before filtering. The
   * caller is responsible for setting a dilation size so that no foreground model pixels will
   * be indeed part of the background after dilation.
   *
   * Dilation is done to the background model pixles as well, if there are any set for the
   * filter.
   *
   * \param modelPointsDilationSize how many pixels are added in every direction, 0 sets back to no dilation
   */
  void SetModelPointsDilationSize(int modelPointsDilationSize);

  /**
   * \brief Use only the region around the foreground model points for the segmentation.
   *
   * This is mainly for reasons of segmentation speed and has the drawback that the foreground
   * model points (plus the given additional border) have to cover the whole foreground object.
   *
   * The segmentation filter can be set back to using the whole image by calling
   * mitk::GrabCutOpenCVImageFilter::SetUseFullImage().
   *
   * \param additionalBorder size of the border around the foreground points which will be used for segmentation, too
   */
  void SetUseOnlyRegionAroundModelPoints(unsigned int additionalBorder);

  /**
   * \brief The full image is used as input for the segmentation.
   * This method sets the behaviour back to the default behaviour in case
   * mitk::GrabCutOpenCVImageFilter::SetUseOnlyRegionAroundModelPoints() was
   * called before.
   */
  void SetUseFullImage();

  /**
   * \brief Getter for the rectangle used for the area of segmentation.
   * See mitk::GrabCutOpenCVImageFilter::SetUseOnlyRegionAroundModelPoints().
   * This method is mainly for debugging purposes and may be removed in
   * the future.
   */
  cv::Rect GetRegionAroundModelPoints();

  /**
   * \brief Getter for an ascending id of the current result image.
   * The id will be increased for every segmentation that is produced by the worker thread.
   * It can be used to determine if a new segmentation was produced since the last time a
   * segmentation was got from this filter.
   *
   * int lastResultImageId = grabCutFilter->GetResultImageId();
   * // do something
   * if ( lastResultImageId != grabCutFilter->GetResultImageId() )
   *   // get new segmentation
   */
  int GetResultImageId();

  /**
   * \brief Getter for the result mask of the current segmentation.
   * The result of this method is not necessarily consistent with the result of
   * mitk::GrabCutOpenCVImageFilter::GetResultContours() if they are called afterwards.
   * The segmentation may have changed in the meantime. One should decide if he needs
   * a mask or a contour or convert one into the other on his own.
   * \return image of the size of the input image where all pixels segmented as foreground are non-zero
   */
  cv::Mat GetResultMask();

  /**
   * \brief Getter for the contours of the current segmentation.
   *
   * A segmentation can consist of multiple regions therefore a list of contours
   * is returned. If one needs only one specific region he can call
   * mitk::GrabCutOpenCVImageFilter::GetResultContourWithPixel().
   *
   * This result of this method is not necessarily consistent with the result of
   * mitk::GrabCutOpenCVImageFilter::GetResultContours() if they are called afterwards.
   * The segmentation may have changed in the meantime. One should decide if he needs
   * a mask or a contour or convert one into the other on his own.
   *
   * \return List containing lists of pixel indices for every contour.
   */
  std::vector<ModelPointsList> GetResultContours();

  /**
   * \brief Getter for one specific contour of the current segmentation.
   *
   * Can be used if only one (of possible multiple contours) is needed. A pixel index
   * must be given to select from the contours. This could be one of the foreground
   * model pixels for example. If other criteria are needed to distinguish the contours
   * mitk::GrabCutOpenCVImageFilter::GetResultContours() can be used instead and therefore
   * contour selection can be done by hand then.
   *
   * This result of this method is not necessarily consistent with the result of
   * mitk::GrabCutOpenCVImageFilter::GetResultContours() if they are called afterwards.
   * The segmentation may have changed in the meantime. One should decide if he needs
   * a mask or a contour or convert one into the other on his own.
   *
   * \param pixelIndex index of a pixel which lies inside the contour
   * \return list of pixel indices for the selected contour
   */
  ModelPointsList GetResultContourWithPixel(itk::Index<2> pixelIndex);

protected:
  /** \brief Creates an image mask for GrabCut algorithm by using the foreground and background point sets.
   * Background and foreground points will be dilated by the size set by
   * mitk::GrabCutOpenCVImageFilter::SetModelPointsDilationSize().
   */
  cv::Mat GetMaskFromPointSets();

  /**
   * \brief Creates a bounding box around all pixels which aren't propably background.
   * The bounding box is widened as specified by
   * mitk::GrabCutOpenCVImageFilter::SetUseOnlyRegionAroundModelPoints().
   */
  cv::Rect GetBoundingRectFromMask(cv::Mat mask);

  /**
   * \brief Performs a GrabCut segmentation of the given input image.
   * \param input image on which the segmentation will be performed
   * \param mask foreground and background pixels used as basis for segmentation
   * \return mask with every pixel of the segmented foreground object set non-zero
   */
  cv::Mat RunSegmentation(cv::Mat input, cv::Mat mask);

  /**
   * \brief Creates a list of points from every non-zero pixel of the given mask.
   */
  ModelPointsList ConvertMaskToModelPointsList(cv::Mat mask);

  int                          m_ModelPointsDilationSize;

  bool                         m_UseOnlyRegionAroundModelPoints;
  unsigned int                 m_AdditionalWidth;
  cv::Rect                     m_BoundingBox;

  ModelPointsList              m_ForegroundPoints;
  ModelPointsList              m_BackgroundPoints;

  cv::Mat                      m_InputImage;
  cv::Mat                      m_GrabCutMask;
  cv::Mat                      m_ResultMask;

  unsigned int                 m_CurrentProcessImageNum;

  /** \brief id of the image currently set as m_InputImage */
  int                          m_InputImageId;

  /** \brief id of the image which segmentation result is currently present in m_ResultMask */
  int                          m_ResultImageId;

private:
  /**
   * \brief Worker thread for doing the segmentation.
   * It blocks every time a image was segmented and will be waken up again by
   * the mitk::GrabCutOpenCVImageFilter::OnFilterImage() method.
   *
   * \return
   */
  void SegmentationWorker();

  std::thread m_Thread;

  /** \brief worker thread will terminate after the next wakeup if set to true */
  bool                                      m_StopThread;

  std::condition_variable m_WorkerBarrier;

  /** \brief mutex for guarding m_InputImage and m_InputImageId */
  std::mutex     m_ImageMutex;

  /** \brief mutex for guarding m_ResultMask and m_ResultImageId */
  std::mutex     m_ResultMutex;

  /** \brief mutex for guarding m_ForegroundPoints and m_BackgroundPoints */
  std::mutex     m_PointSetsMutex;
};
} // namespace mitk

#endif
