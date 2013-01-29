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

//Ocl
#include "mitkOclFilter.h"
#include "mitkOclUtils.h"
#include "mitkOpenCLActivator.h"

//Mitk
#include <mitkLogMacros.h>
#include <mitkDataNodeFactory.h>
#include <mitkStandardFileLocations.h>
#include <mitkConfig.h>

mitk::OclFilter::OclFilter()
  : m_ClFile(),
    m_ClSource(NULL),
    m_ClCompilerFlags(""),
    m_ClProgram(NULL),
    m_CommandQue(NULL),
    m_FilterID("mitkOclFilter"),
    m_Preambel(" "),
    m_Initialized(false)
{
  m_ClSourcePath = MITK_ROOT;
  m_ClSourcePath += "Modules/OpenCL/ShaderSources";
}

mitk::OclFilter::OclFilter(const char* filename)
  : m_ClFile(),
    m_ClSource(NULL),
    m_ClCompilerFlags(""),
    m_ClProgram(NULL),
    m_CommandQue(NULL),
    m_FilterID(filename),
    m_Preambel(" "),
    m_Initialized(false)
{
  m_ClSourcePath = MITK_ROOT;
  m_ClSourcePath += "Modules/OpenCL/ShaderSources";
}

mitk::OclFilter::~OclFilter()
{
  MITK_DEBUG << "OclFilter Destructor";

  // release program
  if (m_ClProgram)
  {
    cl_int clErr = 0;

    // remove program from storage
    OpenCLActivator::GetResourceServiceRef()->RemoveProgram(m_FilterID);

    // release program
    clErr = clReleaseProgram(this->m_ClProgram);
    CHECK_OCL_ERR(clErr);
  }

}

bool mitk::OclFilter::ExecuteKernel( cl_kernel kernel, unsigned int workSizeDim )
{
  cl_int clErr = 0;

  clErr = clEnqueueNDRangeKernel( this->m_CommandQue, kernel, workSizeDim,
                                  NULL, this->m_GlobalWorkSize, m_LocalWorkSize, 0, NULL, NULL);

  CHECK_OCL_ERR( clErr );

  return ( clErr == CL_SUCCESS );
}


bool mitk::OclFilter::Initialize()
{
  m_CommandQue = OpenCLActivator::GetResourceServiceRef()->GetCommandQueue();

  cl_int clErr = 0;
  m_Initialized = CHECK_OCL_ERR(clErr);

  if ((m_ClSource==NULL) && (m_ClFile.empty()))
  {
    MITK_ERROR<<"No OpenCL Source FILE specified";
    return false;
  }

  if (m_ClProgram == NULL)
  {
    try
    {
      this->m_ClProgram = OpenCLActivator::GetResourceServiceRef()->GetProgram( this->m_FilterID );
    }
    catch(const mitk::Exception& e)
    {
      MITK_INFO << "Program not stored in resource manager, compiling.";
      this->CompileSource();
    }
  }

  return m_Initialized;
}

void mitk::OclFilter::SetSourceFile(const char* filename)
{
  MITK_DEBUG("ocl.filter") << "Setting source [" <<  filename <<" ]";

  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( m_ClSourcePath.c_str(), true);
  // search for file
  m_ClFile = mitk::StandardFileLocations::GetInstance()->FindFile(  filename);
}

void mitk::OclFilter::CompileSource()
{
  if (m_ClFile.empty() && m_ClSource == NULL)
  {
    MITK_ERROR("ocl.filter") << "No shader source file was set";
    return;
  }

  // help variables
  size_t szKernelLength;
  int clErr = 0;

  //get a valid opencl context
  cl_context gpuContext = OpenCLActivator::GetResourceServiceRef()->GetContext();

  // load the program source from file
  m_ClSource = oclLoadProgramSource( m_ClFile.c_str(), this->m_Preambel, &szKernelLength);

  if (m_ClSource != NULL)
  {
    m_ClProgram = clCreateProgramWithSource(gpuContext, 1, (const char**)&m_ClSource, &szKernelLength, &clErr);
    CHECK_OCL_ERR(clErr);

    // build the source code
    MITK_DEBUG << "Building Program Source";
    std::string compilerOptions = "";

    compilerOptions.append(m_ClCompilerFlags);
    // activate the include compiler flag
    compilerOptions.append(" -I");
    // set the path of the current gpu source dir as opencl
    // include folder
    compilerOptions.append(m_ClSourcePath.c_str());
    MITK_DEBUG("ocl.filter") << "cl compiler flags: " << compilerOptions.c_str();

    clErr = clBuildProgram(m_ClProgram, 0, NULL, compilerOptions.c_str(), NULL, NULL);
    CHECK_OCL_ERR(clErr);

    // if OpenCL Source build failed
    if (clErr != CL_SUCCESS)
    {
      MITK_ERROR("ocl.filter") << "Failed to build source";
      oclLogBuildInfo(m_ClProgram,  OpenCLActivator::GetResourceServiceRef()->GetCurrentDevice() );
      oclLogBinary(m_ClProgram,  OpenCLActivator::GetResourceServiceRef()->GetCurrentDevice() );
      m_Initialized = false;
    }

    // store the succesfully build program into the program storage provided by the resource service
    OpenCLActivator::GetResourceServiceRef()->InsertProgram(m_ClProgram, m_FilterID, true);
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

  // estimate the global work size
  this->m_GlobalWorkSize[0] = iDivUp( dimx, this->m_LocalWorkSize[0]) *this->m_LocalWorkSize[0];
  this->m_GlobalWorkSize[1] = iDivUp( dimy, this->m_LocalWorkSize[1]) * this->m_LocalWorkSize[1];
  if( dimz <= 1 )
    this->m_GlobalWorkSize[2] = 1;
  else
    this->m_GlobalWorkSize[2] = iDivUp( dimz, this->m_LocalWorkSize[2]) * this->m_LocalWorkSize[2];
}

void mitk::OclFilter::SetSourcePreambel(const char* preambel)
{
  this->m_Preambel = preambel;
}

void mitk::OclFilter::SetSourcePath(const char* path)
{
  m_ClSourcePath = path;
}

void mitk::OclFilter::SetCompilerFlags(const char* flags)
{
  m_ClCompilerFlags = flags;
}


bool mitk::OclFilter::IsInitialized()
{
  return m_Initialized;
}
