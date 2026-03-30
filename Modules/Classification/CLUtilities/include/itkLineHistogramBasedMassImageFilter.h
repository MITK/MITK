/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkLineHistogramBasedMassImageFilter_h
#define itkLineHistogramBasedMassImageFilter_h

#include <itkImageToImageFilter.h>

namespace itk
{
  /**
   * \brief Computes line-histogram-based mass features for each voxel in an image.
   *
   * For each voxel, a line from the center of mass of the mask through the voxel
   * is considered and histogram-based mass features are computed along this line.
   *
   * \tparam TInputImageType The input image type.
   * \tparam TOutputImageType The output image type (defaults to input type).
   * \tparam TMaskImageType The mask image type (defaults to itk::Image<short,3>).
   */
  template< class TInputImageType, class TOutputImageType = TInputImageType, class TMaskImageType = itk::Image<short,3> >
  class LineHistogramBasedMassImageFilter
    : public itk::ImageToImageFilter<TInputImageType, TOutputImageType>
  {
  public:

    typedef LineHistogramBasedMassImageFilter< TInputImageType, TOutputImageType > Self;
    typedef SmartPointer<Self>                      Pointer;
    typedef SmartPointer<const Self>                ConstPointer;
    typedef ImageToImageFilter<  TInputImageType, TOutputImageType  > Superclass;

    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /**
     * \brief Set the mask image defining the region of interest.
     * \param maskimage Pointer to the mask image.
     */
    void SetImageMask(TMaskImageType * maskimage);

    /**
     * \brief Set the binary contour image.
     * \param contouriamge Pointer to the binary contour image.
     */
    void SetBinaryContour(TMaskImageType * contouriamge);

  private:

    typename TMaskImageType::Pointer m_ImageMask;
    typename TMaskImageType::Pointer m_BinaryContour;
    vnl_vector<double> m_CenterOfMask;

    void ThreadedGenerateData(const typename Superclass::OutputImageRegionType &outputRegionForThread, ThreadIdType threadId) override;
    void BeforeThreadedGenerateData() override;
    //  void GenerateOutputInformation();

    vnl_vector<double> GetCenterOfMass(const TMaskImageType * maskImage);

    LineHistogramBasedMassImageFilter();
    ~LineHistogramBasedMassImageFilter() override;
  };
}

#ifndef ITK_MANUAL_INSTANTIATION
#include "../src/Features/itkLineHistogramBasedMassImageFilter.cpp"
#endif

#endif
