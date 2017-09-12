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

#ifdef PHOTOACOUSTICS_USE_GPU

#include "mitkPhotoacousticOCLBeamformer.h"

#include "usServiceReference.h"

mitk::PhotoacousticOCLBeamformingFilter::PhotoacousticOCLBeamformingFilter()
: m_PixelCalculation( NULL ), m_InputImage(mitk::Image::New())
{
  this->AddSourceFile("DASQuadratic.cl");
  this->AddSourceFile("DMASQuadratic.cl");
  this->AddSourceFile("DMAS.cl");

  unsigned int dim[] = { 128, 2048, 2 };

  mitk::Vector3D spacing;
  spacing[0] = 1;
  spacing[1] = 1;
  spacing[2] = 1;

  m_InputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dim);
  m_InputImage->SetSpacing(spacing);

  m_ChunkSize[0] = 128;
  m_ChunkSize[1] = 128;
  m_ChunkSize[2] = 8;

  m_SumFilter = mitk::OCLMemoryLocSum::New();
  m_UsedLinesCalculation = mitk::OCLUsedLinesCalculation::New();
  m_DelayCalculation = mitk::OCLDelayCalculation::New();

  this->m_FilterID = "OpenCLBeamformingFilter";
}

mitk::PhotoacousticOCLBeamformingFilter::~PhotoacousticOCLBeamformingFilter()
{
  if ( this->m_PixelCalculation )
  {
    clReleaseKernel( m_PixelCalculation );
  }
}

void mitk::PhotoacousticOCLBeamformingFilter::Update()
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

#include <chrono>

void mitk::PhotoacousticOCLBeamformingFilter::Execute()
{
  cl_int clErr = 0;

  try
  {
    size_t outputSize = m_OutputDim[0] * m_OutputDim[1] * m_OutputDim[2];
    this->InitExec(this->m_PixelCalculation, m_OutputDim, outputSize, sizeof(float));
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

  cl_mem ApodizationBuffer = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * m_ApodArraySize, m_Apodisation, &clErr);
  CHECK_OCL_ERR(clErr);

  m_PAImage = (unsigned short)m_Conf.Photoacoustic;

  if (m_Conf.Algorithm == BeamformingFilter::beamformingSettings::BeamformingAlgorithm::DAS)
  {
    // set kernel arguments
    clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &ApodizationBuffer);
    clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_ushort), &(this->m_ApodArraySize));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_float), &(this->m_Conf.SpeedOfSound));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_float), &(this->m_Conf.TimeSpacing));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_float), &(this->m_Conf.Pitch));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_float), &(this->m_Conf.Angle));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 8, sizeof(cl_ushort), &(this->m_PAImage));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 9, sizeof(cl_ushort), &(this->m_Conf.TransducerElements));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 10, sizeof(cl_uint), &(this->m_InputDim[0]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 11, sizeof(cl_uint), &(this->m_InputDim[1]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 12, sizeof(cl_uint), &(this->m_InputDim[2]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 13, sizeof(cl_uint), &(this->m_OutputDim[0]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 14, sizeof(cl_uint), &(this->m_OutputDim[1]));

    CHECK_OCL_ERR(clErr);
  }
  else if (m_Conf.Algorithm == BeamformingFilter::beamformingSettings::BeamformingAlgorithm::DMAS)
  {
    m_UsedLinesCalculation->SetConfig(m_Conf);
    m_UsedLinesCalculation->Update();

    unsigned int sumDimensions[2] = { m_Conf.ReconstructionLines, m_Conf.SamplesPerLine };

    m_SumFilter->SetInput(m_UsedLinesCalculation->GetGPUOutput());
    m_SumFilter->SetInputDimensions(sumDimensions);
    m_SumFilter->Update();

    unsigned int size = m_SumFilter->GetSum();

    cl_mem UsedLinesBuffer = m_UsedLinesCalculation->GetGPUOutput()->GetGPUBuffer();
    cl_mem MemoryLocationsBuffer = m_SumFilter->GetGPUOutput()->GetGPUBuffer();

    m_DelayCalculation->SetConfig(m_Conf);
    m_DelayCalculation->SetInputs(UsedLinesBuffer, MemoryLocationsBuffer, size);
    m_DelayCalculation->Update();

    cl_mem DelaysBuffer = m_DelayCalculation->GetGPUOutput()->GetGPUBuffer();

    clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &UsedLinesBuffer);
    clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_mem), &MemoryLocationsBuffer);
    clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_mem), &DelaysBuffer);
    clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_mem), &ApodizationBuffer);
    clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_ushort), &(this->m_ApodArraySize));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_uint), &(this->m_InputDim[0]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 8, sizeof(cl_uint), &(this->m_InputDim[1]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 9, sizeof(cl_uint), &(this->m_InputDim[2]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 10, sizeof(cl_uint), &(this->m_OutputDim[0]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 11, sizeof(cl_uint), &(this->m_OutputDim[1]));

    CHECK_OCL_ERR(clErr);
  }

  // execute the filter on a 3D NDRange
  if (m_OutputDim[2] == 1)
    this->ExecuteKernelChunks(m_PixelCalculation, 2, m_ChunkSize);
  else
    this->ExecuteKernelChunks(m_PixelCalculation, 3, m_ChunkSize);
  
  // signalize the GPU-side data changed
  m_Output->Modified( GPU_DATA );
}

us::Module *mitk::PhotoacousticOCLBeamformingFilter::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::PhotoacousticOCLBeamformingFilter::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if ( OclFilter::Initialize() )
  {
    switch (m_Conf.Algorithm)
    {
      case BeamformingFilter::beamformingSettings::BeamformingAlgorithm::DAS:
      {
        if(m_Conf.DelayCalculationMethod == BeamformingFilter::beamformingSettings::DelayCalc::QuadApprox)
          this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDASQuad", &clErr);
        else if (m_Conf.DelayCalculationMethod == BeamformingFilter::beamformingSettings::DelayCalc::Spherical)
          this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDASSphe", &clErr);
        break;
      }
      case BeamformingFilter::beamformingSettings::BeamformingAlgorithm::DMAS:
      {
        if (m_Conf.DelayCalculationMethod == BeamformingFilter::beamformingSettings::DelayCalc::QuadApprox)
          this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMAS", &clErr);
        else if (m_Conf.DelayCalculationMethod == BeamformingFilter::beamformingSettings::DelayCalc::Spherical)
          this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMAS", &clErr);
        break;
      }
      default:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDASQuad", &clErr);
        break;
      }
    }
    buildErr |= CHECK_OCL_ERR( clErr );
  }
  return (OclFilter::IsInitialized() && buildErr );
}

void mitk::PhotoacousticOCLBeamformingFilter::SetInput(mitk::Image::Pointer image)
{
  OclDataSetToDataSetFilter::SetInput(image);

  m_InputImage = image;
  m_InputDim[0] = m_InputImage->GetDimension(0);
  m_InputDim[1] = m_InputImage->GetDimension(1);
  m_InputDim[2] = m_InputImage->GetDimension(2);
}

void mitk::PhotoacousticOCLBeamformingFilter::SetInput(void* data, unsigned int* dimensions, unsigned int BpE)
{
  OclDataSetToDataSetFilter::SetInput(data, dimensions[0] * dimensions[1] * dimensions[2], BpE);

  m_InputDim[0] = dimensions[0];
  m_InputDim[1] = dimensions[1];
  m_InputDim[2] = dimensions[2];
}

mitk::Image::Pointer mitk::PhotoacousticOCLBeamformingFilter::GetOutputAsImage()
{
  mitk::Image::Pointer outputImage = mitk::Image::New();

  if (m_Output->IsModified(GPU_DATA))
  {
    void* pData = m_Output->TransferDataToCPU(m_CommandQue);

    const unsigned int dimension = 3;
    unsigned int dimensions[3] = { (unsigned int)m_OutputDim[0], (unsigned int)m_OutputDim[1], (unsigned int)m_OutputDim[2] };

    const mitk::SlicedGeometry3D::Pointer p_slg = m_InputImage->GetSlicedGeometry();

    MITK_DEBUG << "Creating new MITK Image.";

    outputImage->Initialize(this->GetOutputType(), dimension, dimensions);
    outputImage->SetSpacing(p_slg->GetSpacing());
    outputImage->SetImportVolume(pData, 0, 0, mitk::Image::ReferenceMemory);
  }

  MITK_DEBUG << "Image Initialized.";

  return outputImage;
}
#endif
