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
#ifndef __itkIntelligentBinaryClosingFilter_h
#define __itkIntelligentBinaryClosingFilter_h

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
  /** \class itkIntelligentBinaryClosingFilter
   * WARNING: This filtezr needs at least ITK version 3.2
   * or above to run and compile!!!
   *  */

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
    itkFactorylessNewMacro(Self) itkCloneMacro(Self)

      /** Standard process object method.  This filter is not multithreaded. */
      void GenerateData() override;

    /** Overloaded to link the input to this filter with the input of the
        mini-pipeline */
    void SetInput(const InputImageType *input) override
    {
      // processObject is not const-correct so a const_cast is needed here
      this->ProcessObject::SetNthInput(0, const_cast<InputImageType *>(input));
      m_DilateImageFilter->SetInput(input);
    }

    using Superclass::SetInput;
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

    itkGetMacro(ClosingRadius, float);
    itkSetMacro(ClosingRadius, float);

    itkGetMacro(SurfaceRatio, float);
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
#include "itkIntelligentBinaryClosingFilter.txx"

#endif

#endif // IntelligentBinaryClosingFilter_h
