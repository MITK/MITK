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

#ifndef __itkShToRgbImageFilter_h
#define __itkShToRgbImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkOrientationDistributionFunction.h"
#include "itkRGBAPixel.h"
#include "itkImageRegionConstIterator.h"
#include "itkImageRegionIterator.h"
#include <mitkDiffusionFunctionCollection.h>

namespace itk
{

  #define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );


/** \class ShToRgbImageFilter
*
*/
template <typename TInputImage, int ShOrder,
typename TOutputImage=itk::Image<itk::RGBAPixel<unsigned char>,3> >
                      class ShToRgbImageFilter :
                      public ImageToImageFilter<TInputImage,TOutputImage>
                      {
                      public:
  /** Standard class typedefs. */
  typedef ShToRgbImageFilter  Self;
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
  itkTypeMacro( ShToRgbImageFilter, ImageToImageFilter )

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
  ShToRgbImageFilter(){}
  ~ShToRgbImageFilter() override{}

  void ThreadedGenerateData( const typename OutputImageType::RegionType &outputRegionForThread, ThreadIdType) override
  {
    typename InputImageType::Pointer coeff_image = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );
    typename OutputImageType::Pointer outputImage = static_cast< OutputImageType * >(this->ProcessObject::GetPrimaryOutput());

    outputImage->SetSpacing( coeff_image->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( coeff_image->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( coeff_image->GetDirection() );  // Set the image direction
    outputImage->SetRegions( coeff_image->GetLargestPossibleRegion());
    outputImage->Allocate();

    typedef ImageRegionConstIterator< InputImageType > OdfImageIteratorType;
    OdfImageIteratorType it(coeff_image, outputRegionForThread);

    typedef ImageRegionIterator< OutputImageType > OutputImageIteratorType;
    OutputImageIteratorType oit(outputImage, outputRegionForThread);

    it.GoToBegin();
    oit.GoToBegin();

    typedef itk::OrientationDistributionFunction<float,ODF_SAMPLING_SIZE> OdfType;
    vnl_matrix<float> sh2Basis =  mitk::sh::CalcShBasisForDirections(ShOrder, itk::PointShell<ODF_SAMPLING_SIZE, vnl_matrix_fixed<double, 3, ODF_SAMPLING_SIZE> >::DistributePointShell()->as_matrix());

    while(!it.IsAtEnd() && !oit.IsAtEnd())
    {
      InputPixelType x = it.Get();

      Vector< float, ODF_SAMPLING_SIZE > odf_vals;
      vnl_vector< float > coeffs(x.GetNumberOfComponents());
      for(unsigned int i=0; i<x.GetNumberOfComponents(); i++)
        coeffs[i] = (float)x[i];
      odf_vals = ( sh2Basis * coeffs ).data_block();

      OdfType odf;
      for(int i=0; i<ODF_SAMPLING_SIZE; i++)
        odf[i] = odf_vals[i];

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

      oit.Set(out);

      ++it;
      ++oit;
    }

  }

private:
  ShToRgbImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // end namespace itk

#endif
