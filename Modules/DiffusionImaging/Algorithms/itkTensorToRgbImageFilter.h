/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkTensorFractionalAnisotropyImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2008-10-18 16:11:14 $
  Version:   $Revision: 1.8 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkTensorToRgbImageFilter_h
#define __itkTensorToRgbImageFilter_h

#include "itkUnaryFunctorImageFilter.h"

namespace itk
{

  #define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );

// This functor class invokes the computation of fractional anisotropy from
// every pixel.
namespace Functor {  

  template< typename TInput, typename TOutput >
  class TensorToRgbFunction
  {
  public:
    TensorToRgbFunction() {}
    ~TensorToRgbFunction() {}
    bool operator!=( const TensorToRgbFunction & ) const
    {
      return false;
    }
    bool operator==( const TensorToRgbFunction & other ) const
    {
      return !(*this != other);
    }
    inline TOutput operator()( const TInput & x )
    {

      typename TInput::EigenValuesArrayType eigenvalues;
      typename TInput::EigenVectorsMatrixType eigenvectors;
      x.ComputeEigenAnalysis(eigenvalues, eigenvectors);

      int index = 2;
      if( (eigenvalues[0] >= eigenvalues[1])
        && (eigenvalues[0] >= eigenvalues[2]) )
        index = 0;
      else if(eigenvalues[1] >= eigenvalues[2])
        index = 1;

      const float fa = x.GetFractionalAnisotropy();
      float r = fabs(eigenvectors(index,0)) * fa;
      float g = fabs(eigenvectors(index,1)) * fa;
      float b = fabs(eigenvectors(index,2)) * fa;

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


/** \class TensorToRgbImageFilter
 *
 */
template <typename TInputImage, 
          typename TOutputImage=itk::Image<itk::RGBPixel<unsigned char>,3> >
class TensorToRgbImageFilter :
    public
    UnaryFunctorImageFilter<TInputImage,TOutputImage, 
                        Functor::TensorToRgbFunction< 
                                        typename TInputImage::PixelType,
                                        typename TOutputImage::PixelType> > 
{
public:
  /** Standard class typedefs. */
  typedef TensorToRgbImageFilter  Self;
  typedef UnaryFunctorImageFilter<
    TInputImage,TOutputImage, 
    Functor::TensorToRgbFunction< 
      typename TInputImage::PixelType,
      typename TOutputImage::PixelType > >       Superclass;
  typedef SmartPointer<Self>                     Pointer;
  typedef SmartPointer<const Self>               ConstPointer;

  typedef typename Superclass::OutputImageType    OutputImageType;
  typedef typename OutputImageType::PixelType     OutputPixelType;
  typedef typename TInputImage::PixelType         InputPixelType;
  typedef typename InputPixelType::ValueType      InputValueType;

  /** Run-time type information (and related methods).   */
  itkTypeMacro( TensorToRgbImageFilter, UnaryFunctorImageFilter );

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
  TensorToRgbImageFilter() {};
  virtual ~TensorToRgbImageFilter() {};

private:
  TensorToRgbImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented

};


  
} // end namespace itk
  
#endif
