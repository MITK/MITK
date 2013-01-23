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

/*===================================================================

This file is based heavily on a corresponding ITK filter.

===================================================================*/
#ifndef __itkGaussianInterpolateImageFunction_h
#define __itkGaussianInterpolateImageFunction_h

#include "itkInterpolateImageFunction.h"

#include "itkConceptChecking.h"
#include "itkFixedArray.h"
#include "vnl/vnl_erf.h"

namespace itk
{

/** \class GaussianInterpolateImageFunction
 * \brief Evaluates the Gaussian interpolation of an image.
 *
 * This class defines an N-dimensional Gaussian interpolation function using
 * the vnl error function.  The two parameters associated with this function
 * are:
 *   1. Sigma - a scalar array of size ImageDimension determining the width
 *      of the interpolation function.
 *   2. Alpha - a scalar specifying the cutoff distance over which the function
 *      is calculated.
 *
 * \ingroup ImageFunctions ImageInterpolators
 */

template <class TInputImage, class TCoordRep = double>
class ITK_EXPORT GaussianInterpolateImageFunction :
  public InterpolateImageFunction<TInputImage, TCoordRep>
{
public:
  /** Standard class typedefs. */
  typedef GaussianInterpolateImageFunction                 Self;
  typedef InterpolateImageFunction<TInputImage, TCoordRep> Superclass;
  typedef SmartPointer<Self>                               Pointer;
  typedef SmartPointer<const Self>                         ConstPointer;

  /** Run-time type information (and related methods). */
  itkTypeMacro( GaussianInterpolateImageFunction, InterpolateImageFunction );

  /** Method for creation through the object factory. */
  itkNewMacro( Self );

  /** ImageDimension constant */
  itkStaticConstMacro( ImageDimension, unsigned int,
    TInputImage::ImageDimension );


  /** OutputType typedef support. */
  typedef typename Superclass::OutputType OutputType;

  /** InputImageType typedef support. */
  typedef typename Superclass::InputImageType InputImageType;

  /** RealType typedef support. */
  typedef typename Superclass::RealType RealType;

  /** Index typedef support. */
  typedef typename Superclass::IndexType IndexType;

  /** ContinuousIndex typedef support. */
  typedef typename Superclass::ContinuousIndexType ContinuousIndexType;

  /** Array typedef support */
  typedef FixedArray<RealType,
    itkGetStaticConstMacro( ImageDimension )> ArrayType;

  /**
   * Set input image
   */
  virtual void SetInputImage( const TInputImage *image )
    {
    Superclass::SetInputImage( image );
    this->ComputeBoundingBox();
    }

  /**
   * Set/Get ivars
   */
  virtual void SetSigma( const ArrayType s )
    {
    itkDebugMacro( "setting Sigma to " << s );
    if( this->m_Sigma != s )
      {
      this->m_Sigma = s;
      this->ComputeBoundingBox();
      this->Modified();
      }
    }
  itkGetConstMacro( Sigma, ArrayType );
  void SetSigma( RealType *s )
    {
    ArrayType sigma;
    for( unsigned int d = 0; d < ImageDimension; d++ )
      {
      sigma[d] = s[d];
      }
    this->SetSigma( sigma );
    }

  virtual void SetAlpha( const RealType a )
    {
    itkDebugMacro( "setting Alpha to " << a );
    if( this->m_Alpha != a )
      {
      this->m_Alpha = a;
      this->ComputeBoundingBox();
      this->Modified();
      }
    }
  itkGetConstMacro( Alpha, RealType );

  void SetParameters( RealType *sigma, RealType alpha )
    {
    this->SetSigma( sigma );
    this->SetAlpha( alpha );
    }

  /**
   * Evaluate at the given index
   */
  virtual OutputType EvaluateAtContinuousIndex(
    const ContinuousIndexType & cindex ) const
    {
    return this->EvaluateAtContinuousIndex( cindex, NULL );
    }

  /**
   * Evaluate function value and gradient at the given index
   */
  virtual OutputType EvaluateAtContinuousIndex(
    const ContinuousIndexType &, OutputType * ) const;

protected:
  GaussianInterpolateImageFunction();
  ~GaussianInterpolateImageFunction(){};
  void PrintSelf( std::ostream& os, Indent indent ) const;

private:
  GaussianInterpolateImageFunction( const Self& ); //purposely not implemented
  void operator=( const Self& ); //purposely not implemented

  void ComputeBoundingBox();

  void ComputeErrorFunctionArray( unsigned int dimension, RealType cindex,
    vnl_vector<RealType> &erfArray, vnl_vector<RealType> &gerfArray,
    bool evaluateGradient = false ) const;

  ArrayType                                 m_Sigma;
  RealType                                  m_Alpha;

  ArrayType                                 m_BoundingBoxStart;
  ArrayType                                 m_BoundingBoxEnd;
  ArrayType                                 m_ScalingFactor;
  ArrayType                                 m_CutoffDistance;
};

} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkGaussianInterpolateImageFunction.txx"
#endif

#endif
