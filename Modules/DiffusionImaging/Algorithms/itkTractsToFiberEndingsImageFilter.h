#ifndef __itkTractsToFiberEndingsImageFilter_h__
#define __itkTractsToFiberEndingsImageFilter_h__

#include "itkImageToImageFilter.h"
#include "itkVectorContainer.h"
#include "itkRGBAPixel.h"
#include "mitkFiberBundle.h"

namespace itk{

template< class TInputImage, class TOutputPixelType >
class TractsToFiberEndingsImageFilter :
    public ImageToImageFilter<TInputImage, itk::Image<TOutputPixelType,3> >
//huhu    public ImageToImageFilter<TInputImage, itk::Image<float,3> >
{

public:
  typedef TractsToFiberEndingsImageFilter Self;
  typedef ImageToImageFilter<TInputImage, itk::Image<TOutputPixelType,3> > Superclass;
//huhu  typedef ImageToImageFilter<TInputImage, itk::Image<float,3> > Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;
  
  itkNewMacro(Self);
  itkTypeMacro( TractsToFiberEndingsImageFilter,
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
  
  itkSetMacro(FiberBundle, mitk::FiberBundle::Pointer);
  itkGetMacro(FiberBundle, mitk::FiberBundle::Pointer);

  void GenerateData();
  
protected:

  TractsToFiberEndingsImageFilter();
  virtual ~TractsToFiberEndingsImageFilter();

  unsigned int m_UpsamplingFactor;
  mitk::FiberBundle::Pointer m_FiberBundle;

};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToFiberEndingsImageFilter.cpp"
#endif

#endif // __itkTractsToFiberEndingsImageFilter_h__
