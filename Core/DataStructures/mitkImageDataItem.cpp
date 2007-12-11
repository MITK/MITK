/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImageDataItem.h"
#include "mitkMemoryUtilities.h"
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

#include "ipFunc/ipFunc.h"

mitk::ImageDataItem::ImageDataItem(const ImageDataItem& aParent, unsigned int dimension, void *data, bool manageMemory, size_t offset) : 
  m_Data(NULL), m_ManageMemory(false), m_PicDescriptor(NULL), m_VtkImageData(NULL), m_Offset(offset), m_IsComplete(false),
  m_Parent(&aParent)
{
  m_PixelType = aParent.GetPixelType();
  m_PicDescriptor=ipPicNew();
  m_PicDescriptor->bpe=m_PixelType.GetBpe();
  m_PicDescriptor->type=m_PixelType.GetType();
  m_PicDescriptor->dim=dimension;
  memcpy(m_PicDescriptor->n, aParent.GetPicDescriptor()->n, sizeof(ipUInt4_t)*_ipPicNDIM);
  m_PicDescriptor->data=m_Data=static_cast<unsigned char*>(aParent.GetData())+offset;
  ipFuncCopyTags(m_PicDescriptor, aParent.GetPicDescriptor());

  if(data != NULL)
  {
    memcpy(m_Data, data, _ipPicSize(m_PicDescriptor));
    if(manageMemory)
    {
      delete [] (unsigned char*) data;
    }
  }

  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
}

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
    if(m_ManageMemory)
      delete [] m_Data;
  }
}

mitk::ImageDataItem::ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, void *data, bool manageMemory) : 
  m_Data((unsigned char*)data), m_ManageMemory(manageMemory), m_PicDescriptor(NULL), m_VtkImageData(NULL), m_Offset(0), m_IsComplete(false),
  m_Parent(NULL)
{
  //const std::type_info & typeId=*type.GetTypeId();
  m_PixelType = type;
  m_PicDescriptor=ipPicNew();
  m_PicDescriptor->bpe=m_PixelType.GetBpe();
  m_PicDescriptor->type=m_PixelType.GetType();
  m_PicDescriptor->dim=dimension;
  memcpy(m_PicDescriptor->n, dimensions, sizeof(ipUInt4_t)*(dimension<=_ipPicNDIM?dimension:_ipPicNDIM));
  unsigned char i;
  for(i=dimension; i < _ipPicNDIM; ++i)
    m_PicDescriptor->n[i] = 1;
  if(m_Data == NULL)
  {
    m_Data = mitk::MemoryUtilities::AllocateElements<unsigned char>( _ipPicSize(m_PicDescriptor) );
    m_ManageMemory = true;
  }
  m_PicDescriptor->data=m_Data;
  
  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
}

void mitk::ImageDataItem::ConstructVtkImageData() const
{
  vtkImageData *inData = vtkImageData::New();
  vtkDataArray *scalars = NULL;
    
  inData->SetNumberOfScalarComponents(m_PixelType.GetNumberOfComponents());

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
  
  if ( ( m_PixelType.GetType() == ipPicInt || m_PixelType.GetType() == ipPicUInt ) && m_PixelType.GetBitsPerComponent() == 1 ) 
  {
    inData->SetScalarType( VTK_BIT );
    scalars = vtkBitArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicInt && m_PixelType.GetBitsPerComponent() == 8 )
  {
    inData->SetScalarType( VTK_CHAR );
    scalars = vtkCharArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicUInt && m_PixelType.GetBitsPerComponent() == 8 )
  {
    inData->SetScalarType( VTK_UNSIGNED_CHAR );
    scalars = vtkUnsignedCharArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicInt && m_PixelType.GetBitsPerComponent() == 16 )
  {
    inData->SetScalarType( VTK_SHORT );
    scalars = vtkShortArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicUInt && m_PixelType.GetBitsPerComponent() == 16 )
  {
    inData->SetScalarType( VTK_UNSIGNED_SHORT );
    scalars = vtkUnsignedShortArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicInt && m_PixelType.GetBitsPerComponent() == 32 )
  {
    inData->SetScalarType( VTK_INT );
    scalars = vtkIntArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicUInt && m_PixelType.GetBitsPerComponent() == 32 )
  {
    inData->SetScalarType( VTK_UNSIGNED_INT );
    scalars = vtkUnsignedIntArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicInt && m_PixelType.GetBitsPerComponent() == 64 )
  {
    inData->SetScalarType( VTK_LONG );
    scalars = vtkLongArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicUInt && m_PixelType.GetBitsPerComponent() == 64 )
  {
    inData->SetScalarType( VTK_UNSIGNED_LONG );
    scalars = vtkUnsignedLongArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicFloat && m_PixelType.GetBitsPerComponent() == 32 )
  {
    inData->SetScalarType( VTK_FLOAT );
    scalars = vtkFloatArray::New();
  }
  else if ( m_PixelType.GetType() == ipPicFloat && m_PixelType.GetBitsPerComponent() == 64 )
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
}

void mitk::ImageDataItem::Modified() const
{
  if(m_VtkImageData)
    m_VtkImageData->Modified();
}
