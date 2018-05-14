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

#ifndef __itkOdfToRgbImageFilter_h
#define __itkOdfToRgbImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkOrientationDistributionFunction.h"
#include "itkRGBAPixel.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"

namespace itk
{

  #define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );


/** \class OdfToRgbImageFilter
*
*/
template <typename TInputImage,
typename TOutputImage=itk::Image<itk::RGBAPixel<unsigned char>,3> >
                      class OdfToRgbImageFilter :
                      public ImageToImageFilter<TInputImage,TOutputImage>
                      {
                      public:
  /** Standard class typedefs. */
  typedef OdfToRgbImageFilter  Self;
  typedef ImageToImageFilter<TInputImage,TOutputImage>
      Superclass;
  typedef SmartPointer<Self>                     Pointer;
  typedef SmartPointer<const Self>               ConstPointer;

  typedef typename Superclass::InputImageType    InputImageType;
  typedef typename Superclass::OutputImageType    OutputImageType;
  typedef typename OutputImageType::PixelType     OutputPixelType;
  typedef typename TInputImage::PixelType         InputPixelType;
  typedef typename InputPixelType::ValueType      InputValueType;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( OdfToRgbImageFilter, ImageToImageFilter )

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Print internal ivars */
  void PrintSelf(std::ostream& os, Indent indent) const override
  { this->Superclass::PrintSelf( os, indent ); }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputHasNumericTraitsCheck,
                  (Concept::HasNumericTraits<InputValueType>));
  /** End concept checking */
#endif

protected:
  OdfToRgbImageFilter(){}
  ~OdfToRgbImageFilter() override{}

  void GenerateData() override
  {

    typename InputImageType::Pointer OdfImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

    typename InputImageType::RegionType region = OdfImage->GetLargestPossibleRegion();

    outputImage->SetSpacing( OdfImage->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( OdfImage->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( OdfImage->GetDirection() );  // Set the image direction
    outputImage->SetRegions( OdfImage->GetLargestPossibleRegion());
    outputImage->Allocate();

    typedef ImageRegionConstIterator< InputImageType > OdfImageIteratorType;
    OdfImageIteratorType OdfIt(OdfImage, OdfImage->GetLargestPossibleRegion());

    typedef ImageRegionIterator< OutputImageType > OutputImageIteratorType;
    OutputImageIteratorType outputIt(outputImage, outputImage->GetLargestPossibleRegion());

    OdfIt.GoToBegin();
    outputIt.GoToBegin();

    while(!OdfIt.IsAtEnd() && !outputIt.IsAtEnd()){

      InputPixelType x = OdfIt.Get();
      typedef itk::OrientationDistributionFunction<float,ODF_SAMPLING_SIZE> OdfType;
      OdfType odf(x.GetDataPointer());

      vnl_vector_fixed<double,3> dir;
      int pd = odf.GetPrincipalDiffusionDirectionIndex();
      if (pd==-1)
        dir.fill(0);
      else
        dir = OdfType::GetDirection(pd);

      const float fa = odf.GetGeneralizedFractionalAnisotropy();
      float r = fabs(dir[0]) * fa;
      float g = fabs(dir[1]) * fa;
      float b = fabs(dir[2]) * fa;
//      float a = (fa-(m_OpacLevel-m_OpacWindow/2.0f))/m_OpacWindow;
      float a = fa;

      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(r);
      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(g);
      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(b);
      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(a);

      OutputPixelType out;
      out.SetRed(   r * 255.0f);
      out.SetGreen( g * 255.0f);
      out.SetBlue(  b * 255.0f);
      out.SetAlpha( a * 255.0f);

      outputIt.Set(out);

      ++OdfIt;
      ++outputIt;
    }

  }

private:
  OdfToRgbImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // end namespace itk

#endif
