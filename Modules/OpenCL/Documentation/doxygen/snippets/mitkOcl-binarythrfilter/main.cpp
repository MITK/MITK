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

mitk::OclBinaryThresholdImageFilter::OclBinaryThresholdImageFilter()
: m_ckBinaryThreshold( NULL )
{
//! [Create]
  std::string path = "BinaryThresholdFilter.cl";
  this->SetSourceFile( path.c_str() );
  this->m_FilterID = "BinaryThreshold";
//! [Create]
  this->m_lowerThr = 10;
  this->m_upperThr = 200;

  this->m_insideVal = 100;
  this->m_outsideVal = 0;
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
    // clean-up also the resources
    OpenCLActivator::GetResourceServiceRef()->InvalidateStorage();
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
//! [ExecutePrepare]
  try
  {
    this->InitExec( this->m_ckBinaryThreshold );
  }
  catch( const mitk::Exception& e)
  {
    MITK_ERROR << "Catched exception while initializing filter: " << e.what();
    return;
  }
//! [ExecutePrepare]

  // set kernel arguments
//! [SetKernelArguments]
  clErr =  clSetKernelArg( this->m_ckBinaryThreshold, 2, sizeof(cl_int), &(this->m_lowerThr) );
  clErr |= clSetKernelArg( this->m_ckBinaryThreshold, 3, sizeof(cl_int), &(this->m_upperThr) );
  clErr |= clSetKernelArg( this->m_ckBinaryThreshold, 4, sizeof(cl_int), &(this->m_outsideVal) );
  clErr |= clSetKernelArg( this->m_ckBinaryThreshold, 5, sizeof(cl_int), &(this->m_insideVal) );
  CHECK_OCL_ERR( clErr );
//! [SetKernelArguments]

  // execute the filter on a 3D NDRange
  //! [Execute]
  this->ExecuteKernel( m_ckBinaryThreshold, 3);

  // signalize the GPU-side data changed
  m_output->Modified( GPU_DATA );
  //! [Execute]

}

bool mitk::OclBinaryThresholdImageFilter::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;
//! [Initialize]
  if ( OclFilter::Initialize() )
  {
    this->m_ckBinaryThreshold = clCreateKernel( this->m_clProgram, "ckBinaryThreshold", &clErr);
    buildErr |= CHECK_OCL_ERR( clErr );
  }
//! [Initialize]
  return (Superclass::IsInitialized() && buildErr );
}
