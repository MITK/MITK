/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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
  m_Data(NULL), m_PicDescriptor(NULL), m_VtkImageData(NULL), m_Offset(offset), m_IsComplete(false),
  m_Parent(&aParent)
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


//##ModelId=3E159C240213
mitk::ImageDataItem::ImageDataItem(const mitk::PixelType& type, unsigned int dimension, unsigned int *dimensions) : 
  m_Data(NULL), m_PicDescriptor(NULL), m_VtkImageData(NULL), m_Offset(0), m_IsComplete(false),
  m_Parent(NULL)
{
  //const std::type_info & typeId=*type.GetTypeId();
  m_PicDescriptor=ipPicNew();
  m_PicDescriptor->bpe=type.GetBpe();
  m_PicDescriptor->type=type.GetType();
  m_PicDescriptor->dim=dimension;
  memcpy(m_PicDescriptor->n, dimensions, sizeof(unsigned int)*(dimension<=8?dimension:8));
  m_PicDescriptor->data=m_Data=new unsigned char[_ipPicSize(m_PicDescriptor)];
  
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
}

//##ModelId=3E70F51001F2
void mitk::ImageDataItem::Modified() const
{
  if(m_VtkImageData)
    m_VtkImageData->Modified();
}
