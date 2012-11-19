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

#include "mitkOclFilter.h"

#include "mitkLogMacros.h"
#include "mitkOclUtils.h"

#include "mitkDataNodeFactory.h"
#include "mitkStandardFileLocations.h"

#include "mitkConfig.h"

#include "mitkOpenCLActivator.h"


mitk::OclFilter::OclFilter()
  : m_clFile(),
    m_clSourcePath(NULL),
    m_clCompilerFlags(NULL),
    m_clProgram(NULL),
    m_preambel(" "),
    m_Initialized(false),
    m_commandQue(NULL),
    m_FilterID("mitkOclFilter")
{
  this->SetSourcePath("Modules/OpenCL/ShaderSources");
  m_clCompilerFlags = "";
}

mitk::OclFilter::OclFilter(const char *_filename)
  : m_FilterID(_filename)
{
  m_commandQue = NULL;
  m_clProgram = NULL;
  m_clCompilerFlags = "";
  this->SetSourceFile( _filename);
  this->SetSourcePath("/Modules/OpenCL/ShaderSources");
}

mitk::OclFilter::~OclFilter()
{
  MITK_INFO << "OclFilter Destructor \n";

  // release program
  if (m_clProgram)
  {
    cl_int clErr = 0;

    // remove program from storage
    OpenCLActivator::GetResourceServiceRef()->RemoveProgram(m_FilterID);

    // release program
    clErr = clReleaseProgram(this->m_clProgram);
    CHECK_OCL_ERR(clErr);
  }

}

bool mitk::OclFilter::ExecuteKernel( cl_kernel kernel, unsigned int workSizeDim )
{
  cl_int clErr = 0;

  clErr = clEnqueueNDRangeKernel( this->m_commandQue, kernel, workSizeDim,
                                  NULL, this->m_globalWorkSize, m_localWorkSize, 0, NULL, NULL);

  CHECK_OCL_ERR( clErr );

  return ( clErr == CL_SUCCESS );
}


bool mitk::OclFilter::Initialize()
{
  m_commandQue = OpenCLActivator::GetResourceServiceRef()->GetCommandQueue();

  cl_int clErr = 0;
  m_Initialized = CHECK_OCL_ERR(clErr);

  if ((m_clSource==NULL) && (m_clFile.empty()))
  {
    MITK_ERROR<<"No OpenCL Source FILE specified";
    return false;
  }

  if (m_clProgram == NULL)
  {
    try
    {
      this->m_clProgram = OpenCLActivator::GetResourceServiceRef()->GetProgram( this->m_FilterID );
    }
    catch(const mitk::Exception& e)
    {
      MITK_INFO << "Program not stored in resource manager, compiling.";
      this->CompileSource("\n");
    }
  }

  return m_Initialized;
}

void mitk::OclFilter::SetSourceFile(const char *_filename)
{
  MITK_INFO("ocl.filter") << "Setting source [" << _filename <<" ]";

  std::string clsourceDir = MITK_ROOT;
  clsourceDir  += m_clSourcePath;

  mitk::StandardFileLocations::GetInstance()->AddDirectoryForSearch( clsourceDir.c_str(), true);
  // search for file
  clsourceDir = mitk::StandardFileLocations::GetInstance()->FindFile( _filename);

  m_clFile = clsourceDir;
}

void mitk::OclFilter::CompileSource(const char *preambel)
{
  if (m_clFile.empty() && m_clSource == NULL)
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
  m_clSource = oclLoadProgramSource( m_clFile.c_str(), this->m_preambel, &szKernelLength);

  if (m_clSource != NULL)
  {
    m_clProgram = clCreateProgramWithSource(gpuContext, 1, (const char**)&m_clSource, &szKernelLength, &clErr);
    CHECK_OCL_ERR(clErr);

    // build the source code
    MITK_INFO << "Building Program Source\n";
    std::string compilerOptions = "";

    compilerOptions.append(m_clCompilerFlags);
    // activate the include compiler flag
    compilerOptions.append(" -I");
    // set the path of the current gpu source dir as opencl
    // include folder
    compilerOptions.append(MITK_ROOT);
    compilerOptions.append(m_clSourcePath);

    clErr = clBuildProgram(m_clProgram, 0, NULL, compilerOptions.c_str(), NULL, NULL);
    CHECK_OCL_ERR(clErr);

    // if OpenCL Source build failed
    if (clErr != CL_SUCCESS)
    {
      MITK_ERROR("ocl.filter") << "Failed to build source";
      oclLogBuildInfo(m_clProgram,  OpenCLActivator::GetResourceServiceRef()->GetCurrentDevice() );
      oclLogBinary(m_clProgram,  OpenCLActivator::GetResourceServiceRef()->GetCurrentDevice() );
      m_Initialized = false;

    }

    // store the succesfully build program into the program storage provided by the resource service
    OpenCLActivator::GetResourceServiceRef()->InsertProgram(m_clProgram, m_FilterID, true);
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
  this->m_localWorkSize[0] = locx;
  this->m_localWorkSize[1] = locy;
  this->m_localWorkSize[2] = locz;

  // estimate the global work size
  this->m_globalWorkSize[0] = iDivUp( dimx, this->m_localWorkSize[0]) *this->m_localWorkSize[0];
  this->m_globalWorkSize[1] = iDivUp( dimy, this->m_localWorkSize[1]) * this->m_localWorkSize[1];
  if( dimz <= 1 )
    this->m_globalWorkSize[2] = 1;
  else
    this->m_globalWorkSize[2] = iDivUp( dimz, this->m_localWorkSize[2]) * this->m_localWorkSize[2];
}

void mitk::OclFilter::SetSourcePreambel(const char* _preambel)
{
  this->m_preambel = _preambel;
}

void mitk::OclFilter::SetSourcePath(const char * _path)
{
  m_clSourcePath = _path;
}

void mitk::OclFilter::SetCompilerFlags(const char * _flags)
{
  m_clCompilerFlags = _flags;
}


bool mitk::OclFilter::IsInitialized()
{
  return m_Initialized;
}
