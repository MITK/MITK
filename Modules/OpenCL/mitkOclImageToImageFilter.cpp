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
  m_output = mitk::OclImage::New();
}


mitk::OclImageToImageFilter::~OclImageToImageFilter()
{

}

mitk::OclImage::Pointer mitk::OclImageToImageFilter::GetGPUOutput()
{
  // initialize some variables
  m_output->SetPixelType(m_input->GetPixelType());

  // create new image, for passing the essential information to the output
  m_output->InitializeMITKImage();

  const unsigned int dimension = m_input->GetDimension();
  unsigned int* dimensions = m_input->GetDimensions();

  m_output->SetDimensions( dimensions );
  m_output->SetDimension( (unsigned short)dimension );

  m_output->GetMITKImage()->Initialize( this->GetOutputType(), dimension, dimensions);
  const mitk::SlicedGeometry3D::Pointer p_slg = m_input->GetMITKImage()->GetSlicedGeometry(0);
  m_output->GetMITKImage()->SetSpacing( p_slg->GetSpacing() );
  m_output->GetMITKImage()->SetGeometry( m_input->GetMITKImage()->GetGeometry() );

  return this->m_output;

}

mitk::Image::Pointer mitk::OclImageToImageFilter::GetOutput()
{
  if (m_output->IsModified(GPU_DATA))
  {
    void* pData = m_output->TransferDataToCPU(m_commandQue);

    const unsigned int dimension = m_input->GetDimension();
    unsigned int* dimensions = m_input->GetDimensions();

    const mitk::SlicedGeometry3D::Pointer p_slg = m_input->GetMITKImage()->GetSlicedGeometry();

    MITK_INFO << " Creating new MITK Image. ";

    m_output->GetMITKImage()->Initialize( this->GetOutputType(), dimension, dimensions);
    m_output->GetMITKImage()->SetSpacing( p_slg->GetSpacing());
    m_output->GetMITKImage()->SetGeometry( m_input->GetMITKImage()->GetGeometry() );
    m_output->GetMITKImage()->SetImportVolume( pData, 0, 0, mitk::Image::ReferenceMemory);
  }

  MITK_INFO<< "Image Initialized.";

  return m_output->GetMITKImage();
}

void mitk::OclImageToImageFilter::Update()
{

}

mitk::PixelType mitk::OclImageToImageFilter::GetOutputType()
{
  // get the current image format from the input image
  const cl_image_format* currentImFormat = this->m_input->GetPixelType();

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
  return (this->m_currentType + 1);
}

bool mitk::OclImageToImageFilter::InitExec(cl_kernel ckKernel)
{
  cl_int clErr = 0;

  if( m_input.IsNull() )
    mitkThrow() << "Input image is null.";

  // get image size once
  const unsigned int uiImageWidth  = m_input->GetDimension(0);
  const unsigned int uiImageHeight = m_input->GetDimension(1);
  const unsigned int uiImageDepth  = m_input->GetDimension(2);

  // compute work sizes
  this->SetWorkingSize( 8, uiImageWidth, 8, uiImageHeight , 8, uiImageDepth );

  cl_mem clBuffIn = m_input->GetGPUImage(this->m_commandQue);
  cl_mem clBuffOut = m_output->GetGPUBuffer();

  if (!clBuffIn)
  {
    if ( m_input->TransferDataToGPU(m_commandQue) != CL_SUCCESS )
    {
      mitkThrow()<< "Could not create / initialize gpu image.";
    }

    clBuffIn = m_input->GetGPUImage(m_commandQue);
  }

  // output image not initialized
  if (!clBuffOut)
  {
    //TODO bpp, or SetImageWidth/Height/...
    MITK_INFO << "Create GPU Image call " << uiImageWidth<< "x"<<uiImageHeight<< "x"<<uiImageDepth;
    clBuffOut = m_output->CreateGPUImage(uiImageWidth, uiImageHeight, uiImageDepth, this->m_currentType + 1);
  }

  clErr = 0;
  clErr  = clSetKernelArg(ckKernel, 0, sizeof(cl_mem), &clBuffIn);
  clErr |= clSetKernelArg(ckKernel, 1, sizeof(cl_mem), &clBuffOut);
  CHECK_OCL_ERR( clErr );

  if( clErr != CL_SUCCESS )
    mitkThrow() << "OpenCL Part initialization failed with " << GetOclErrorString(clErr);

  return( clErr == CL_SUCCESS );
}
