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


mitk::ImageDataItem::ImageDataItem(const ImageDataItem& aParent, const mitk::ImageDescriptor::Pointer desc, unsigned int dimension, void *data, bool manageMemory, size_t offset) :
  m_Data(NULL), m_ManageMemory(false), m_VtkImageData(NULL), m_Offset(offset), m_IsComplete(false), m_Size(0),
  m_Parent(&aParent)
{
  m_PixelType = new mitk::PixelType(aParent.GetPixelType());
  m_Data = static_cast<unsigned char*>(aParent.GetData())+offset;

  // compute size
  const unsigned int *dims = desc->GetDimensions();
  m_Dimension = dimension;
  for( unsigned int i=0; i<dimension; i++)
    m_Dimensions[i] = desc->GetDimensions()[i];
  this->ComputeItemSize(m_Dimensions,dimension);

  if(data != NULL)
  {
    memcpy(m_Data, data, m_Size);
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
  if(m_Parent.IsNull())
  {
    if(m_ManageMemory)
      delete [] m_Data;
  }
  delete m_PixelType;
}

mitk::ImageDataItem::ImageDataItem(const mitk::ImageDescriptor::Pointer desc, void *data, bool manageMemory)
  : m_Data((unsigned char*)data), m_ManageMemory(manageMemory), m_VtkImageData(NULL), m_Offset(0), m_IsComplete(false), m_Size(0)
{
  m_PixelType = new mitk::PixelType(desc->GetChannelDescriptor(0).GetPixelType());

  // compute size
  const unsigned int *dimensions = desc->GetDimensions();

  m_Dimension = desc->GetNumberOfDimensions();
  for( unsigned int i=0; i<m_Dimension; i++)
    m_Dimensions[i] = dimensions[i];

  this->ComputeItemSize(m_Dimensions, m_Dimension );

  if(m_Data == NULL)
  {
    m_Data = mitk::MemoryUtilities::AllocateElements<unsigned char>( m_Size );
    m_ManageMemory = true;
  }

  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();

}

mitk::ImageDataItem::ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions, void *data, bool manageMemory) :
  m_Data((unsigned char*)data), m_ManageMemory(manageMemory), m_VtkImageData(NULL), m_Offset(0), m_IsComplete(false), m_Size(0),
  m_Parent(NULL)
{
  m_PixelType = new mitk::PixelType(type);

  m_Dimension = dimension;
  for( unsigned int i=0; i<m_Dimension; i++)
    m_Dimensions[i] = dimensions[i];

  this->ComputeItemSize(dimensions, dimension);

  if(m_Data == NULL)
  {
    m_Data = mitk::MemoryUtilities::AllocateElements<unsigned char>( m_Size );
    m_ManageMemory = true;
  }

  m_ReferenceCountLock.Lock();
  m_ReferenceCount = 0;
  m_ReferenceCountLock.Unlock();
}

mitk::ImageDataItem::ImageDataItem(const ImageDataItem &other)
  : m_PixelType(other.m_PixelType), m_ManageMemory(other.m_ManageMemory), m_Offset(other.m_Offset),
    m_IsComplete(other.m_IsComplete), m_Size(other.m_Size)
{

}

void mitk::ImageDataItem::ComputeItemSize(const unsigned int *dimensions, unsigned int dimension)
{
  m_Size = m_PixelType->GetSize();
  for( unsigned int i=0; i<dimension; i++)
  {
    m_Size *= *(dimensions+i);
  }
}

void mitk::ImageDataItem::ConstructVtkImageData() const
{
  vtkImageData *inData = vtkImageData::New();
  vtkDataArray *scalars = NULL;

  const unsigned int *dims = m_Dimensions;
  const unsigned int dim = m_Dimension;

  unsigned long size = 0;
  if ( dim == 1 )
  {
    inData->SetDimensions( dims[0] -1, 1, 1);
    size = dims[0];
    inData->SetOrigin( ((float) dims[0]) / 2.0f, 0, 0 );
  }
  else
  if ( dim == 2 )
  {
    inData->SetDimensions( dims[0] , dims[1] , 1 );
    size = dims[0] * dims[1];
    inData->SetOrigin( ((float) dims[0]) / 2.0f, ((float) dims[1]) / 2.0f, 0 );
  }
  else
  if ( dim >= 3 )
  {
    inData->SetDimensions( dims[0], dims[1], dims[2] );
    size = dims[0] * dims[1] * dims[2];
    // Test
    //inData->SetOrigin( (float) dims[0] / 2.0f, (float) dims[1] / 2.0f, (float) dims[2] / 2.0f );
    inData->SetOrigin( 0, 0, 0 );
  }
  else
  {
    inData->Delete () ;
    return;
  }

  inData->SetNumberOfScalarComponents(m_PixelType->GetNumberOfComponents());

/*  if ( ( m_PixelType.GetType() == mitkIpPicInt || m_PixelType.GetType() == mitkIpPicUInt ) && m_PixelType.GetBitsPerComponent() == 1 )
  {
    inData->SetScalarType( VTK_BIT );
    scalars = vtkBitArray::New();
  }
  else*/ if ( m_PixelType->GetTypeId() == typeid(char) )
  {
    inData->SetScalarType( VTK_CHAR );
    scalars = vtkCharArray::New();
  }
  else if (   m_PixelType->GetTypeId() == typeid(unsigned char))
  {
    inData->SetScalarType( VTK_UNSIGNED_CHAR );
    scalars = vtkUnsignedCharArray::New();
  }
  else if (  m_PixelType->GetTypeId() == typeid(short) )
  {
    inData->SetScalarType( VTK_SHORT );
    scalars = vtkShortArray::New();
  }
  else if (   m_PixelType->GetTypeId() == typeid(unsigned short) )
  {
    inData->SetScalarType( VTK_UNSIGNED_SHORT );
    scalars = vtkUnsignedShortArray::New();
  }
  else if (  m_PixelType->GetTypeId() == typeid(int) )
  {
    inData->SetScalarType( VTK_INT );
    scalars = vtkIntArray::New();
  }
  else if (   m_PixelType->GetTypeId() == typeid(unsigned int) )
  {
    inData->SetScalarType( VTK_UNSIGNED_INT );
    scalars = vtkUnsignedIntArray::New();
  }
  else if ( m_PixelType->GetTypeId() == typeid(long int) )
  {
    inData->SetScalarType( VTK_LONG );
    scalars = vtkLongArray::New();
  }
  else if (  m_PixelType->GetTypeId() == typeid(unsigned long int) )
  {
    inData->SetScalarType( VTK_UNSIGNED_LONG );
    scalars = vtkUnsignedLongArray::New();
  }
  else if (  m_PixelType->GetTypeId() == typeid(float)  )
  {
    inData->SetScalarType( VTK_FLOAT );
    scalars = vtkFloatArray::New();
  }
  else if (  m_PixelType->GetTypeId() == typeid(double)  )
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

  scalars->SetVoidArray(m_Data, size * m_VtkImageData->GetNumberOfScalarComponents(), 1);

  m_VtkImageData->GetPointData()->SetScalars(scalars);
  scalars->Delete();

}

void mitk::ImageDataItem::Modified() const
{
  if(m_VtkImageData)
    m_VtkImageData->Modified();
}


