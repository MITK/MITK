#ifndef ITKIMAGETOIMAGEFILTERADAPTER_H_HEADER_INCLUDED_C1E5E869
#define ITKIMAGETOIMAGEFILTERADAPTER_H_HEADER_INCLUDED_C1E5E869

#include "mitkCommon.h"
#include "mitkImageToImageFilter.h"

#include "itkImage.h"
#include "itkImageToImageFilter.h"

namespace mitk {

//##Documentation
//## @brief adapter class, that encapsulates any itk::ImageToImageFilter
//## This filter converts a mitk::Image to an itk::Image, executes the 
//## itkFilter and converts the itk::Image back to a mitk::Image as output
//## @ingroup Process
class ItkImageToImageFilterAdapter : public ImageToImageFilter
{
public:
  typedef short PixelType;
  typedef itk::Image<PixelType, 3> ItkImageType;
  typedef itk::ImageToImageFilter<ItkImageType, ItkImageType> ImageToImageFilterType;

	mitkClassMacro(ItkImageToImageFilterAdapter,ImageSource);
	/** Method for creation through the object factory. */
	itkNewMacro(Self);

  itkSetObjectMacro(ItkImageToImageFilter, ImageToImageFilterType);
  itkGetObjectMacro(ItkImageToImageFilter, ImageToImageFilterType);

  virtual void GenerateOutputInformation();
  virtual void GenerateData();


protected:
	ItkImageToImageFilterAdapter();
	virtual ~ItkImageToImageFilterAdapter();

  ImageToImageFilterType::Pointer m_ItkImageToImageFilter;
};

} // namespace mitk

#endif /* ITKIMAGETOIMAGEFILTERADAPTER_H_HEADER_INCLUDED_C1E5E869 */
