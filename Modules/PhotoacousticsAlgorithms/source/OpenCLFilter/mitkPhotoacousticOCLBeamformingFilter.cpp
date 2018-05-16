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

#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

#include "./OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.h"
#include "usServiceReference.h"

mitk::PhotoacousticOCLBeamformingFilter::PhotoacousticOCLBeamformingFilter()
: m_PixelCalculation( NULL ), m_InputImage(mitk::Image::New()), m_ApodizationBuffer(nullptr), m_MemoryLocationsBuffer(nullptr), m_DelaysBuffer(nullptr), m_UsedLinesBuffer(nullptr)
{
  this->AddSourceFile("DAS.cl");
  this->AddSourceFile("DMAS.cl");
  this->AddSourceFile("sDMAS.cl");
  this->m_FilterID = "OpenCLBeamformingFilter";

  this->Initialize();

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

  m_UsedLinesCalculation = mitk::OCLUsedLinesCalculation::New();
  m_DelayCalculation = mitk::OCLDelayCalculation::New();
}

mitk::PhotoacousticOCLBeamformingFilter::~PhotoacousticOCLBeamformingFilter()
{
  if ( this->m_PixelCalculation )
  {
    clReleaseKernel( m_PixelCalculation );
  }

  if (m_ApodizationBuffer) clReleaseMemObject(m_ApodizationBuffer);
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

void mitk::PhotoacousticOCLBeamformingFilter::UpdateDataBuffers()
{
  /*us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  cl_ulong globalMemSize = oclGetGlobalMemSize(resources->GetCurrentDevice());*/
  //Initialize the Output

  try
  {
    MITK_DEBUG << "Updating Workgroup size for new dimensions";
    size_t outputSize = (size_t)m_Conf.ReconstructionLines * (size_t)m_Conf.SamplesPerLine * (size_t)m_Conf.inputDim[2];
    m_OutputDim[0] = m_Conf.ReconstructionLines;
    m_OutputDim[1] = m_Conf.SamplesPerLine;
    m_OutputDim[2] = m_Conf.inputDim[2];
    this->InitExec(this->m_PixelCalculation, m_OutputDim, outputSize, sizeof(float));
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing filter: " << e.what();
    return;
  }

  if (BeamformingSettings::SettingsChangedOpenCL(m_Conf, m_ConfOld))
  {
    cl_int clErr = 0;
    MITK_DEBUG << "Updating GPU Buffers for new configuration";

    // create the apodisation buffer

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

    if (m_ApodizationBuffer) clReleaseMemObject(m_ApodizationBuffer);

    this->m_ApodizationBuffer = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * m_ApodArraySize, m_Apodisation, &clErr);
    CHECK_OCL_ERR(clErr);

    // calculate used lines

    m_UsedLinesCalculation->SetConfig(m_Conf);
    m_UsedLinesCalculation->Update();
    m_UsedLinesBuffer = m_UsedLinesCalculation->GetGPUOutput()->GetGPUBuffer();

    // calculate the Delays
    m_DelayCalculation->SetConfig(m_Conf);
    m_DelayCalculation->SetInputs(m_UsedLinesBuffer);
    m_DelayCalculation->Update();

    m_DelaysBuffer = m_DelayCalculation->GetGPUOutput()->GetGPUBuffer();

    m_ConfOld = m_Conf;
  }
}

void mitk::PhotoacousticOCLBeamformingFilter::Execute()
{
  cl_int clErr = 0;
  UpdateDataBuffers();

  clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &(this->m_UsedLinesBuffer));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_mem), &(this->m_DelaysBuffer));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_mem), &(this->m_ApodizationBuffer));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_ushort), &(this->m_ApodArraySize));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_uint), &(this->m_Conf.inputDim[0]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_uint), &(this->m_Conf.inputDim[1]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 8, sizeof(cl_uint), &(this->m_Conf.inputDim[2]));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 9, sizeof(cl_uint), &(this->m_Conf.ReconstructionLines));
  clErr |= clSetKernelArg(this->m_PixelCalculation, 10, sizeof(cl_uint), &(this->m_Conf.SamplesPerLine));

  // execute the filter on a 3D NDRange
  if (m_OutputDim[2] == 1 || m_ChunkSize[2] == 1)
  {
    if(!this->ExecuteKernelChunksInBatches(m_PixelCalculation, 2, m_ChunkSize, 16, 50))
      mitkThrow() << "openCL Error when executing Kernel";
  }
  else
  {
    if(!this->ExecuteKernelChunksInBatches(m_PixelCalculation, 3, m_ChunkSize, 16, 50))
      mitkThrow() << "openCL Error when executing Kernel";
  }

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
      case BeamformingSettings::BeamformingAlgorithm::DAS:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDAS", &clErr);
        break;
      }
      case BeamformingSettings::BeamformingAlgorithm::DMAS:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMAS", &clErr);
        break;
      }
      case BeamformingSettings::BeamformingAlgorithm::sDMAS:
      {
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "cksDMAS", &clErr);
        break;
      }
      default:
      {
        MITK_INFO << "No beamforming algorithm specified, setting to DAS";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDAS", &clErr);
        break;
      }
    }
    buildErr |= CHECK_OCL_ERR( clErr );
  }

  CHECK_OCL_ERR(clErr);

  return (OclFilter::IsInitialized() && buildErr );
}

void mitk::PhotoacousticOCLBeamformingFilter::SetInput(mitk::Image::Pointer image)
{
  OclDataSetToDataSetFilter::SetInput(image);

  m_InputImage = image;
  m_Conf.inputDim[0] = m_InputImage->GetDimension(0);
  m_Conf.inputDim[1] = m_InputImage->GetDimension(1);
  m_Conf.inputDim[2] = m_InputImage->GetDimension(2);
}

void mitk::PhotoacousticOCLBeamformingFilter::SetInput(void* data, unsigned int* dimensions, unsigned int BpE)
{
  OclDataSetToDataSetFilter::SetInput(data, dimensions[0] * dimensions[1] * dimensions[2], BpE);

  m_Conf.inputDim[0] = dimensions[0];
  m_Conf.inputDim[1] = dimensions[1];
  m_Conf.inputDim[2] = dimensions[2];
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
    outputImage->SetImportVolume(pData, 0, 0, mitk::Image::ImportMemoryManagementType::ManageMemory);
  }

  MITK_DEBUG << "Image Initialized.";

  return outputImage;
}

void* mitk::PhotoacousticOCLBeamformingFilter::GetOutput()
{
  return OclDataSetToDataSetFilter::GetOutput();
}
#endif
