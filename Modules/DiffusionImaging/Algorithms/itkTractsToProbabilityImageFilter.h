#ifndef __itkTractsToProbabilityImageFilter_h__
#define __itkTractsToProbabilityImageFilter_h__

#include "itkImageToImageFilter.h"
#include "itkVectorContainer.h"
#include "itkRGBAPixel.h"
#include "mitkFiberBundle.h"

namespace itk{

template< class TInputImage, class TOutputPixelType >
class TractsToProbabilityImageFilter :
    public ImageToImageFilter<TInputImage, itk::Image<TOutputPixelType,3> >
//huhu    public ImageToImageFilter<TInputImage, itk::Image<float,3> >
{

public:
  typedef TractsToProbabilityImageFilter Self;
  typedef ImageToImageFilter<TInputImage, itk::Image<TOutputPixelType,3> > Superclass;
//huhu  typedef ImageToImageFilter<TInputImage, itk::Image<float,3> > Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro( TractsToProbabilityImageFilter,
            ImageToImageFilter );

  /** Types for the Output Image**/
  typedef TInputImage InputImageType;

  /** Types for the Output Image**/
  typedef itk::Image<TOutputPixelType,3> OutputImageType;

  /** Type for the directions **/
  typedef VectorContainer< unsigned int, vnl_vector_fixed< double, 3 > >
    TractOrientationsType;

  /** Type for the directions container **/
  typedef VectorContainer< unsigned int, TractOrientationsType >
    TractOrientationsContainerType;

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, unsigned int);
  itkGetMacro( UpsamplingFactor, unsigned int);

  /** Upsampling factor **/
  itkSetMacro( InvertImage, bool);
  itkGetMacro( InvertImage, bool);

  itkSetMacro(FiberBundle, mitk::FiberBundle::Pointer);
  itkGetMacro(FiberBundle, mitk::FiberBundle::Pointer);

  itkSetMacro( BinaryEnvelope, bool);
  itkGetMacro( BinaryEnvelope, bool);

  void GenerateData();

protected:

  TractsToProbabilityImageFilter();
  virtual ~TractsToProbabilityImageFilter();

  unsigned int m_UpsamplingFactor;
  mitk::FiberBundle::Pointer m_FiberBundle;

  bool m_InvertImage;
  bool m_BinaryEnvelope;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToProbabilityImageFilter.cpp"
#endif

#endif // __itkTractsToProbabilityImageFilter_h__
