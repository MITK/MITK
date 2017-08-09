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

#include "mitkPhotoacousticOCLBeamformer.h"
#include "usServiceReference.h"

mitk::PhotoacousticOCLBeamformer::PhotoacousticOCLBeamformer()
: m_PixelCalculation( NULL )
{
  this->AddSourceFile("DASQuadratic.cl");
  this->AddSourceFile("DMASQuadratic.cl");
  this->AddSourceFile("DASspherical.cl");
  this->AddSourceFile("DMASspherical.cl");

  this->m_FilterID = "PixelCalculation";
}

mitk::PhotoacousticOCLBeamformer::~PhotoacousticOCLBeamformer()
{
  if ( this->m_PixelCalculation )
  {
    clReleaseKernel( m_PixelCalculation );
  }
}

void mitk::PhotoacousticOCLBeamformer::Update()
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

void mitk::PhotoacousticOCLBeamformer::Execute()
{
  cl_int clErr = 0;

  try
  {
    this->InitExec(this->m_PixelCalculation, m_OutputDim);
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing filter: " << e.what();
    return;
  }

  if (m_Apodisation == nullptr)
  {
    MITK_INFO << "No apodisation function set; Beamforming will be done without any apodisation.";
    m_Apodisation = new float[1];
    m_Apodisation[0] = 1;
    m_ApodArraySize = 1;
  }

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  cl_context gpuContext = resources->GetContext();

  cl_mem cl_input = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * m_ApodArraySize, m_Apodisation, &clErr);
  CHECK_OCL_ERR(clErr);

  // set kernel arguments
  clErr = clSetKernelArg( this->m_PixelCalculation, 2, sizeof(cl_mem), &cl_input);
  clErr |= clSetKernelArg( this->m_PixelCalculation, 3, sizeof(cl_ushort), &(this->m_ApodArraySize) );
  clErr |= clSetKernelArg( this->m_PixelCalculation, 4, sizeof(cl_float), &(this->m_SpeedOfSound) );
  clErr |= clSetKernelArg( this->m_PixelCalculation, 5, sizeof(cl_float), &(this->m_TimeSpacing) );
  clErr |= clSetKernelArg( this->m_PixelCalculation, 6, sizeof(cl_float), &(this->m_Pitch) );
  clErr |= clSetKernelArg( this->m_PixelCalculation, 7, sizeof(cl_float), &(this->m_Angle) );
  clErr |= clSetKernelArg( this->m_PixelCalculation, 8, sizeof(cl_ushort), &(this->m_PAImage));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 9, sizeof(cl_ushort), &(this->m_TransducerElements));

  CHECK_OCL_ERR( clErr );

  // execute the filter on a 3D NDRange
  this->ExecuteKernel( m_PixelCalculation, 3);
  
  // signalize the GPU-side data changed
  m_Output->Modified( GPU_DATA );
}

us::Module *mitk::PhotoacousticOCLBeamformer::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::PhotoacousticOCLBeamformer::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if ( OclFilter::Initialize() )
  {
    switch (m_Algorithm)
    {
      case BeamformingAlgorithm::DASQuad:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDASQuad", &clErr);
        break;
      }
      case BeamformingAlgorithm::DMASQuad:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMASQuad", &clErr);
        break;
      }
      case BeamformingAlgorithm::DASSphe:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDASSphe", &clErr);
        break;
      }
      case BeamformingAlgorithm::DMASSphe:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMASSphe", &clErr);
        break;
      }
    }
    buildErr |= CHECK_OCL_ERR( clErr );
  }
  return (OclFilter::IsInitialized() && buildErr );
}

void mitk::PhotoacousticOCLBeamformer::SetInput(mitk::Image::Pointer image)
{
  if(image->GetDimension() != 3)
  {
    mitkThrowException(mitk::Exception) << "Input for " << this->GetNameOfClass() <<
                                           " is not 3D. The filter only supports 3D. Please change your input.";
  }
  OclImageToImageFilter::SetInput(image);
}

mitk::Image::Pointer mitk::PhotoacousticOCLBeamformer::GetOutput()
{
  if (m_Output->IsModified(GPU_DATA))
  {
    void* pData = m_Output->TransferDataToCPU(m_CommandQue);

    const unsigned int dimension = 3;
    unsigned int* dimensions = m_OutputDim;

    const mitk::SlicedGeometry3D::Pointer p_slg = m_Input->GetMITKImage()->GetSlicedGeometry();

    MITK_DEBUG << "Creating new MITK Image.";

    m_Output->GetMITKImage()->Initialize(this->GetOutputType(), dimension, dimensions);
    m_Output->GetMITKImage()->SetSpacing(p_slg->GetSpacing());
    m_Output->GetMITKImage()->SetGeometry(m_Input->GetMITKImage()->GetGeometry());
    m_Output->GetMITKImage()->SetImportVolume(pData, 0, 0, mitk::Image::ReferenceMemory);
  }

  MITK_DEBUG << "Image Initialized.";

  return m_Output->GetMITKImage();
}
