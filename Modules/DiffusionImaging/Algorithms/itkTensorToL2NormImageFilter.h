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
#ifndef _itk_TensorToL2NormImageFilter_h_
#define _itk_TensorToL2NormImageFilter_h_

#include "itkImageToImageFilter.h"

namespace itk
{

  template <class TInputImage, class TOutputImage>
    class ITK_EXPORT TensorToL2NormImageFilter
    : public ImageToImageFilter<TInputImage, TOutputImage>
  {

  public:

    typedef TensorToL2NormImageFilter Self;
    typedef ImageToImageFilter<TInputImage, TOutputImage> Superclass;

    typedef SmartPointer<Self> Pointer;
    typedef SmartPointer<const Self> ConstPointer;

    itkTypeMacro (TensorToL2NormImageFilter, ImageToImageFilter);

    itkStaticConstMacro (ImageDimension, unsigned int,
                         TOutputImage::ImageDimension);

    itkNewMacro (Self);

    typedef TInputImage                           InputImageType;
    typedef typename InputImageType::PixelType    InputPixelType;
    typedef TOutputImage                          OutputImageType;
    typedef typename OutputImageType::PixelType   OutputPixelType;
    typedef typename OutputImageType::RegionType  OutputImageRegionType;

  protected:
    TensorToL2NormImageFilter(){};
    ~TensorToL2NormImageFilter(){};

    void PrintSelf (std::ostream& os, Indent indent) const
    {
      Superclass::PrintSelf (os, indent);
    }


    void ThreadedGenerateData(const OutputImageRegionType &outputRegionForThread, int threadId);


  private:
    TensorToL2NormImageFilter (const Self&);
    void operator=(const Self&);

    };


} // end of namespace


#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTensorToL2NormImageFilter.txx"
#endif


#endif
