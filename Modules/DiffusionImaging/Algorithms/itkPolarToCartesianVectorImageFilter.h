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

#ifndef __itkPolarToCartesianVectorImageFilter_h
#define __itkPolarToCartesianVectorImageFilter_h

#include "itkUnaryFunctorImageFilter.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define P2C_PI M_PI

namespace itk
{

  namespace Functor {

    template< typename TInput, typename TOutput, bool symmetric >
    class PolarToCartesianFunction
    {
    public:
      PolarToCartesianFunction() {}
      ~PolarToCartesianFunction() {}
      bool operator!=( const PolarToCartesianFunction & ) const
      {
        return false;
      }
      bool operator==( const PolarToCartesianFunction & other ) const
      {
        return !(*this != other);
      }
      inline TOutput operator()( const TInput & x )
      {

        TOutput opoint;
        opoint[0] = x[0] * cos( x[1] ) * sin( x[2] );
        opoint[1] = x[0] * sin( x[1] ) * sin( x[2] );
        opoint[2] = x[0] * cos( x[2] );

        if(symmetric && opoint[2]<0)
        {
          opoint[2] = -opoint[2];
        }

        return opoint;
        ;
      }
    };

  }  // end namespace functor


  /** \class PolarToCartesianVectorImageFilter
 *
 */
  template <typename TInputImage, typename TOutputImage, bool symmetric>
      class PolarToCartesianVectorImageFilter :
      public
      UnaryFunctorImageFilter<TInputImage,TOutputImage,
      Functor::PolarToCartesianFunction<
      typename TInputImage::PixelType,
      typename TOutputImage::PixelType, symmetric> >
  {
  public:
    /** Standard class typedefs. */
    typedef PolarToCartesianVectorImageFilter  Self;
    typedef UnaryFunctorImageFilter<
        TInputImage,TOutputImage,
        Functor::PolarToCartesianFunction<
        typename TInputImage::PixelType,
        typename TOutputImage::PixelType, symmetric > >       Superclass;
    typedef SmartPointer<Self>                     Pointer;
    typedef SmartPointer<const Self>               ConstPointer;

    typedef typename Superclass::OutputImageType    OutputImageType;
    typedef typename OutputImageType::PixelType     OutputPixelType;
    typedef typename TInputImage::PixelType         InputPixelType;
    typedef typename InputPixelType::ValueType      InputValueType;

    /** Run-time type information (and related methods).   */
    itkTypeMacro( PolarToCartesianVectorImageFilter, UnaryFunctorImageFilter );

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
    PolarToCartesianVectorImageFilter() {};
    virtual ~PolarToCartesianVectorImageFilter() {};

  private:
    PolarToCartesianVectorImageFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

  };



} // end namespace itk

#endif // __itkPolarToCartesianVectorImageFilter_h
