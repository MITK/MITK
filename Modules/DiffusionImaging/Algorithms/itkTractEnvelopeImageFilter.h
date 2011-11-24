#ifndef __itkTractDensityImageFilter_h__
#define __itkTractDensityImageFilter_h__

#include <itkImageToImageFilter.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundleX.h>

namespace itk{

template< class TInputImage, class TOutputPixelType >
class TractDensityImageFilter :
    public ImageToImageFilter<TInputImage, itk::Image<TOutputPixelType,3> >
{

public:
  typedef TractDensityImageFilter Self;
  typedef ImageToImageFilter<TInputImage, itk::Image<TOutputPixelType,3> > Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  itkNewMacro(Self);
  itkTypeMacro( TractDensityImageFilter,
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

  itk::Point<float, 3> GetItkPoint(double point[3]);

  TractDensityImageFilter();
  virtual ~TractDensityImageFilter();

  mitk::FiberBundleX::Pointer m_FiberBundle;
  unsigned int m_UpsamplingFactor;
  bool m_InvertImage;
  bool m_BinaryEnvelope;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractDensityImageFilter.cpp"
#endif

#endif // __itkTractDensityImageFilter_h__
