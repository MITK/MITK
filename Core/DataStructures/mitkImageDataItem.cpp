#if _MSC_VER<=1300
#pragma warning( disable : 4290 )
#endif

#include "ImageDataItem.h"

//##ModelId=3E0B7882024B
mitk::ImageDataItem::ImageDataItem(const ImageDataItem& aParent, unsigned int dimension, int offset) : 
	m_Parent(&aParent), m_Data(NULL), m_itkImageObject(NULL), m_VtkImageData(NULL), m_PicDescriptor(NULL), m_IsComplete(false),
		m_Offset(offset)
{
	mitk::PixelType type(aParent.GetPicDescriptor());
	m_PicDescriptor=ipPicNew();
	m_PicDescriptor->bpe=type.GetBpe();
	m_PicDescriptor->type=type.GetType();
	m_PicDescriptor->dim=dimension;
	memcpy(m_PicDescriptor->n, aParent.GetPicDescriptor()->n, sizeof(unsigned int)*(dimension<=8?dimension:8));
	m_PicDescriptor->data=m_Data=static_cast<char*>(aParent.GetData())+offset;
}

//##ModelId=3E0B78820287
mitk::ImageDataItem::~ImageDataItem()
{
	if(m_VtkImageData!=NULL)
		m_VtkImageData->Delete();
	if(m_PicDescriptor!=NULL)
	{
		m_PicDescriptor->data=NULL;
		ipPicFree(m_PicDescriptor);
	}
	if(m_itkImageObject!=NULL)
	{
	}
	if(m_Parent==NULL)
	{
		if(m_Data!=NULL)
			delete m_Data;
	}
}

//#define ALLOCATE_ITK_IMAGE(type, typeId, dimension, dimensions) \
//	if ( typeId == typeid(type) )                               \
//	{                                                           \
//		m_itkImageObject=itk::Image<type, dimension>::New();    \
//	}                                                           
//
//#define ALLOCATE_ALL_ITK_IMAGES(typeId, dimension, dimensions)                            \
//{                                                                                         \
//	ALLOCATE_ITK_IMAGE(double, typeId, dimension, dimensions) else                        \
//	ALLOCATE_ITK_IMAGE(float, typeId, dimension, dimensions) else                         \
//	ALLOCATE_ITK_IMAGE(long, typeId, dimension, dimensions) else                          \
//	ALLOCATE_ITK_IMAGE(unsigned long, typeId, dimension, dimensions) else                 \
//	ALLOCATE_ITK_IMAGE(int, typeId, dimension, dimensions) else                           \
//	ALLOCATE_ITK_IMAGE(unsigned int, typeId, dimension, dimensions) else                  \
//	ALLOCATE_ITK_IMAGE(short, typeId, dimension, dimensions) else                         \
//	ALLOCATE_ITK_IMAGE(unsigned short, typeId, dimension, dimensions) else                \
//	ALLOCATE_ITK_IMAGE(char, typeId, dimension, dimensions) else                          \
//	ALLOCATE_ITK_IMAGE(unsigned char, typeId, dimension, dimensions) else                 \
//	if ( typeId == typeid(itk::RGBPixel<unsigned char>) )                                 \
//	{                                                                                     \
//		itkExceptionMacro("Pixel type currently not supported.");                         \
//	}                                                                                     \
//	else                                                                                  \
//	if ( typeId == typeid(itk::RGBAPixel<unsigned char>) )                                \
//	{                                                                                     \
//		itkExceptionMacro("Pixel type currently not supported.");                         \
//	}                                                                                     \
//	else                                                                                  \
//	{                                                                                     \
//		itkExceptionMacro("Pixel type currently not supported.");                         \
//	}                                                                                     \
//}                                                                                         \
////	ALLOCATE_ITK_IMAGE(itk::RGBPixel<unsigned char>, typeId, dimension, dimensions) else                          \
////	ALLOCATE_ITK_IMAGE(itk::RGBAPixel<unsigned char>, typeId, dimension, dimensions) else                          \

//##ModelId=3E159C240213
mitk::ImageDataItem::ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions) : 
	m_Parent(NULL), m_Data(NULL), m_itkImageObject(NULL), m_VtkImageData(NULL), m_PicDescriptor(NULL), m_IsComplete(false)
{
	const type_info & typeId=*type.GetTypeId();
	m_PicDescriptor=ipPicNew();
	m_PicDescriptor->bpe=type.GetBpe();
	m_PicDescriptor->type=type.GetType();
	m_PicDescriptor->dim=dimension;
	memcpy(m_PicDescriptor->n, dimensions, sizeof(unsigned int)*(dimension<=8?dimension:8));
	m_PicDescriptor->data=m_Data=new unsigned char[_ipPicSize(m_PicDescriptor)];

	//if(dimension==2)
	//	ALLOCATE_ALL_ITK_IMAGES(typeId, 2, dimensions)
	//else
	//if(dimension==3)
	//	ALLOCATE_ALL_ITK_IMAGES(typeId, 3, dimensions)
	//else
	//if(dimension==4)
	//	ALLOCATE_ALL_ITK_IMAGES(typeId, 4, dimensions)
	////else
	////if(dimension==5)
	////	ALLOCATE_ALL_ITK_IMAGES(typeId, 5, dimensions)
	////else
	////if(dimension==6)
	////	ALLOCATE_ALL_ITK_IMAGES(typeId, 6, dimensions)
	////else
	////if(dimension==7)
	////	ALLOCATE_ALL_ITK_IMAGES(typeId, 7, dimensions)
	////else
	////if(dimension==8)
	////	ALLOCATE_ALL_ITK_IMAGES(typeId, 8, dimensions)
}

//##ModelId=3E33F08A03B8
#include "pic2vtk/pic2vtk.h"
void mitk::ImageDataItem::ConstructVtkImageData() const
{
    m_VtkImageData=Pic2vtk::convert(m_PicDescriptor);
    m_VtkImageData->GetPointData()->GetScalars()->SetVoidArray(m_PicDescriptor->data, _ipPicElements(m_PicDescriptor), 1);
}

//##ModelId=3E70F51001F2
void mitk::ImageDataItem::Modified() const
{
    if(m_VtkImageData)
        m_VtkImageData->Modified();
}
