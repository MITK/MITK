/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *  Copyright Samara State Medical University
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/
#ifndef itkLabelMedianImageFilter_h
#define itkLabelMedianImageFilter_h

#include "itkBinaryMedianImageFilter.h"

namespace itk
{
/** \class LabelMedianImageFilter
 * \brief Applies a version of the weighted median filter optimized and adopted for lebel images.
 *
 * This filter was developed by Sergey Khlutchin.
 *
 * This filter is based on BinaryMedianImageFilter with the following changes.
 * 1. The central element and its nearest neighbors (Manhattan distance < 2) are counted several times according to the CenterWeight parameter.
 * 2. Elements at Manhattan distance 2 from the center are counted twice (or once if CenterWeight==1).
 * 3. The pixel value can only be changed by +/- 1 or unchanged.
 * 4. Only pixels with value from the background to the foreground are processed, others are copied without changes.
 *
 * A median filter is one of the family of nonlinear filters.  It is
 * used to smooth an image without being biased by outliers or shot noise.
 *
 * \sa Image
 * \sa Neighborhood
 * \sa NeighborhoodOperator
 * \sa NeighborhoodIterator
 *
 * \ingroup IntensityImageFilters
 * \ingroup ITKLabelVoting
 */
template< typename TInputImage, typename TOutputImage >
class LabelMedianImageFilter:
  public BinaryMedianImageFilter< TInputImage, TOutputImage >
{
public:

  /** Convenient typedefs for simplifying declarations. */
  typedef TInputImage  InputImageType;
  typedef TOutputImage OutputImageType;

  /** Standard class typedefs. */
  typedef LabelMedianImageFilter                               Self;
  typedef BinaryMedianImageFilter< TInputImage, TOutputImage > Superclass;
  typedef SmartPointer< Self >                                 Pointer;
  typedef SmartPointer< const Self >                           ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(LabelMedianImageFilter, BinaryMedianImageFilter);

  /** Image typedef support. */
  typedef typename InputImageType::PixelType  InputPixelType;
  typedef typename OutputImageType::PixelType OutputPixelType;

  typedef typename InputImageType::RegionType  InputImageRegionType;
  typedef typename OutputImageType::RegionType OutputImageRegionType;

  typedef typename InputImageType::SizeType InputSizeType;

  itkSetMacro(CenterWeight, unsigned int);

protected:

  LabelMedianImageFilter();
  virtual ~LabelMedianImageFilter() {}

  /** LabelMedianImageFilter can be implemented as a multithreaded filter.
   * Therefore, this implementation provides a ThreadedGenerateData()
   * routine which is called for each processing thread. The output
   * image data is allocated automatically by the superclass prior to
   * calling ThreadedGenerateData().  ThreadedGenerateData can only
   * write to the portion of the output image specified by the
   * parameter "outputRegionForThread"
   *
   * \sa ImageToImageFilter::ThreadedGenerateData(),
   *     ImageToImageFilter::GenerateData() */
  void ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread,
                            ThreadIdType threadId) ITK_OVERRIDE;
  unsigned int m_CenterWeight;

private:

  LabelMedianImageFilter(const Self &) ITK_DELETE_FUNCTION;
  void operator=(const Self &) ITK_DELETE_FUNCTION;
};
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLabelMedianImageFilter.hxx"
#endif

#endif
