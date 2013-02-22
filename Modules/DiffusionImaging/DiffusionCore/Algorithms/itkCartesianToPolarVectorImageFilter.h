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

#ifndef __itkCartesianToPolarVectorImageFilter_h
#define __itkCartesianToPolarVectorImageFilter_h

#include "itkUnaryFunctorImageFilter.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define C2P_PI M_PI

namespace itk
{

  namespace Functor {

    template< typename TInput, typename TOutput, bool symmetric >
    class CartesianToPolarFunction
    {
    public:
      CartesianToPolarFunction() {}
      ~CartesianToPolarFunction() {}
      bool operator!=( const CartesianToPolarFunction & ) const
      {
        return false;
      }
      bool operator==( const CartesianToPolarFunction & other ) const
      {
        return !(*this != other);
      }
      inline TOutput operator()( const TInput & x )
      {

        TOutput opoint;

        if(x[0] || x[1] || x[2])
        {
          opoint[0] = sqrt( x[0] * x[0] + x[1] * x[1] + x[2] * x[2] );
          opoint[1] = atan2( x[1], x[0] );
          opoint[2] = 0.5*C2P_PI - atan( x[2] / sqrt( x[0] * x[0] + x[1] * x[1] ) );

          if(symmetric && opoint[1]>C2P_PI)
          {
            opoint[1] = opoint[1] - C2P_PI;
          }
        }
        else
        {
          opoint[0] = 0;
          opoint[1] = 0;
          opoint[2] = 0;
        }
        return opoint;

      }
    };

  }  // end namespace functor


  /** \class CartesianToPolarVectorImageFilter
 *
 */
  template <typename TInputImage, typename TOutputImage, bool symmetric>
      class CartesianToPolarVectorImageFilter :
      public
      UnaryFunctorImageFilter<TInputImage,TOutputImage,
      Functor::CartesianToPolarFunction<
      typename TInputImage::PixelType,
      typename TOutputImage::PixelType, symmetric> >
  {
  public:
    /** Standard class typedefs. */
    typedef CartesianToPolarVectorImageFilter  Self;
    typedef UnaryFunctorImageFilter<
        TInputImage,TOutputImage,
        Functor::CartesianToPolarFunction<
        typename TInputImage::PixelType,
        typename TOutputImage::PixelType, symmetric > >       Superclass;
    typedef SmartPointer<Self>                     Pointer;
    typedef SmartPointer<const Self>               ConstPointer;

    typedef typename Superclass::OutputImageType    OutputImageType;
    typedef typename OutputImageType::PixelType     OutputPixelType;
    typedef typename TInputImage::PixelType         InputPixelType;
    typedef typename InputPixelType::ValueType      InputValueType;

    /** Run-time type information (and related methods).   */
    itkTypeMacro( CartesianToPolarVectorImageFilter, UnaryFunctorImageFilter );

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
    CartesianToPolarVectorImageFilter() {};
    virtual ~CartesianToPolarVectorImageFilter() {};

  private:
    CartesianToPolarVectorImageFilter(const Self&); //purposely not implemented
    void operator=(const Self&); //purposely not implemented

  };



} // end namespace itk

#endif // __itkCartesianToPolarVectorImageFilter_h
