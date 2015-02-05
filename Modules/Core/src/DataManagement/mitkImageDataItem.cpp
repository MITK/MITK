/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/


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

#include <mitkImageVtkReadAccessor.h>
#include <mitkImageVtkWriteAccessor.h>
#include <mitkImage.h>


mitk::ImageDataItem::ImageDataItem(const ImageDataItem& aParent, const mitk::ImageDescriptor::Pointer desc, int timestep, unsigned int dimension, void *data, bool manageMemory, size_t offset)
  : m_Data(static_cast<unsigned char*>(aParent.m_Data)+offset)
  , m_PixelType(new mitk::PixelType(aParent.GetPixelType()))
  , m_ManageMemory(false)
  , m_VtkImageData(NULL)
  , m_VtkImageReadAccessor(NULL)
  , m_VtkImageWriteAccessor(NULL)
  , m_Offset(offset)
  , m_IsComplete(false)
  , m_Size(0)
  , m_Parent(&aParent)
  , m_Dimension(dimension)
  , m_Timestep(timestep)
{
  // compute size
  //const unsigned int *dims = desc->GetDimensions();
  for( unsigned int i=0; i<dimension; i++)
  {
    m_Dimensions[i] = desc->GetDimensions()[i];
  }
  this->ComputeItemSize(m_Dimensions,dimension);

  if(data != NULL && data != m_Data)
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
  {
    m_VtkImageData->Delete();
  }
  if(m_VtkImageReadAccessor != NULL)
  {
    delete m_VtkImageReadAccessor;
  }
  if(m_VtkImageWriteAccessor != NULL)
  {
    delete m_VtkImageWriteAccessor;
  }
  if(m_Parent.IsNull())
  {
    if(m_ManageMemory)
      delete [] m_Data;
  }
  delete m_PixelType;
}

mitk::ImageDataItem::ImageDataItem(const mitk::ImageDescriptor::Pointer desc, int timestep,
                                   void *data, bool manageMemory)
  : m_Data(static_cast<unsigned char*>(data))
  , m_PixelType(new mitk::PixelType(desc->GetChannelDescriptor(0).GetPixelType()))
  , m_ManageMemory(manageMemory)
  , m_VtkImageData(NULL)
  , m_VtkImageReadAccessor(NULL)
  , m_VtkImageWriteAccessor(NULL)
  , m_Offset(0)
  , m_IsComplete(false)
  , m_Size(0)
  , m_Dimension(desc->GetNumberOfDimensions())
  , m_Timestep(timestep)
{

  // compute size
  const unsigned int *dimensions = desc->GetDimensions();
  for( unsigned int i=0; i<m_Dimension; i++)
  {
    m_Dimensions[i] = dimensions[i];
  }

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

mitk::ImageDataItem::ImageDataItem(const mitk::PixelType& type, int timestep,
                                   unsigned int dimension, unsigned int *dimensions,
                                   void *data, bool manageMemory)
  : m_Data(static_cast<unsigned char*>(data))
  , m_PixelType(new mitk::PixelType(type))
  , m_ManageMemory(manageMemory)
  , m_VtkImageData(NULL)
  , m_VtkImageReadAccessor(NULL)
  , m_VtkImageWriteAccessor(NULL)
  , m_Offset(0)
  , m_IsComplete(false)
  , m_Size(0)
  , m_Parent(NULL)
  , m_Dimension(dimension)
  , m_Timestep(timestep)
{
  for( unsigned int i=0; i<m_Dimension; i++)
  {
    m_Dimensions[i] = dimensions[i];
  }

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
  : itk::LightObject()
  , m_Data(NULL)
  , m_PixelType(other.m_PixelType)
  , m_ManageMemory(other.m_ManageMemory)
  , m_VtkImageData(NULL)
  , m_VtkImageReadAccessor(NULL)
  , m_VtkImageWriteAccessor(NULL)
  , m_Offset(other.m_Offset)
  , m_IsComplete(other.m_IsComplete)
  , m_Size(other.m_Size)
  , m_Parent(other.m_Parent)
  , m_Dimension(other.m_Dimension)
  , m_Timestep(other.m_Timestep)
{
  // copy m_Data ??
}

void mitk::ImageDataItem::ComputeItemSize(const unsigned int *dimensions, unsigned int dimension)
{
  m_Size = m_PixelType->GetSize();
  for( unsigned int i=0; i<dimension; i++)
  {
    m_Size *= *(dimensions+i);
  }
}

void mitk::ImageDataItem::ConstructVtkImageData(ImageConstPointer iP) const
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
    inData->SetOrigin( ((mitk::ScalarType) dims[0]) / 2.0, 0, 0 );
  }
  else
  if ( dim == 2 )
  {
    inData->SetDimensions( dims[0] , dims[1] , 1 );
    size = dims[0] * dims[1];
    inData->SetOrigin( ((mitk::ScalarType) dims[0]) / 2.0f, ((mitk::ScalarType) dims[1]) / 2.0f, 0 );
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

  if (m_Timestep >= 0)
  {
    SlicedGeometry3D* geom3d = iP->GetSlicedGeometry(m_Timestep);
    const mitk::Vector3D vspacing = geom3d->GetSpacing();
    double dspacing[3] = { vspacing[0], vspacing[1], vspacing[2] };
    inData->SetSpacing( dspacing );
  }

  if ( m_PixelType->GetComponentType() == itk::ImageIOBase::CHAR )
  {
    scalars = vtkCharArray::New();
  }
  else if (   m_PixelType->GetComponentType() == itk::ImageIOBase::UCHAR)
  {
    scalars = vtkUnsignedCharArray::New();
  }
  else if (  m_PixelType->GetComponentType() == itk::ImageIOBase::SHORT )
  {
    scalars = vtkShortArray::New();
  }
  else if (   m_PixelType->GetComponentType() == itk::ImageIOBase::USHORT )
  {
    scalars = vtkUnsignedShortArray::New();
  }
  else if (  m_PixelType->GetComponentType() == itk::ImageIOBase::INT )
  {
    scalars = vtkIntArray::New();
  }
  else if (   m_PixelType->GetComponentType() == itk::ImageIOBase::UINT )
  {
    scalars = vtkUnsignedIntArray::New();
  }
  else if ( m_PixelType->GetComponentType() == itk::ImageIOBase::LONG )
  {
    scalars = vtkLongArray::New();
  }
  else if (  m_PixelType->GetComponentType() == itk::ImageIOBase::ULONG )
  {
    scalars = vtkUnsignedLongArray::New();
  }
  else if (  m_PixelType->GetComponentType() == itk::ImageIOBase::FLOAT  )
  {
    scalars = vtkFloatArray::New();
  }
  else if (  m_PixelType->GetComponentType() == itk::ImageIOBase::DOUBLE  )
  {
    scalars = vtkDoubleArray::New();
  }
  else
  {
    inData->Delete();
    return;
  }

  m_VtkImageData = inData;

  // set mitk imageDataItem void array to vtk scalar values
  scalars->SetNumberOfComponents(m_PixelType->GetNumberOfComponents());
  scalars->SetVoidArray(m_Data, size * m_PixelType->GetNumberOfComponents(), 1);

  m_VtkImageData->GetPointData()->SetScalars(scalars);
  scalars->Delete();
}

void mitk::ImageDataItem::Modified() const
{
  if(m_VtkImageData)
    m_VtkImageData->Modified();
}

mitk::ImageVtkReadAccessor* mitk::ImageDataItem::GetVtkImageAccessor(mitk::ImageDataItem::ImageConstPointer iP) const
{
  if(m_VtkImageData==NULL)
  {
    ConstructVtkImageData(iP);
  }
  if (m_VtkImageReadAccessor == NULL)
  {
    m_VtkImageReadAccessor = new ImageVtkReadAccessor(iP, this, m_VtkImageData);
  }
  return m_VtkImageReadAccessor;
}

mitk::ImageVtkWriteAccessor* mitk::ImageDataItem::GetVtkImageAccessor(ImagePointer iP)
{
  if(m_VtkImageData==NULL)
  {
    ConstructVtkImageData(iP.GetPointer());
  }
  if (m_VtkImageWriteAccessor == NULL)
  {
    m_VtkImageWriteAccessor = new ImageVtkWriteAccessor(iP, this, m_VtkImageData);
  }
  return m_VtkImageWriteAccessor;
}

