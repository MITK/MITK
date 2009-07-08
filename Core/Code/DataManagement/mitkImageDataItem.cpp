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

#include "itkDiffusionTensor3D.h"

#include "ipFunc/ipFunc.h"

#ifdef _OPENMP
#include "omp.h"
#endif

#define __IMG_DAT_ITEM__CEIL_ZERO_ONE__(val) (val) =       \
  ( (val) < 0 ) ? ( 0 ) : ( ( (val)>1 ) ? ( 1 ) : ( (val) ) );

mitk::ImageDataItem::ImageDataItem(const ImageDataItem& aParent, unsigned int dimension, void *data, bool manageMemory, size_t offset) :
  m_Data(NULL), m_ManageMemory(false), m_PicDescriptor(NULL), m_VtkImageData(NULL), m_Offset(offset), m_IsComplete(false), m_Size(0),
  m_Parent(&aParent)
{
  m_PixelType = aParent.GetPixelType();
  m_PicDescriptor=ipPicNew();
  m_PicDescriptor->bpe=m_PixelType.GetBpe();
  m_PicDescriptor->type=m_PixelType.GetType();
  m_PicDescriptor->dim=dimension;
  memcpy(m_PicDescriptor->n, aParent.GetPicDescriptor()->n, sizeof(ipUInt4_t)*_mitkIpPicNDIM);
  m_PicDescriptor->data=m_Data=static_cast<unsigned char*>(aParent.GetData())+offset;
  ipFuncCopyTags(m_PicDescriptor, aParent.GetPicDescriptor());

  m_Size = _ipPicSize(m_PicDescriptor);
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
  m_Data((unsigned char*)data), m_ManageMemory(manageMemory), m_PicDescriptor(NULL), m_VtkImageData(NULL), m_Offset(0), m_IsComplete(false), m_Size(0),
  m_Parent(NULL)
{
  //const std::type_info & typeId=*type.GetTypeId();
  m_PixelType = type;
  m_PicDescriptor=ipPicNew();
  m_PicDescriptor->bpe=m_PixelType.GetBpe();
  m_PicDescriptor->type=m_PixelType.GetType();
  m_PicDescriptor->dim=dimension;
  memcpy(m_PicDescriptor->n, dimensions, sizeof(ipUInt4_t)*(dimension<=_mitkIpPicNDIM?dimension:_mitkIpPicNDIM));
  unsigned char i;
  for(i=dimension; i < _mitkIpPicNDIM; ++i)
    m_PicDescriptor->n[i] = 1;
  m_Size = _ipPicSize(m_PicDescriptor);
  if(m_Data == NULL)
  {
    m_Data = mitk::MemoryUtilities::AllocateElements<unsigned char>( m_Size );
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

  // special treatment for tensors
  if(  !(m_PixelType == typeid(itk::DiffusionTensor3D<float>))
    && !(m_PixelType == typeid(itk::DiffusionTensor3D<double>)) )
  {

    // in this case we DO NOT have a tensor image

    inData->SetNumberOfScalarComponents(m_PixelType.GetNumberOfComponents());

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

    scalars->SetVoidArray(m_PicDescriptor->data, _ipPicElements(m_PicDescriptor)*m_VtkImageData->GetNumberOfScalarComponents(), 1);

    m_VtkImageData->GetPointData()->SetScalars(scalars);
    scalars->Delete();
  }
  else
  {
    // in this case we have a tensor image

    // allocate new scalars with three components for RGB
    inData->SetNumberOfScalarComponents(3);
    inData->SetScalarType( VTK_UNSIGNED_CHAR );
    scalars = vtkUnsignedCharArray::New();
    m_VtkImageData = inData;
    scalars->SetNumberOfComponents(m_VtkImageData->GetNumberOfScalarComponents());

    // calculate RGB information from tensors
    if(m_PixelType == typeid(itk::DiffusionTensor3D<float>))
      scalars->SetVoidArray(ConvertTensorsToRGB<itk::DiffusionTensor3D<float> >(), m_Size/2, 1);
    if(m_PixelType == typeid(itk::DiffusionTensor3D<double>))
      scalars->SetVoidArray(ConvertTensorsToRGB<itk::DiffusionTensor3D<double> >(), m_Size/2, 1);

    m_VtkImageData->GetPointData()->SetScalars(scalars);
    scalars->Delete();
  }
}

void mitk::ImageDataItem::Modified() const
{
  if(m_VtkImageData)
    m_VtkImageData->Modified();
}

/**
 * This method calculates RGB image from tensor information.
 * Templated over pixeltype, output always three uchar components.
 */
template <class TPixeltype>
unsigned char *mitk::ImageDataItem::ConvertTensorsToRGB() const
{
  const unsigned char *p   = m_Data;
  unsigned char *out = (unsigned char *) malloc(m_Size/2);
  const int pixelsize = sizeof(TPixeltype);
  const int numIts = m_Size/pixelsize;

#ifdef _OPENMP
#pragma omp parallel for
#endif
  for(int i=0; i<numIts; i++)
  {
    const TPixeltype* tensor = static_cast<TPixeltype* >((void*)(p+i*pixelsize));
    typename TPixeltype::EigenValuesArrayType eigenvalues;
    typename TPixeltype::EigenVectorsMatrixType eigenvectors;
    tensor->ComputeEigenAnalysis(eigenvalues, eigenvectors);

    int index = 2;
    if( (eigenvalues[0] >= eigenvalues[1])
      && (eigenvalues[0] >= eigenvalues[2]) )
      index = 0;
    else if(eigenvalues[1] >= eigenvalues[2])
      index = 1;

    const float fa = tensor->GetFractionalAnisotropy();
    float r = abs(eigenvectors(index,0)) * fa;
    float g = abs(eigenvectors(index,1)) * fa;
    float b = abs(eigenvectors(index,2)) * fa;

    __IMG_DAT_ITEM__CEIL_ZERO_ONE__(r);
    __IMG_DAT_ITEM__CEIL_ZERO_ONE__(g);
    __IMG_DAT_ITEM__CEIL_ZERO_ONE__(b);

    *(out+i*3+0) = (unsigned char)( 255.0f * r );
    *(out+i*3+1) = (unsigned char)( 255.0f * g );
    *(out+i*3+2) = (unsigned char)( 255.0f * b );
  }
  return out;
}
