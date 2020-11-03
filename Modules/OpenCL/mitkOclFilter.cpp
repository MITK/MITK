/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

//Ocl
#include "mitkOclFilter.h"
#include "mitkOclUtils.h"
#include "mitkOpenCLActivator.h"


//Mitk
#include <mitkLogMacros.h>
#include <mitkConfig.h>

//usService
#include "usServiceReference.h"
#include <usServiceRegistration.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usModule.h>
#include <usModuleResource.h>
#include <usModuleResourceStream.h>

//standard library
#include <thread>

mitk::OclFilter::OclFilter()
  : m_ClCompilerFlags(""),
    m_ClProgram(nullptr),
    m_CommandQue(nullptr),
    m_FilterID("mitkOclFilter"),
    m_Preambel(" "),
    m_Initialized(false)
{
}

mitk::OclFilter::OclFilter(const char* filename)
  : m_ClCompilerFlags(""),
    m_ClProgram(nullptr),
    m_CommandQue(nullptr),
    m_FilterID(filename),
    m_Preambel(" "),
    m_Initialized(false)
{
  m_ClFiles.push_back(filename);
}

mitk::OclFilter::~OclFilter()
{
  MITK_DEBUG << "OclFilter Destructor";

  // release program
  if (m_ClProgram)
  {
    us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
    OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

    // remove program from storage
    resources->RemoveProgram(m_FilterID);
  }
}

bool mitk::OclFilter::ExecuteKernel( cl_kernel kernel, unsigned int workSizeDim )
{
  cl_int clErr = 0;

  clErr = clEnqueueNDRangeKernel( this->m_CommandQue, kernel, workSizeDim,
                                  nullptr, this->m_GlobalWorkSize, m_LocalWorkSize, 0, nullptr, nullptr);

  CHECK_OCL_ERR( clErr );

  return ( clErr == CL_SUCCESS );
}

bool mitk::OclFilter::ExecuteKernelChunks( cl_kernel kernel, unsigned int workSizeDim, size_t* chunksDim )
{
  size_t offset[3] ={0, 0, 0};
  cl_int clErr = 0;

  if(workSizeDim == 2)
  {
    for(offset[0] = 0; offset[0] < m_GlobalWorkSize[0]; offset[0] += chunksDim[0])
    {
      for(offset[1] = 0; offset[1] < m_GlobalWorkSize[1]; offset[1] += chunksDim[1])
      {
        clErr |= clEnqueueNDRangeKernel(this->m_CommandQue, kernel, workSizeDim,
          offset, chunksDim, m_LocalWorkSize, 0, nullptr, nullptr);
      }
    }
  }
  else if(workSizeDim == 3)
  {
    for(offset[0] = 0; offset[0] < m_GlobalWorkSize[0]; offset[0] += chunksDim[0])
    {
      for(offset[1] = 0; offset[1] < m_GlobalWorkSize[1]; offset[1] += chunksDim[1])
      {
        for(offset[2] = 0; offset[2] < m_GlobalWorkSize[2]; offset[2] += chunksDim[2])
        {
          clErr |= clEnqueueNDRangeKernel( this->m_CommandQue, kernel, workSizeDim,
                                          offset, chunksDim, m_LocalWorkSize, 0, nullptr, nullptr);
        }
      }
    }
  }

  CHECK_OCL_ERR(clErr);

  return ( clErr == CL_SUCCESS );
}

bool mitk::OclFilter::ExecuteKernelChunksInBatches(cl_kernel kernel, unsigned int workSizeDim, size_t* chunksDim, size_t batchSize, int waitTimems)
{
  size_t offset[3] = { 0, 0, 0 };
  cl_int clErr = 0;

  unsigned int currentChunk = 0;
  cl_event* waitFor = new cl_event[batchSize];

  if (workSizeDim == 2)
  {
    for (offset[0] = 0; offset[0] < m_GlobalWorkSize[0]; offset[0] += chunksDim[0])
    {
      for (offset[1] = 0; offset[1] < m_GlobalWorkSize[1]; offset[1] += chunksDim[1])
      {
        if (currentChunk % batchSize == 0 && currentChunk != 0)
        {
          clWaitForEvents(batchSize, &waitFor[0]);
          std::this_thread::sleep_for(std::chrono::milliseconds(waitTimems));
          clErr |= clEnqueueNDRangeKernel(this->m_CommandQue, kernel, workSizeDim,
            offset, chunksDim, m_LocalWorkSize, 0, nullptr, &waitFor[0]);
        }
        else
        {
          clErr |= clEnqueueNDRangeKernel(this->m_CommandQue, kernel, workSizeDim,
            offset, chunksDim, m_LocalWorkSize, 0, nullptr, &waitFor[currentChunk % batchSize]);
        }
        currentChunk++;
      }
    }
  }
  else if (workSizeDim == 3)
  {
    for (offset[0] = 0; offset[0] < m_GlobalWorkSize[0]; offset[0] += chunksDim[0])
    {
      for (offset[1] = 0; offset[1] < m_GlobalWorkSize[1]; offset[1] += chunksDim[1])
      {
        for (offset[2] = 0; offset[2] < m_GlobalWorkSize[2]; offset[2] += chunksDim[2])
        {
          if (currentChunk % batchSize == 0 && currentChunk != 0)
          {
            clWaitForEvents(batchSize, &waitFor[0]);
            std::this_thread::sleep_for(std::chrono::milliseconds(waitTimems));
            clErr |= clEnqueueNDRangeKernel(this->m_CommandQue, kernel, workSizeDim,
              offset, chunksDim, m_LocalWorkSize, 0, nullptr, &waitFor[0]);
          }
          else
          {
            clErr |= clEnqueueNDRangeKernel(this->m_CommandQue, kernel, workSizeDim,
              offset, chunksDim, m_LocalWorkSize, 0, nullptr, &waitFor[currentChunk % batchSize]);
          }
          currentChunk++;
        }
      }
    }
  }
  CHECK_OCL_ERR(clErr);

  return (clErr == CL_SUCCESS);
}


bool mitk::OclFilter::Initialize()
{
  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

  m_CommandQue = resources->GetCommandQueue();

  cl_int clErr = 0;
  m_Initialized = CHECK_OCL_ERR(clErr);

  if ( m_ClFiles.empty())
  {
    MITK_ERROR<<"No OpenCL Source FILE specified";
    return false;
  }

  if (m_ClProgram == nullptr)
  {
    try
    {
      this->m_ClProgram = resources->GetProgram( this->m_FilterID );
    }
    catch(const mitk::Exception& e)
    {
      MITK_INFO << "Program not stored in resource manager, compiling. " << e;
      this->CompileSource();
    }
  }

  return m_Initialized;
}

void mitk::OclFilter::LoadSourceFiles(CStringList &sourceCode, ClSizeList &sourceCodeSize)
{
  for( CStringList::iterator it = m_ClFiles.begin(); it != m_ClFiles.end(); ++it )
  {
    MITK_DEBUG << "Load file :" << *it;
    us::ModuleResource mdr = GetModule()->GetResource(*it);

    if( !mdr.IsValid() )
      MITK_WARN << "Could not load resource: " << mdr.GetName() << " is invalid!";

    us::ModuleResourceStream rss(mdr);

    // read resource file to a string
    std::istreambuf_iterator<char> eos;
    std::string source(std::istreambuf_iterator<char>(rss), eos);

    // add preambel and build up string to compile
    std::string src(m_Preambel);
    src.append("\n");
    src.append(source);

    // allocate new char buffer
    char* tmp = new char[src.size() + 1];
    strcpy(tmp,src.c_str());

    // add source to list
    sourceCode.push_back((const char*)tmp);
    sourceCodeSize.push_back(src.size());
  }
}

void mitk::OclFilter::CompileSource()
{
  // helper variable
  int clErr = 0;
  CStringList sourceCode;
  ClSizeList sourceCodeSize;

  if (m_ClFiles.empty())
  {
    MITK_ERROR("ocl.filter") << "No shader source file was set";
    return;
  }

  //get a valid opencl context
  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

  cl_context gpuContext = resources->GetContext();
  // load the program source from file
  LoadSourceFiles(sourceCode, sourceCodeSize);

  if ( !sourceCode.empty() )
  {
    // create program from all files in the file list
    m_ClProgram = clCreateProgramWithSource(gpuContext, sourceCode.size(), &sourceCode[0], &sourceCodeSize[0], &clErr);
    CHECK_OCL_ERR(clErr);

    // build the source code
    MITK_DEBUG << "Building Program Source";
    std::string compilerOptions = "";
    compilerOptions.append(m_ClCompilerFlags);

    MITK_DEBUG("ocl.filter") << "cl compiler flags: " << compilerOptions.c_str();

    clErr = clBuildProgram(m_ClProgram, 0, nullptr, compilerOptions.c_str(), nullptr, nullptr);
    CHECK_OCL_ERR(clErr);

    // if OpenCL Source build failed
    if (clErr != CL_SUCCESS)
    {
      MITK_ERROR("ocl.filter") << "Failed to build source";
      oclLogBuildInfo(m_ClProgram, resources->GetCurrentDevice() );
      oclLogBinary(m_ClProgram, resources->GetCurrentDevice() );
      m_Initialized = false;
    }

    // store the succesfully build program into the program storage provided by the resource service
    resources->InsertProgram(m_ClProgram, m_FilterID, true);

    // free the char buffers with the source code
    for( CStringList::iterator it = sourceCode.begin(); it != sourceCode.end(); ++it )
    {
      delete[] *it;
    }
  }
  else
  {
    MITK_ERROR("ocl.filter") << "Could not load from source";
    m_Initialized = false;
  }
}

void mitk::OclFilter::SetWorkingSize(unsigned int locx, unsigned int dimx, unsigned int locy, unsigned int dimy, unsigned int locz, unsigned int dimz)
{
  // set the local work size
  this->m_LocalWorkSize[0] = locx;
  this->m_LocalWorkSize[1] = locy;
  this->m_LocalWorkSize[2] = locz;

  this->m_GlobalWorkSize[0] = dimx;
  this->m_GlobalWorkSize[1] = dimy;
  this->m_GlobalWorkSize[2] = dimz;

  // estimate the global work size
  this->m_GlobalWorkSize[0] = iDivUp( dimx, this->m_LocalWorkSize[0]) * this->m_LocalWorkSize[0];

  if ( dimy > 1)
    this->m_GlobalWorkSize[1] = iDivUp( dimy, this->m_LocalWorkSize[1]) * this->m_LocalWorkSize[1];
  if( dimz > 1 )
    this->m_GlobalWorkSize[2] = iDivUp( dimz, this->m_LocalWorkSize[2]) * this->m_LocalWorkSize[2];
}

void mitk::OclFilter::SetSourcePreambel(const char* preambel)
{
  this->m_Preambel = preambel;
}

void mitk::OclFilter::AddSourceFile(const char* filename)
{
  m_ClFiles.push_back(filename);
}

void mitk::OclFilter::SetCompilerFlags(const char* flags)
{
  m_ClCompilerFlags = flags;
}


bool mitk::OclFilter::IsInitialized()
{
  return m_Initialized;
}

unsigned long mitk::OclFilter::GetDeviceMemory()
{
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(GetModuleContext()->GetServiceReference<OclResourceService>());
  auto device = resources->GetCurrentDevice();
  return oclGetGlobalMemSize(device);
}
