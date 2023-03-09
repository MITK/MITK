/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkOclResourceServiceImpl_p_h
#define mitkOclResourceServiceImpl_p_h

#include <map>

//Micro Services
#include <usModuleActivator.h>
#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceProperties.h>

//ocl
#include "mitkOclResourceService.h"
#include "mitkOclUtils.h"
#include "mitkOclImageFormats.h"

#include <itkFastMutexLock.h>

US_USE_NAMESPACE

//todo add docu!

/** @struct OclContextCollection
 *  @brief An capsulation of all OpenCL context related variables needed for the OclResourceService implementation
 *
 *  The struct gets created on first call to GetContext in the OclResourceService and attepts to initialize all
 *  relevant parts, i.e. the context itself, the device and the command queue
 */
struct OclContextCollection{
public:
  OclContextCollection()
    : m_Context(nullptr), m_Devices(nullptr), m_CreateContextFailed(false)
  {
    cl_int clErr = 0;
    size_t szParmDataBytes;
    cl_platform_id cpPlatform;
    cl_device_id m_cdDevice;

    try{
      clErr = oclGetPlatformID( &cpPlatform);
      CHECK_OCL_ERR( clErr );

      clErr = clGetDeviceIDs( cpPlatform, CL_DEVICE_TYPE_GPU, 1, &m_cdDevice, nullptr);
      CHECK_OCL_ERR( clErr );

      this->m_Context = clCreateContext( 0, 1, &m_cdDevice, nullptr, nullptr, &clErr);
      m_CreateContextFailed = (clErr != CL_SUCCESS);

      // get the info size
      clErr = clGetContextInfo(m_Context, CL_CONTEXT_DEVICES, 0,nullptr, &szParmDataBytes );
      this->m_Devices = (cl_device_id*) malloc(szParmDataBytes);

      // get device info
      clErr = clGetContextInfo(m_Context, CL_CONTEXT_DEVICES, szParmDataBytes, m_Devices, nullptr);
      CHECK_OCL_ERR( clErr );

      // create command queue
      m_CommandQueue = clCreateCommandQueue(m_Context,  m_Devices[0], 0, &clErr);
      CHECK_OCL_ERR( clErr );

      this->PrintContextInfo( );

      // collect available image formats for current context
      this->m_ImageFormats = mitk::OclImageFormats::New();
      this->m_ImageFormats->SetGPUContext(m_Context);
    }
    catch( std::exception& e)
    {
      MITK_ERROR("OpenCL.ResourceService") << "Exception while creating context: \n" << e.what();
    }
  }

  ~OclContextCollection()
  {
    // if devices were allocated, delete
    if(m_Devices)
    {
      // TODO: Available first in OpenCL 1.2 : query the device for CL_PLATFORM_VERSION
      // through clGetPlatformInfo
      // clReleaseDevice(m_Devices[0]);

      delete [] m_Devices;
    }

    // if context was created release it
    if( m_Context )
      clReleaseContext( this->m_Context );
  }

  bool CanProvideContext() const
  {
    return ( m_Context != nullptr && !m_CreateContextFailed );
  }

  void PrintContextInfo() const
  {
    if( m_Devices )
    {
      oclPrintDeviceInfo( m_Devices[0] );
    }
  }

  /** The context */
  cl_context m_Context;

  /** Available OpenCL devices */
  cl_device_id* m_Devices;

  /** Class for handling (un)supported GPU image formats **/
  mitk::OclImageFormats::Pointer m_ImageFormats;

  /** The command queue*/
  cl_command_queue m_CommandQueue;

  bool m_CreateContextFailed;
};

class OclResourceServiceImpl
    : public OclResourceService
{

private:
  // define programmdata private class
  struct ProgramData
  {
    int counter;
    cl_program program;
    itk::FastMutexLock::Pointer mutex;
    ProgramData() :counter(1), program(nullptr)
    {}
  };

  typedef std::map< std::string, ProgramData > ProgramMapType;
  //typedef std::map< std::string, std::pair< int, cl_program> > ProgramMapType;

  mutable OclContextCollection* m_ContextCollection;

  /** Map containing all available (allready compiled) OpenCL Programs */
  ProgramMapType m_ProgramStorage;
  /** mutex for manipulating the program storage */
  itk::FastMutexLock::Pointer m_ProgramStorageMutex;

public:

  OclResourceServiceImpl();

  ~OclResourceServiceImpl();

  cl_context GetContext() const;

  cl_command_queue GetCommandQueue() const;

  cl_device_id GetCurrentDevice() const;

  bool GetIsFormatSupported(cl_image_format *);

  void PrintContextInfo() const;

  void InsertProgram(cl_program _program_in, std::string name, bool forceOverride=true);

  cl_program GetProgram(const std::string&name);

  void InvalidateStorage();

  void RemoveProgram(const std::string&name);

  unsigned int GetMaximumImageSize(unsigned int dimension, cl_mem_object_type _imagetype);
};

#endif
