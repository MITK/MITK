/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-07-14 19:11:20 +0200 (Tue, 14 Jul 2009) $
Version:   $Revision: 18127 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef __itkQBallToRgbImageFilter_h
#define __itkQBallToRgbImageFilter_h

#include "itkUnaryFunctorImageFilter.h"
#include "itkOrientationDistributionFunction.h"
#include "itkRGBAPixel.h"

namespace itk
{

  #define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );


/** \class QBallToRgbImageFilter
*
*/
template <typename TInputImage,
typename TOutputImage=itk::Image<itk::RGBAPixel<unsigned char>,3> >
                      class QBallToRgbImageFilter :
                      public ImageToImageFilter<TInputImage,TOutputImage>
                      {
                      public:
  /** Standard class typedefs. */
  typedef QBallToRgbImageFilter  Self;
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
  itkTypeMacro( QBallToRgbImageFilter, ImageToImageFilter );

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Print internal ivars */
  void PrintSelf(std::ostream& os, Indent indent) const
  { this->Superclass::PrintSelf( os, indent ); }

#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputHasNumericTraitsCheck,
                  (Concept::HasNumericTraits<InputValueType>));
  /** End concept checking */
#endif

protected:
  QBallToRgbImageFilter() {};
  virtual ~QBallToRgbImageFilter() {};

  void GenerateData()
  {

    typename InputImageType::Pointer qballImage = static_cast< InputImageType * >( this->ProcessObject::GetInput(0) );

    typename OutputImageType::Pointer outputImage =
        static_cast< OutputImageType * >(this->ProcessObject::GetOutput(0));

    typename InputImageType::RegionType region = qballImage->GetLargestPossibleRegion();

    outputImage->SetSpacing( qballImage->GetSpacing() );   // Set the image spacing
    outputImage->SetOrigin( qballImage->GetOrigin() );     // Set the image origin
    outputImage->SetDirection( qballImage->GetDirection() );  // Set the image direction
    outputImage->SetRegions( qballImage->GetLargestPossibleRegion());
    outputImage->Allocate();

    typedef ImageRegionConstIterator< InputImageType > QBallImageIteratorType;
    QBallImageIteratorType qballIt(qballImage, qballImage->GetLargestPossibleRegion());

    typedef ImageRegionIterator< OutputImageType > OutputImageIteratorType;
    OutputImageIteratorType outputIt(outputImage, outputImage->GetLargestPossibleRegion());

    qballIt.GoToBegin();
    outputIt.GoToBegin();

    while(!qballIt.IsAtEnd() && !outputIt.IsAtEnd()){

      InputPixelType x = qballIt.Get();
      typedef itk::OrientationDistributionFunction<float,QBALL_ODFSIZE> OdfType;
      OdfType odf(x.GetDataPointer());

      int pd = odf.GetPrincipleDiffusionDirection();
      if (pd==-1)
        MITK_ERROR << "ODF corrupted: GetPrincipleDiffusionDirection returned -1";

      vnl_vector_fixed<double,3> dir = OdfType::GetDirection(pd);

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

      ++qballIt;
      ++outputIt;
    }

  }

private:
  QBallToRgbImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

} // end namespace itk

#endif
