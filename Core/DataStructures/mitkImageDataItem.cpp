#if _MSC_VER<=1300
#pragma warning( disable : 4290 )
#endif

#include "mitkImageDataItem.h"
#include <vtkImageData.h>
#include <vtkPointData.h>

#include <vtkBitArray.h>
#include <vtkCharArray.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>
#include <vtkLongArray.h>
#include <vtkShortArray.h>
#include <vtkUnsignedCharArray.h>
#include <vtkUnsignedIntArray.h>
#include <vtkUnsignedLongArray.h>
#include <vtkUnsignedShortArray.h>

//##ModelId=3E0B7882024B
mitk::ImageDataItem::ImageDataItem(const ImageDataItem& aParent, unsigned int dimension, int offset) : 
  m_Parent(&aParent), m_Data(NULL), m_VtkImageData(NULL), m_PicDescriptor(NULL), m_IsComplete(false),
  m_Offset(offset)
{
  mitk::PixelType type(aParent.GetPicDescriptor());
  m_PicDescriptor=ipPicNew();
  m_PicDescriptor->bpe=type.GetBpe();
  m_PicDescriptor->type=type.GetType();
  m_PicDescriptor->dim=dimension;
  memcpy(m_PicDescriptor->n, aParent.GetPicDescriptor()->n, sizeof(unsigned int)*(dimension<=8?dimension:8));
  m_PicDescriptor->data=m_Data=static_cast<unsigned char*>(aParent.GetData())+offset;
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
  if(m_Parent.IsNull())
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
m_Parent(NULL), m_Data(NULL), m_VtkImageData(NULL), m_PicDescriptor(NULL), m_IsComplete(false), m_Offset(0)
{
  //const std::type_info & typeId=*type.GetTypeId();
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
  vtkImageData *inData = vtkImageData::New();
  vtkDataArray *scalars = NULL;
    
  inData->SetNumberOfScalarComponents(1);

  unsigned long size = 0;
  if ( m_PicDescriptor->dim == 1 ) 
  {
    inData->SetDimensions( m_PicDescriptor->n[0] -1, 1, 1);		
    size = m_PicDescriptor->n[0];		
    inData->SetOrigin( ((float) m_PicDescriptor->n[0]) / 2.0f, 0, 0 );
  } 
  else 
  if ( m_PicDescriptor->dim == 2 ) 
  {
    inData->SetDimensions( m_PicDescriptor->n[0] , m_PicDescriptor->n[1] , 1 );
    size = m_PicDescriptor->n[0] * m_PicDescriptor->n[1];
    inData->SetOrigin( ((float) m_PicDescriptor->n[0]) / 2.0f, ((float) m_PicDescriptor->n[1]) / 2.0f, 0 );
  }
  else
  if ( m_PicDescriptor->dim >= 3 )
  {
    inData->SetDimensions( m_PicDescriptor->n[0], m_PicDescriptor->n[1], m_PicDescriptor->n[2] );
    size = m_PicDescriptor->n[0] * m_PicDescriptor->n[1] * m_PicDescriptor->n[2];
    // Test
    //inData->SetOrigin( (float) m_PicDescriptor->n[0] / 2.0f, (float) m_PicDescriptor->n[1] / 2.0f, (float) m_PicDescriptor->n[2] / 2.0f );
    inData->SetOrigin( 0, 0, 0 );
  } 
  else
  {
    inData->Delete () ;
    return;
  }
  
  if ( ( m_PicDescriptor->type == ipPicInt || m_PicDescriptor->type == ipPicUInt ) && m_PicDescriptor->bpe == 1 ) 
  {
    inData->SetScalarType( VTK_BIT );
    scalars = vtkBitArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicInt && m_PicDescriptor->bpe == 8 )
  {
    inData->SetScalarType( VTK_CHAR );
    scalars = vtkCharArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicUInt && m_PicDescriptor->bpe == 8 )
  {
    inData->SetScalarType( VTK_UNSIGNED_CHAR );
    scalars = vtkUnsignedCharArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicInt && m_PicDescriptor->bpe == 16 )
  {
    inData->SetScalarType( VTK_SHORT );
    scalars = vtkShortArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicUInt && m_PicDescriptor->bpe == 16 )
  {
    inData->SetScalarType( VTK_UNSIGNED_SHORT );
    scalars = vtkUnsignedShortArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicInt && m_PicDescriptor->bpe == 32 )
  {
    inData->SetScalarType( VTK_INT );
    scalars = vtkIntArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicUInt && m_PicDescriptor->bpe == 32 )
  {
    inData->SetScalarType( VTK_UNSIGNED_INT );
    scalars = vtkUnsignedIntArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicInt && m_PicDescriptor->bpe == 64 )
  {
    inData->SetScalarType( VTK_LONG );
    scalars = vtkLongArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicUInt && m_PicDescriptor->bpe == 64 )
  {
    inData->SetScalarType( VTK_UNSIGNED_LONG );
    scalars = vtkUnsignedLongArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicFloat && m_PicDescriptor->bpe == 32 )
  {
    inData->SetScalarType( VTK_FLOAT );
    scalars = vtkFloatArray::New();
  }
  else if ( m_PicDescriptor->type == ipPicFloat && m_PicDescriptor->bpe == 64 )
  {
    inData->SetScalarType( VTK_DOUBLE );
    scalars = vtkDoubleArray::New();
  }
  else
  {
    inData->Delete();
    return;
  }

  m_VtkImageData = inData;

  // allocate the new scalars
  scalars->SetNumberOfComponents(m_VtkImageData->GetNumberOfScalarComponents());

  scalars->SetVoidArray(m_PicDescriptor->data, _ipPicElements(m_PicDescriptor), 1);

  m_VtkImageData->GetPointData()->SetScalars(scalars);

  scalars->Delete();

  //    m_VtkImageData=Pic2vtk::convert(m_PicDescriptor);
  //m_VtkImageData->GetPointData()->GetScalars()->SetVoidArray(m_PicDescriptor->data, _ipPicElements(m_PicDescriptor), 1);
}

//##ModelId=3E70F51001F2
void mitk::ImageDataItem::Modified() const
{
  if(m_VtkImageData)
    m_VtkImageData->Modified();
}
