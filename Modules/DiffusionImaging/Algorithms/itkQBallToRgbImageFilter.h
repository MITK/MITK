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

namespace itk
{

  #define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );

// This functor class invokes the computation of fractional anisotropy from
// every pixel.
namespace Functor {  

  template< typename TInput, typename TOutput >
  class QBallToRgbFunction
  {
  public:
    QBallToRgbFunction() {}
    ~QBallToRgbFunction() {}
    bool operator!=( const QBallToRgbFunction & ) const
    {
      return false;
    }
    bool operator==( const QBallToRgbFunction & other ) const
    {
      return !(*this != other);
    }
    inline TOutput operator()( const TInput & x )
    {
      typedef itk::OrientationDistributionFunction<float,QBALL_ODFSIZE> OdfType;
      OdfType odf(x.GetDataPointer());
      //for( unsigned int i=0; i<InternalDimension; i++) 
      //{
      //  result[i] = (*this)[i] + r[i];
      //}
      int pd = odf.GetPrincipleDiffusionDirection();
      vnl_vector_fixed<double,3> dir = OdfType::GetDirection(pd);
      
      const float fa = odf.GetGeneralizedFractionalAnisotropy();
      float r = fabs(dir[0]) * fa;
      float g = fabs(dir[1]) * fa;
      float b = fabs(dir[2]) * fa;

      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(r);
      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(g);
      __IMG_DAT_ITEM__CEIL_ZERO_ONE__(b);

      TOutput out;
      out.SetRed(   r * 255.0f);
      out.SetGreen( g * 255.0f);
      out.SetBlue(  b * 255.0f);

      return out;
    }
  }; 

}  // end namespace functor


/** \class QBallToRgbImageFilter
 *
 */
template <typename TInputImage, 
          typename TOutputImage=itk::Image<itk::RGBPixel<unsigned char>,3> >
class QBallToRgbImageFilter :
    public
    UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                        Functor::QBallToRgbFunction< 
                                        typename TInputImage::PixelType,
                                        typename TOutputImage::PixelType> > 
{
public:
  /** Standard class typedefs. */
  typedef QBallToRgbImageFilter  Self;
  typedef UnaryFunctorImageFilter<
    TInputImage,TOutputImage, 
    Functor::QBallToRgbFunction< 
      typename TInputImage::PixelType,
      typename TOutputImage::PixelType > >       Superclass;
  typedef SmartPointer<Self>                     Pointer;
  typedef SmartPointer<const Self>               ConstPointer;

  typedef typename Superclass::OutputImageType    OutputImageType;
  typedef typename OutputImageType::PixelType     OutputPixelType;
  typedef typename TInputImage::PixelType         InputPixelType;
  typedef typename InputPixelType::ValueType      InputValueType;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( QBallToRgbImageFilter, UnaryFunctorImageFilter );

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

private:
  QBallToRgbImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


  
} // end namespace itk
  
#endif
