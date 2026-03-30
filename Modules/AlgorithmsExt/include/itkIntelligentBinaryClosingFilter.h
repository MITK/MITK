/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef itkIntelligentBinaryClosingFilter_h
#define itkIntelligentBinaryClosingFilter_h

#if defined(_MSC_VER)
#pragma warning(disable : 4786)
#endif

#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryDilateImageFilter.h>
#include <itkBinaryErodeImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkGrayscaleDilateImageFilter.h>
#include <itkImageRegionIterator.h>
#include <itkImageToImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkSubtractImageFilter.h>

namespace itk
{
  /**
   * \class IntelligentBinaryClosingFilter
   * \brief Performs a morphological closing on a binary image, selectively filling
   * only those holes whose surface-to-volume ratio exceeds a given threshold.
   *
   * Unlike a standard binary closing, this filter analyzes connected components
   * in the closed region (the difference between dilated and eroded images) and
   * only fills components that meet the surface ratio criterion. This prevents
   * the closing operation from filling large cavities that should be preserved.
   *
   * The filter internally uses a mini-pipeline consisting of dilation, erosion,
   * subtraction, connected component analysis, and relabeling filters.
   *
   * \tparam TInputImage  The input image type (binary image).
   * \tparam TOutputImage The output image type (typically a labeled or binary image).
   *
   * \sa itk::BinaryDilateImageFilter
   * \sa itk::BinaryErodeImageFilter
   * \sa itk::ConnectedComponentImageFilter
   */
  template <class TInputImage, class TOutputImage>
  class ITK_EXPORT IntelligentBinaryClosingFilter : public ImageToImageFilter<TInputImage, TOutputImage>
  {
  public:
    /** Standard "Self" typedef.   */
    typedef IntelligentBinaryClosingFilter Self;

    /** The type of input image.   */
    typedef TInputImage InputImageType;

    /** Dimension of the input and output images. */
    itkStaticConstMacro(ImageDimension, unsigned int, TInputImage::ImageDimension);

    /** The type of output image.   */
    typedef TOutputImage OutputImageType;

    /** Standard super class typedef support. */
    typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;

    /** Smart pointer typedef support  */
    typedef SmartPointer<Self> Pointer;

    /** Run-time type information (and related methods) */
    itkTypeMacro(IntelligentBinaryClosingFilter, ImageToImageFilter);

    /** Method for creation through the object factory. */
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      /**
       * \brief Execute the intelligent binary closing algorithm.
       *
       * This method is not multithreaded. It runs the internal mini-pipeline
       * (dilate, erode, subtract, connected component analysis) and selectively
       * fills holes based on the surface ratio.
       */
      void GenerateData() override;

    /**
     * \brief Set the input binary image.
     *
     * Overloaded to also connect the input to the internal dilation filter.
     *
     * \param[in] input The input binary image.
     */
    void SetInput(const InputImageType *input) override
    {
      // processObject is not const-correct so a const_cast is needed here
      this->ProcessObject::SetNthInput(0, const_cast<InputImageType *>(input));
      m_DilateImageFilter->SetInput(input);
    }

    using Superclass::SetInput;
    /**
     * \brief Set the input at a given index. Only index 0 is supported.
     * \param[in] i Input index (must be 0).
     * \param[in] image The input binary image.
     * \throw itk::ExceptionObject if \p i is not 0.
     */
    void SetInput(unsigned int i, const TInputImage *image) override
    {
      if (i != 0)
      {
        itkExceptionMacro(<< "Filter has only one input.");
      }
      else
      {
        this->SetInput(image);
      }
    }

    /**
     * \brief Get the radius used for the morphological closing structuring element.
     * \return The closing radius in pixels.
     */
    itkGetMacro(ClosingRadius, float);

    /**
     * \brief Set the radius of the structuring element for morphological closing.
     * \param[in] _arg The closing radius in pixels.
     */
    itkSetMacro(ClosingRadius, float);

    /**
     * \brief Get the surface-to-volume ratio threshold.
     * \return The surface ratio threshold.
     */
    itkGetMacro(SurfaceRatio, float);

    /**
     * \brief Set the surface-to-volume ratio threshold.
     *
     * Connected components in the difference image whose surface ratio
     * exceeds this value will be filled in the output.
     *
     * \param[in] _arg The surface ratio threshold.
     */
    itkSetMacro(SurfaceRatio, float);

  protected:
    IntelligentBinaryClosingFilter();
    ~IntelligentBinaryClosingFilter() override {}
    void PrintSelf(std::ostream &os, Indent indent) const override;

  private:
    typedef typename InputImageType::PixelType InputPixelType;
    typedef BinaryBallStructuringElement<InputPixelType, ImageDimension> StructuringElementType;
    typedef BinaryErodeImageFilter<InputImageType, InputImageType, StructuringElementType> BinaryErodeImageFilterType;
    typedef BinaryDilateImageFilter<InputImageType, InputImageType, StructuringElementType> BinaryDilateImageFilterType;
    typedef SubtractImageFilter<InputImageType, InputImageType, InputImageType> SubtractImageFilterType;
    typedef ConnectedComponentImageFilter<InputImageType, OutputImageType> ConnectedComponentImageFilterType;
    typedef RelabelComponentImageFilter<OutputImageType, OutputImageType> RelabelComponentImageFilterType;
    typedef GrayscaleDilateImageFilter<OutputImageType, OutputImageType, StructuringElementType>
      DilateComponentImageFilterType;
    typedef ImageRegionIterator<InputImageType> InputIteratorType;
    typedef ImageRegionConstIterator<InputImageType> ConstInputIteratorType;
    typedef ImageRegionIterator<OutputImageType> OutputIteratorType;

    typename BinaryErodeImageFilterType::Pointer m_ErodeImageFilter;
    typename BinaryDilateImageFilterType::Pointer m_DilateImageFilter;
    typename SubtractImageFilterType::Pointer m_SubtractImageFilter;
    typename ConnectedComponentImageFilterType::Pointer m_ConnectedComponentImageFilter;
    typename RelabelComponentImageFilterType::Pointer m_RelabelComponentImageFilter;
    typename DilateComponentImageFilterType::Pointer m_BorderDetectionDilateFilter;
    // typename BinaryDilateImageFilterType::Pointer       m_BorderAdjacencyDilateFilter;
    float m_ClosingRadius;
    float m_SurfaceRatio;
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include <itkIntelligentBinaryClosingFilter.tpp>

#endif

#endif // IntelligentBinaryClosingFilter_h
