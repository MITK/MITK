/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef itkTotalVariationDenoisingImageFilter_h
#define itkTotalVariationDenoisingImageFilter_h

#include <itkCastImageFilter.h>
#include <itkImage.h>
#include <itkImageToImageFilter.h>
#include <itkTotalVariationSingleIterationImageFilter.h>

namespace itk
{
  /** \class TotalVariationDenoisingImageFilter
   * \brief Applies iterative total variation (TV) denoising to an image.
   *
   * This filter performs image denoising by minimizing the total variation
   * of the image. It iteratively applies TotalVariationSingleIterationImageFilter,
   * which performs a weighted averaging step that balances fidelity to the
   * original image against smoothness. The trade-off is controlled by the
   * Lambda parameter.
   *
   * Higher Lambda values favor fidelity to the original image (less smoothing),
   * while lower Lambda values favor smoothness (more denoising). The number of
   * iterations controls how many denoising steps are performed.
   *
   * Default values: Lambda = 1.0, NumberIterations = 0.
   *
   * Reference: Tony F. Chan et al., The digital TV filter and nonlinear denoising
   *
   * \tparam TInputImage The type of the input image.
   * \tparam TOutputImage The type of the output image.
   *
   * \sa TotalVariationSingleIterationImageFilter
   * \sa LocalVariationImageFilter
   * \sa Image
   * \sa Neighborhood
   * \sa NeighborhoodOperator
   * \sa NeighborhoodIterator
   *
   * \ingroup IntensityImageFilters
   */
  template <class TInputImage, class TOutputImage>
  class TotalVariationDenoisingImageFilter : public ImageToImageFilter<TInputImage, TOutputImage>
  {
  public:
    /** Extract dimension from input and output image. */
    itkStaticConstMacro(InputImageDimension, unsigned int, TInputImage::ImageDimension);
    itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

    /** Convenient typedefs for simplifying declarations. */
    typedef TInputImage InputImageType;
    typedef TOutputImage OutputImageType;

    /** Standard class typedefs. */
    typedef TotalVariationDenoisingImageFilter Self;
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;
    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /** Run-time type information (and related methods). */
      itkTypeMacro(TotalVariationDenoisingImageFilter, ImageToImageFilter);

    /** Image typedef support. */
    typedef typename InputImageType::PixelType InputPixelType;
    typedef typename OutputImageType::PixelType OutputPixelType;

    typedef typename InputImageType::RegionType InputImageRegionType;
    typedef typename OutputImageType::RegionType OutputImageRegionType;

    typedef typename InputImageType::SizeType InputSizeType;

    /** Type of the single-iteration sub-filter used internally. */
    typedef TotalVariationSingleIterationImageFilter<TOutputImage, TOutputImage> SingleIterationFilterType;

    /** Cast filter type used to convert input to output pixel type. */
    typedef typename itk::CastImageFilter<TInputImage, TOutputImage> CastType;

    /**
     * \brief Set/Get the regularization parameter Lambda.
     *
     * Lambda controls the trade-off between fidelity to the original image
     * and smoothness. Higher values preserve more detail; lower values
     * produce stronger denoising. Default is 1.0.
     */
    itkSetMacro(Lambda, double);
    itkGetMacro(Lambda, double);

    /**
     * \brief Set/Get the number of denoising iterations.
     *
     * Each iteration applies one pass of the total variation single-iteration
     * filter. More iterations yield stronger denoising. Default is 0
     * (no denoising).
     */
    itkSetMacro(NumberIterations, int);
    itkGetMacro(NumberIterations, int);

  protected:
    TotalVariationDenoisingImageFilter();
    ~TotalVariationDenoisingImageFilter() override {}
    void PrintSelf(std::ostream &os, Indent indent) const override;

    void GenerateData() override;

    double m_Lambda;

    int m_NumberIterations;

  private:
    TotalVariationDenoisingImageFilter(const Self &); // purposely not implemented
    void operator=(const Self &);                     // purposely not implemented
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkTotalVariationDenoisingImageFilter.tpp>
#endif

#endif //__itkTotalVariationDenoisingImageFilter__
