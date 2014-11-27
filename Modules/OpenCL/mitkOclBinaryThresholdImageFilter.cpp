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

#include "mitkOclBinaryThresholdImageFilter.h"
#include "usServiceReference.h"

mitk::OclBinaryThresholdImageFilter::OclBinaryThresholdImageFilter()
: m_ckBinaryThreshold( NULL )
{
  this->AddSourceFile("BinaryThresholdFilter.cl");
  this->m_FilterID = "BinaryThreshold";

  this->m_LowerThreshold = 10;
  this->m_UpperThreshold = 200;

  this->m_InsideValue = 100;
  this->m_OutsideValue = 0;
}

mitk::OclBinaryThresholdImageFilter::~OclBinaryThresholdImageFilter()
{
  if ( this->m_ckBinaryThreshold )
  {
    clReleaseKernel( m_ckBinaryThreshold );
  }
}

void mitk::OclBinaryThresholdImageFilter::Update()
{
  //Check if context & program available
  if (!this->Initialize())
  {
    us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
    OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

    // clean-up also the resources
    resources->InvalidateStorage();
    mitkThrow() <<"Filter is not initialized. Cannot update.";
  }
  else{
    // Execute
    this->Execute();
  }
}

void mitk::OclBinaryThresholdImageFilter::Execute()
{
  cl_int clErr = 0;

  try
  {
    this->InitExec( this->m_ckBinaryThreshold );
  }
  catch( const mitk::Exception& e)
  {
    MITK_ERROR << "Catched exception while initializing filter: " << e.what();
    return;
  }

  // set kernel arguments
  clErr =  clSetKernelArg( this->m_ckBinaryThreshold, 2, sizeof(cl_int), &(this->m_LowerThreshold) );
  clErr |= clSetKernelArg( this->m_ckBinaryThreshold, 3, sizeof(cl_int), &(this->m_UpperThreshold) );
  clErr |= clSetKernelArg( this->m_ckBinaryThreshold, 4, sizeof(cl_int), &(this->m_OutsideValue) );
  clErr |= clSetKernelArg( this->m_ckBinaryThreshold, 5, sizeof(cl_int), &(this->m_InsideValue) );
  CHECK_OCL_ERR( clErr );

  // execute the filter on a 3D NDRange
  this->ExecuteKernel( m_ckBinaryThreshold, 3);

  // signalize the GPU-side data changed
  m_Output->Modified( GPU_DATA );
}

us::Module *mitk::OclBinaryThresholdImageFilter::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::OclBinaryThresholdImageFilter::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if ( OclFilter::Initialize() )
  {
    this->m_ckBinaryThreshold = clCreateKernel( this->m_ClProgram, "ckBinaryThreshold", &clErr);
    buildErr |= CHECK_OCL_ERR( clErr );
  }

  return (OclFilter::IsInitialized() && buildErr );
}

void mitk::OclBinaryThresholdImageFilter::SetInput(mitk::Image::Pointer image)
{
  if(image->GetDimension() != 3)
  {
    mitkThrowException(mitk::Exception) << "Input for " << this->GetNameOfClass() <<
                                           " is not 3D. The filter only supports 3D. Please change your input.";
  }
  OclImageToImageFilter::SetInput(image);
}
