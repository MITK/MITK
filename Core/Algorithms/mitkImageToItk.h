#ifndef IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2
#define IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

#include <itkImage.h>
#include "mitkImage.h"

namespace mitk
{
	
template <class TPixel, unsigned int VImageDimension=2>
class ITK_EXPORT ImageToItk : public itk::Image<TPixel,VImageDimension>
{
protected:
//	mitk::Image::Pointer m_MitkImage;
	mitk::ImageDataItem::Pointer m_ImageDataItem;
public:
	/** Standard class typedefs */
	typedef ImageToItk                           Self;
	typedef itk::Image<TPixel, VImageDimension>  Superclass;
	
	typedef itk::SmartPointer<Self>  Pointer;
	typedef itk::SmartPointer<const Self>  ConstPointer;
	
	/** Method for creation through the object factory. */
	itkNewMacro(Self);  
	
	/** Run-time type information (and related methods). */
	itkTypeMacro(ImageToItk, itk::Image);
	
	/** set the @a mitk::Image that shall be accessed from itk via this itk-wrapper-class.
	* @param n channel number to be accessed
	*/
	void SetMitkImage(mitk::Image *image, int n = 0)
	{
//		m_MitkImage = image;

		if(image==NULL)
			itkExceptionMacro( << "image is null" );

		if(image->GetDimension()!=VImageDimension)
			itkExceptionMacro( << "image has dimension " << image->GetDimension() << " instead of " << VImageDimension );

		if(typeid(TPixel) != *image->GetType().GetTypeId())
			itkExceptionMacro( << "image has wrong pixel type " );

		itkDebugMacro("ImageToItk::SetImageToItk ...");
		
		SizeType  size;
		double origin[ VImageDimension ];
		double spacing[ VImageDimension ];
		
		for (int i=0; i < VImageDimension; ++i) 
		{
			size[i]    = image->GetDimension(i);
			spacing[i] = image->GetGeometry()->GetSpacing()[i];
		}
		
		mitk::Point3D mitkorigin(0,0,0);
		image->GetGeometry()->UnitsToMM(mitkorigin, mitkorigin);
		origin[0] = mitkorigin.x;    // X coordinate
		origin[1] = mitkorigin.y;    // Y coordinate
		origin[2] = mitkorigin.z;    // Z coordinate
		
		IndexType start;
		start.Fill( 0 );
		
		RegionType region;
		region.SetIndex( start );
		region.SetSize(  size  );
		
		unsigned long noBytes = size[0];
		for (int i=1; i<VImageDimension; ++i) 
		{
			noBytes = noBytes * size[i];
		}
		
		if (m_CopyMemFlag)
		{
			itkDebugMacro("copyMem ...");
			
			this->SetRegions( region );
			this->SetOrigin( origin );
			this->SetSpacing( spacing );
			
			this->Allocate();
			memcpy( (TPixel *) this->GetBufferPointer(),
				image->GetData(), sizeof(TPixel)*noBytes);
			
		}
		else 
		{
			m_ImageDataItem = image->GetChannelData(n);

			itkDebugMacro("do not copyMem ...");
			
			typedef itk::ImportImageContainer< unsigned long, TPixel >   ImportContainerType;
			ImportContainerType::Pointer import;
			
			import = ImportContainerType::New();
			import->Initialize();
			
			itkDebugMacro( << "size of container = " << import->Size() );
			import->SetImportPointer( (TPixel*) m_ImageDataItem->GetData(),	noBytes, false);
			
			this->SetRegions( region );
			this->SetOrigin( origin );
			this->SetSpacing( spacing );
			this->SetPixelContainer(import);
			//this->Update();
			itkDebugMacro( << "size of container = " << import->Size() );
		}
		
		itkDebugMacro( << "ImageToItk::SetMitkImage OK!" );
	}
	
	void SetCopyMemFlagOn()
	{
		m_CopyMemFlag = true;
	}
	
	void SetCopyMemFlagOff()
	{
		m_CopyMemFlag = false;
	}
	
	void SetCopyMemFlag(bool flag)
	{
		m_CopyMemFlag = flag;
	}
	
	
protected:
	ImageToItk();
	virtual ~ImageToItk();
	void PrintSelf(std::ostream& os, itk::Indent indent) const;
	
private:
	bool m_CopyMemFlag;
	
	
	ImageToItk(const Self&); //purposely not implemented
	void operator=(const Self&); //purposely not implemented
	
};


} // end namespace itk



#ifndef ITK_MANUAL_INSTANTIATION
#include "mitkImageToItk.txx"
#endif

#endif // IMAGETOITK_H_HEADER_INCLUDED_C1C2FCD2

