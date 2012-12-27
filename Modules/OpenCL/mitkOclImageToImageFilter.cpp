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

#include "mitkOclImageToImageFilter.h"
#include "mitkOclImage.h"

#include "mitkException.h"

mitk::OclImageToImageFilter::OclImageToImageFilter()
{
  m_Output = mitk::OclImage::New();
}


mitk::OclImageToImageFilter::~OclImageToImageFilter()
{
}

mitk::OclImage::Pointer mitk::OclImageToImageFilter::GetGPUOutput()
{
  // initialize some variables
  m_Output->SetPixelType(m_Input->GetPixelType());

  // create new image, for passing the essential information to the output
  m_Output->InitializeMITKImage();

  const unsigned int dimension = m_Input->GetDimension();
  unsigned int* dimensions = m_Input->GetDimensions();

  m_Output->SetDimensions( dimensions );
  m_Output->SetDimension( (unsigned short)dimension );

  m_Output->GetMITKImage()->Initialize( this->GetOutputType(), dimension, dimensions);
  const mitk::SlicedGeometry3D::Pointer p_slg = m_Input->GetMITKImage()->GetSlicedGeometry(0);
  m_Output->GetMITKImage()->SetSpacing( p_slg->GetSpacing() );
  m_Output->GetMITKImage()->SetGeometry( m_Input->GetMITKImage()->GetGeometry() );

  return this->m_Output;

}

mitk::Image::Pointer mitk::OclImageToImageFilter::GetOutput()
{
  if (m_Output->IsModified(GPU_DATA))
  {
    void* pData = m_Output->TransferDataToCPU(m_CommandQue);

    const unsigned int dimension = m_Input->GetDimension();
    unsigned int* dimensions = m_Input->GetDimensions();

    const mitk::SlicedGeometry3D::Pointer p_slg = m_Input->GetMITKImage()->GetSlicedGeometry();

    MITK_DEBUG << "Creating new MITK Image.";

    m_Output->GetMITKImage()->Initialize( this->GetOutputType(), dimension, dimensions);
    m_Output->GetMITKImage()->SetSpacing( p_slg->GetSpacing());
    m_Output->GetMITKImage()->SetGeometry( m_Input->GetMITKImage()->GetGeometry() );
    m_Output->GetMITKImage()->SetImportVolume( pData, 0, 0, mitk::Image::ReferenceMemory);
  }

  MITK_DEBUG << "Image Initialized.";

  return m_Output->GetMITKImage();
}

mitk::PixelType mitk::OclImageToImageFilter::GetOutputType()
{
  // get the current image format from the input image
  const cl_image_format* currentImFormat = this->m_Input->GetPixelType();

  // return the value according to the current channel type
  switch( currentImFormat->image_channel_data_type )
  {
  case CL_UNORM_INT8:
    return mitk::MakeScalarPixelType<unsigned char>();
  case CL_UNSIGNED_INT8:
    return mitk::MakeScalarPixelType<unsigned char>();
  case CL_UNORM_INT16:
    return mitk::MakeScalarPixelType<short>();
  default:
    return mitk::MakeScalarPixelType<short>();
  }
}

int mitk::OclImageToImageFilter::GetBytesPerElem()
{
  return (this->m_CurrentType + 1);
}

bool mitk::OclImageToImageFilter::InitExec(cl_kernel ckKernel)
{
  cl_int clErr = 0;

  if( m_Input.IsNull() )
    mitkThrow() << "Input image is null.";

  // get image size once
  const unsigned int uiImageWidth  = m_Input->GetDimension(0);
  const unsigned int uiImageHeight = m_Input->GetDimension(1);
  const unsigned int uiImageDepth  = m_Input->GetDimension(2);

  // compute work sizes
  this->SetWorkingSize( 8, uiImageWidth, 8, uiImageHeight , 8, uiImageDepth );

  cl_mem clBuffIn = m_Input->GetGPUImage(this->m_CommandQue);
  cl_mem clBuffOut = m_Output->GetGPUBuffer();

  if (!clBuffIn)
  {
    if ( m_Input->TransferDataToGPU(m_CommandQue) != CL_SUCCESS )
    {
      mitkThrow()<< "Could not create / initialize gpu image.";
    }

    clBuffIn = m_Input->GetGPUImage(m_CommandQue);
  }

  // output image not initialized
  if (!clBuffOut)
  {
    //TODO bpp, or SetImageWidth/Height/...
    MITK_DEBUG << "Create GPU Image call " << uiImageWidth<< "x"<<uiImageHeight<< "x"<<uiImageDepth;
    clBuffOut = m_Output->CreateGPUImage(uiImageWidth, uiImageHeight, uiImageDepth, this->m_CurrentType + 1);
  }

  clErr = 0;
  clErr  = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), &clBuffIn);
  clErr |= clSetKernelArg(ckKernel, 1, sizeof(cl_mem), &clBuffOut);
  CHECK_OCL_ERR( clErr );

  if( clErr != CL_SUCCESS )
    mitkThrow() << "OpenCL Part initialization failed with " << GetOclErrorAsString(clErr);

  return( clErr == CL_SUCCESS );
}
