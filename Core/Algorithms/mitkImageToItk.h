#ifndef IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2
#define IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

#include <itkImage.h>
#include <itkImageSource.h>
#include "mitkImage.h"

namespace mitk
{
	
template <typename TPixel, unsigned int VImageDimension=2>
class ITK_EXPORT ImageToItk : public itk::ImageSource< itk::Image<TPixel,VImageDimension> >
{
protected:
	mitk::Image::Pointer m_MitkImage;
	mitk::ImageDataItem::Pointer m_ImageDataItem;
  
public:
  typedef itk::Image<TPixel,VImageDimension> TOutputImage;
	mitkClassMacro(ImageToItk, itk::ImageSource<TOutputImage >);

	/** Method for creation through the object factory. */
	itkNewMacro(Self);  
	
	/** Superclass typedefs. */
	typedef typename Superclass::OutputImageRegionType OutputImageRegionType;

	/** Some convenient typedefs. */
	typedef mitk::Image                    InputImageType;
	typedef InputImageType::Pointer        InputImagePointer;
	typedef InputImageType::ConstPointer   InputImageConstPointer;
	typedef SlicedData::RegionType         InputImageRegionType;
  typedef typename TOutputImage::SizeType         SizeType;
  typedef typename TOutputImage::IndexType        IndexType;
  typedef typename TOutputImage::RegionType       RegionType;
  typedef typename TOutputImage::PixelType        PixelType;
  
	const mitk::Image * GetInput(void);
	const mitk::Image * GetInput(unsigned int idx);

  virtual void SetInput(const mitk::Image *input);
  virtual void SetInput(unsigned int index, const mitk::Image * image);
	//void SetMitkImage(mitk::Image::Pointer image, int n = 0);

  itkGetMacro(Channel, int);
  itkSetMacro(Channel, int);
	
protected:
	ImageToItk();
	
	virtual ~ImageToItk();

	void PrintSelf(std::ostream& os, itk::Indent indent) const;
	

  virtual void GenerateData();

  virtual void GenerateOutputInformation();
  
  void SetCopyMemFlagOn();

  void SetCopyMemFlagOff();

  void SetCopyMemFlag(bool flag);

private:
	bool m_CopyMemFlag;
  int m_Channel;
  
	//ImageToItk(const Self&); //purposely not implemented
	void operator=(const Self&); //purposely not implemented

};


} // end namespace itk



#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkImageToItk.txx"
#endif

#endif // IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

