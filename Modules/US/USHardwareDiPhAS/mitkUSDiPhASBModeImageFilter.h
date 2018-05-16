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
/*=========================================================================
*
*  Copyright Insight Software Consortium
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
#ifndef itkPhotoacousticBModeImageFilter_h
#define itkPhotoacousticBModeImageFilter_h

#include "itkComplexToModulusImageFilter.h"
#include "itkConstantPadImageFilter.h"
#include "itkImageToImageFilter.h"
#include "itkImage.h"

#include "ITKUltrasound/itkRegionFromReferenceImageFilter.h"
#include "ITKUltrasound/itkAnalyticSignalImageFilter.h"

namespace itk
{

  /**
  * \class PhotoacousticBModeImageFilter
  *
  * \brief Create an Photoacoustic B-Mode (Brightness-Mode) image from raw
  * "RF" data.  The RF's envelope is calculated from the analytic signal and
  * logarithmic intensity transform is NOT applied. This is for now the only
  * difference to the "normal" BModeImageFilter.
  *
  * Use SetDirection() to define the axis of propagation.
  *
  */
  template < typename TInputImage, typename TOutputImage = TInputImage, typename TComplexImage = Image< std::complex< typename TInputImage::PixelType >, TInputImage::ImageDimension > >
  class PhotoacousticBModeImageFilter :
    public ImageToImageFilter< TInputImage, TOutputImage >
  {
  public:
    /** Standard class typedefs.   */
    typedef PhotoacousticBModeImageFilter                                Self;
    typedef ImageToImageFilter< TInputImage, TOutputImage > Superclass;
    typedef SmartPointer< Self >                            Pointer;
    typedef SmartPointer< const Self >                      ConstPointer;

    /** The type of input image.   */
    typedef TInputImage InputImageType;

    /** Dimension of the input and output images. */
    itkStaticConstMacro(ImageDimension, unsigned int,
      TInputImage::ImageDimension);

    /** Typedef support for the input image scalar value type. */
    typedef typename InputImageType::PixelType InputPixelType;

    /** The type of output image.   */
    typedef TOutputImage OutputImageType;

    /** Typedef support for the output image scalar value type. */
    typedef typename OutputImageType::PixelType OutputPixelType;

    /** Typedef of the image used for internal computations that has
    * std::complex pixels. */
    typedef TComplexImage ComplexImageType;

    /** Other convenient typedefs   */
    typedef typename InputImageType::RegionType InputRegionType;
    typedef typename InputImageType::SizeType   InputSizeType;
    typedef typename InputImageType::IndexType  InputIndexType;

    /** Run-time type information (and related methods) */
    itkTypeMacro(PhotoacousticBModeImageFilter, ImageToImageFilter);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    /** Set the direction in which the envelope is to be calculated. */
    virtual void SetDirection(unsigned int direction)
    {
      this->m_AnalyticFilter->SetDirection(direction);
      this->Modified();
    }

    /** Get the direction in which the envelope is to be calculated. */
    virtual unsigned int GetDirection() const
    {
      return m_AnalyticFilter->GetDirection();
    }

  protected:
    PhotoacousticBModeImageFilter();
    ~PhotoacousticBModeImageFilter() {}

    virtual void PrintSelf(std::ostream& os, Indent indent) const ITK_OVERRIDE;

    virtual void GenerateData() ITK_OVERRIDE;

    // These behave like their analogs in FFT1DRealToComplexConjugateImageFilter.
    virtual void GenerateInputRequestedRegion() ITK_OVERRIDE;
    virtual void EnlargeOutputRequestedRegion(DataObject *output) ITK_OVERRIDE;

    /** Component filters. */
    typedef AnalyticSignalImageFilter< InputImageType, ComplexImageType >                          AnalyticType;
    typedef ComplexToModulusImageFilter< typename AnalyticType::OutputImageType, OutputImageType > ComplexToModulusType;
    typedef ConstantPadImageFilter< InputImageType, InputImageType >                               PadType;
    typedef RegionFromReferenceImageFilter< OutputImageType, OutputImageType >                     ROIType;

  private:
    PhotoacousticBModeImageFilter(const Self&); // purposely not implemented
    void operator=(const Self&); // purposely not implemented

    typename AnalyticType::Pointer         m_AnalyticFilter;
    typename ComplexToModulusType::Pointer m_ComplexToModulusFilter;
    typename PadType::Pointer              m_PadFilter;
    typename ROIType::Pointer              m_ROIFilter;
  };

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkUSDiPhASBModeImageFilter.hxx"
#endif

#endif // itkPhotoacousticBModeImageFilter_h
