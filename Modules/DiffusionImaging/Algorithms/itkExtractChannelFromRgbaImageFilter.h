#ifndef __itkExtractChannelFromRgbaImageFilter_h__
#define __itkExtractChannelFromRgbaImageFilter_h__

#include <itkImageToImageFilter.h>
#include <itkImage.h>
#include <itkRGBAPixel.h>
#include <itkVectorImage.h>

namespace itk{

template< class OutputImageType >
class ExtractChannelFromRgbaImageFilter : public ImageToImageFilter< itk::Image<itk::RGBAPixel<unsigned char>,3>, OutputImageType >
{

public:

  enum Channel {
    RED,
    GREEN,
    BLUE,
    ALPHA
  };

  typedef ExtractChannelFromRgbaImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef itk::Image<itk::RGBAPixel<unsigned char>,3> InputImageType;
  typedef typename OutputImageType::PixelType OutPixelType;
  typedef typename InputImageType::PixelType InPixelType;

  typedef itk::VectorImage< short int, 3 > ReferenceImageType;

  itkNewMacro(Self);
  itkTypeMacro( ExtractChannelFromRgbaImageFilter, ImageSource );

  /** Upsampling factor **/

  void GenerateData();

  itkSetMacro(Channel, Channel)
  itkSetMacro(ReferenceImage, ReferenceImageType::Pointer)

protected:

  ExtractChannelFromRgbaImageFilter();
  virtual ~ExtractChannelFromRgbaImageFilter();
  Channel m_Channel;
  ReferenceImageType::Pointer m_ReferenceImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkExtractChannelFromRgbaImageFilter.cpp"
#endif

#endif // __itkExtractChannelFromRgbaImageFilter_h__
