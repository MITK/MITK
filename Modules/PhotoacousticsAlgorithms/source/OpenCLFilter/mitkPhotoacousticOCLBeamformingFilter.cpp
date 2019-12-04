/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#if defined(PHOTOACOUSTICS_USE_GPU) || DOXYGEN

#include "./OpenCLFilter/mitkPhotoacousticOCLBeamformingFilter.h"
#include "usServiceReference.h"

mitk::PhotoacousticOCLBeamformingFilter::PhotoacousticOCLBeamformingFilter(BeamformingSettings::Pointer settings) :
  m_PixelCalculation(NULL),
  m_inputSlices(1),
  m_Conf(settings),
  m_InputImage(mitk::Image::New()),
  m_ApodizationBuffer(nullptr),
  m_DelaysBuffer(nullptr),
  m_UsedLinesBuffer(nullptr),
  m_ElementHeightsBuffer(nullptr),
  m_ElementPositionsBuffer(nullptr)
{
  MITK_INFO << "Instantiating OCL beamforming Filter...";
  this->AddSourceFile("DAS.cl");
  this->AddSourceFile("DMAS.cl");
  this->AddSourceFile("sDMAS.cl");
  this->m_FilterID = "OpenCLBeamformingFilter";

  this->Initialize();

  unsigned int dim[] = { 128, 2048, 2 };

  m_InputImage->Initialize(mitk::MakeScalarPixelType<float>(), 3, dim);

  m_ChunkSize[0] = 128;
  m_ChunkSize[1] = 128;
  m_ChunkSize[2] = 8;

  m_UsedLinesCalculation = mitk::OCLUsedLinesCalculation::New(m_Conf);
  m_DelayCalculation = mitk::OCLDelayCalculation::New(m_Conf);
  MITK_INFO << "Instantiating OCL beamforming Filter...[Done]";
}

mitk::PhotoacousticOCLBeamformingFilter::~PhotoacousticOCLBeamformingFilter()
{
  if (this->m_PixelCalculation)
  {
    clReleaseKernel(m_PixelCalculation);
  }

  if (m_ApodizationBuffer) clReleaseMemObject(m_ApodizationBuffer);
  if (m_ElementHeightsBuffer) clReleaseMemObject(m_ElementHeightsBuffer);
  if (m_ElementPositionsBuffer) clReleaseMemObject(m_ElementPositionsBuffer);
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
    mitkThrow() << "Filter is not initialized. Cannot update.";
  }
  else {
    // Execute
    this->Execute();
  }
}

void mitk::PhotoacousticOCLBeamformingFilter::UpdateDataBuffers()
{
  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);
  cl_context gpuContext = resources->GetContext();

  //Initialize the Output
  try
  {
    size_t outputSize = (size_t)m_Conf->GetReconstructionLines() * (size_t)m_Conf->GetSamplesPerLine() *
      (size_t)m_inputSlices;
    m_OutputDim[0] = m_Conf->GetReconstructionLines();
    m_OutputDim[1] = m_Conf->GetSamplesPerLine();
    m_OutputDim[2] = m_inputSlices;
    this->InitExec(this->m_PixelCalculation, m_OutputDim, outputSize, sizeof(float));
  }
  catch (const mitk::Exception& e)
  {
    MITK_ERROR << "Caught exception while initializing filter: " << e.what();
    return;
  }

  cl_int clErr = 0;
  MITK_DEBUG << "Updating GPU Buffers for new configuration";

  // create the apodisation buffer
  if (m_Apodisation == nullptr)
  {
    MITK_INFO << "No apodisation function set; Beamforming will be done without any apodisation.";
    m_Apodisation = new float[1]{ 1 };
    m_ApodArraySize = 1;
  }

  if (m_ApodizationBuffer) clReleaseMemObject(m_ApodizationBuffer);

  this->m_ApodizationBuffer = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * m_ApodArraySize, const_cast<float*>(m_Apodisation), &clErr);
  CHECK_OCL_ERR(clErr);

  if (m_ElementHeightsBuffer) clReleaseMemObject(m_ElementHeightsBuffer);
  this->m_ElementHeightsBuffer = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * m_Conf->GetTransducerElements(), const_cast<float*>(m_Conf->GetElementHeights()), &clErr);
  CHECK_OCL_ERR(clErr);

  if (m_ElementPositionsBuffer) clReleaseMemObject(m_ElementPositionsBuffer);
  this->m_ElementPositionsBuffer = clCreateBuffer(gpuContext, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(float) * m_Conf->GetTransducerElements(), const_cast<float*>(m_Conf->GetElementPositions()), &clErr);
  CHECK_OCL_ERR(clErr);
  // calculate used lines

  m_UsedLinesCalculation->SetElementPositionsBuffer(m_ElementPositionsBuffer);
  m_UsedLinesCalculation->SetElementHeightsBuffer(m_ElementHeightsBuffer);
  m_UsedLinesCalculation->Update();
  m_UsedLinesBuffer = m_UsedLinesCalculation->GetGPUOutput()->GetGPUBuffer();

  // calculate the Delays
  m_DelayCalculation->SetInputs(m_UsedLinesBuffer);
  m_DelayCalculation->Update();

  m_DelaysBuffer = m_DelayCalculation->GetGPUOutput()->GetGPUBuffer();
}

void mitk::PhotoacousticOCLBeamformingFilter::Execute()
{
  cl_int clErr = 0;
  UpdateDataBuffers();
  if (m_Conf->GetGeometry() == mitk::BeamformingSettings::ProbeGeometry::Linear)
  {
    unsigned int reconstructionLines = this->m_Conf->GetReconstructionLines();
    unsigned int samplesPerLine = this->m_Conf->GetSamplesPerLine();

    clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &(this->m_UsedLinesBuffer));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_mem), &(this->m_DelaysBuffer));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_mem), &(this->m_ApodizationBuffer));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_ushort), &(this->m_ApodArraySize));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[0]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[1]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 8, sizeof(cl_uint), &(m_inputSlices));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 9, sizeof(cl_uint), &(reconstructionLines));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 10, sizeof(cl_uint), &(samplesPerLine));
  }
  else
  {
    int reconstructionLines = this->m_Conf->GetReconstructionLines();
    int samplesPerLine = this->m_Conf->GetSamplesPerLine();
    float totalSamples_i = (float)(m_Conf->GetReconstructionDepth()) / (float)(m_Conf->GetSpeedOfSound() * m_Conf->GetTimeSpacing());
    totalSamples_i = totalSamples_i <= m_Conf->GetInputDim()[1] ? totalSamples_i : m_Conf->GetInputDim()[1];
    float horizontalExtent = m_Conf->GetHorizontalExtent();
    float mult = 1 / (this->m_Conf->GetTimeSpacing() * this->m_Conf->GetSpeedOfSound());
    char isPAImage = (char)m_Conf->GetIsPhotoacousticImage();

    clErr = clSetKernelArg(this->m_PixelCalculation, 2, sizeof(cl_mem), &(this->m_ElementHeightsBuffer));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 3, sizeof(cl_mem), &(this->m_ElementPositionsBuffer));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 4, sizeof(cl_mem), &(this->m_ApodizationBuffer));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 5, sizeof(cl_ushort), &(this->m_ApodArraySize));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 6, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[0]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 7, sizeof(cl_uint), &(this->m_Conf->GetInputDim()[1]));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 8, sizeof(cl_int), &(m_inputSlices));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 9, sizeof(cl_int), &(reconstructionLines));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 10, sizeof(cl_int), &(samplesPerLine));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 11, sizeof(cl_float), &(totalSamples_i));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 12, sizeof(cl_float), &(horizontalExtent));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 13, sizeof(cl_float), &(mult));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 14, sizeof(cl_char), &(isPAImage));
    clErr |= clSetKernelArg(this->m_PixelCalculation, 15, sizeof(cl_mem), &(this->m_UsedLinesBuffer));
  }
  // execute the filter on a 2D/3D NDRange
  if (m_OutputDim[2] == 1 || m_ChunkSize[2] == 1)
  {
    if (!this->ExecuteKernelChunksInBatches(m_PixelCalculation, 2, m_ChunkSize, m_inputSlices, 50))
      mitkThrow() << "openCL Error when executing Kernel";
  }
  else
  {
    if (!this->ExecuteKernelChunksInBatches(m_PixelCalculation, 3, m_ChunkSize, m_inputSlices, 50))
      mitkThrow() << "openCL Error when executing Kernel";
  }

  // signalize the GPU-side data changed
  m_Output->Modified(GPU_DATA);
}

us::Module *mitk::PhotoacousticOCLBeamformingFilter::GetModule()
{
  return us::GetModuleContext()->GetModule();
}

bool mitk::PhotoacousticOCLBeamformingFilter::Initialize()
{
  bool buildErr = true;
  cl_int clErr = 0;

  if (OclFilter::Initialize())
  {
    if (m_Conf->GetGeometry() == mitk::BeamformingSettings::ProbeGeometry::Linear)
    {
      switch (m_Conf->GetAlgorithm())
      {
      case BeamformingSettings::BeamformingAlgorithm::DAS:
      {
        MITK_INFO << "DAS bf";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDAS", &clErr);
        break;
      }
      case BeamformingSettings::BeamformingAlgorithm::DMAS:
      {
        MITK_INFO << "DMAS bf";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMAS", &clErr);
        break;
      }
      case BeamformingSettings::BeamformingAlgorithm::sDMAS:
      {
        MITK_INFO << "sDMAS bf";
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
    }
    else
    {
      switch (m_Conf->GetAlgorithm())
      {
      case BeamformingSettings::BeamformingAlgorithm::DAS:
      {
        MITK_INFO << "DAS bf";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDAS_g", &clErr);
        break;
      }
      case BeamformingSettings::BeamformingAlgorithm::DMAS:
      {
        MITK_INFO << "DMAS bf";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDMAS_g", &clErr);
        break;
      }
      case BeamformingSettings::BeamformingAlgorithm::sDMAS:
      {
        MITK_INFO << "sDMAS bf";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "cksDMAS_g", &clErr);
        break;
      }
      default:
      {
        MITK_INFO << "No beamforming algorithm specified, setting to DAS";
        this->m_PixelCalculation = clCreateKernel(this->m_ClProgram, "ckDAS_g", &clErr);
        break;
      }
      }
    }
    buildErr |= CHECK_OCL_ERR(clErr);
  }

  CHECK_OCL_ERR(clErr);

  return (OclFilter::IsInitialized() && buildErr);
}

void mitk::PhotoacousticOCLBeamformingFilter::SetInput(mitk::Image::Pointer image)
{
  OclDataSetToDataSetFilter::SetInput(image);
  m_InputImage = image;
  m_inputSlices = image->GetDimension(2);
}

void mitk::PhotoacousticOCLBeamformingFilter::SetInput(void* data, unsigned int* dimensions, unsigned int BpE)
{
  OclDataSetToDataSetFilter::SetInput(data, dimensions[0] * dimensions[1] * dimensions[2], BpE);
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
    outputImage->SetImportVolume(pData, 0, 0, mitk::Image::ImportMemoryManagementType::CopyMemory);
    free(pData);
  }

  MITK_DEBUG << "Image Initialized.";

  return outputImage;
}

void* mitk::PhotoacousticOCLBeamformingFilter::GetOutput()
{
  return OclDataSetToDataSetFilter::GetOutput();
}
#endif
