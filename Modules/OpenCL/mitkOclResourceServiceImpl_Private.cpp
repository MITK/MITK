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

#include "mitkOclResourceServiceImpl_p.h"

OclResourceService::~OclResourceService()
{

}

OclResourceServiceImpl::OclResourceServiceImpl()
  : m_Context(NULL), m_Devices(NULL), m_ProgramStorage()
{
  this->CreateContext();
}

OclResourceServiceImpl::~OclResourceServiceImpl()
{
  // if map non-empty, release all remaining
  if( m_ProgramStorage.size() )
  {
    ProgramMapType::iterator it = m_ProgramStorage.begin();
    while(it != m_ProgramStorage.end() )
    {
      clReleaseProgram( it->second );
      m_ProgramStorage.erase( it++ );
    }
  }

  // if devices were allocated, delete
  if(m_Devices)
  {
    // TODO: Available first in OpenCL 1.2 : query the device for CL_PLATFORM_VERSION
    // through clGetPlatformInfo
    // clReleaseDevice(m_Devices[0]);

    delete [] m_Devices;
  }

  // if context was created release it
  if(m_Context)
    clReleaseContext( this->m_Context );
}

cl_context OclResourceServiceImpl::GetContext() const
{
  return m_Context;
}

cl_command_queue OclResourceServiceImpl::GetCommandQueue() const
{
  // check if queue valid
  cl_int clErr = clGetCommandQueueInfo( m_CommandQueue, CL_QUEUE_CONTEXT, NULL, NULL, NULL );
  if( clErr != CL_SUCCESS )
  {
    MITK_WARN << "Have no valid command queue. Query returned : " << GetOclErrorAsString( clErr );
    return NULL;
  }

  return this->m_CommandQueue;
}

cl_device_id OclResourceServiceImpl::GetCurrentDevice() const
{
  return m_Devices[0];
}

bool OclResourceServiceImpl::GetIsFormatSupported(cl_image_format *fmt)
{
  cl_image_format temp;
  temp.image_channel_data_type = fmt->image_channel_data_type;
  temp.image_channel_order = fmt->image_channel_order;

  return (this->m_ImageFormats->GetNearestSupported(&temp, fmt));
}

void OclResourceServiceImpl::PrintContextInfo()
{
  if( m_Context == NULL){
    MITK_ERROR("OpenCL.ResourceService") << "No OpenCL Context available ";
  }
  else
  {
    oclPrintDeviceInfo( m_Devices[0] );
  }
}

void OclResourceServiceImpl::InsertProgram(cl_program _program_in, std::string name, bool forceOverride)
{
  std::pair< ProgramMapType::iterator, bool> retValue;
  typedef std::pair< std::string, cl_program > MapElemPair;

  retValue = m_ProgramStorage.insert( MapElemPair(name, _program_in) );
  // insertion failed, i.e. a program with same name exists
  if( !retValue.second )
  {
    std::string overrideMsg("");
    if( forceOverride )
    {
      // overwrite old instance
      m_ProgramStorage.at(name) = _program_in;
      overrideMsg +=" The old program was overwritten!";
    }

    MITK_WARN("OpenCL.ResourceService") << "The program " << name << " already exists." << overrideMsg;

  }
}

cl_program OclResourceServiceImpl::GetProgram(const std::string &name) const
{
  ProgramMapType::const_iterator it;
  it = m_ProgramStorage.find(name);

  if( it != m_ProgramStorage.end() )
  {
    return it->second;
  }

  mitkThrow() << "Requested OpenCL Program (" << name <<") not found.";
}

void OclResourceServiceImpl::InvalidateStorage()
{
  // do nothing if no context present
  if(m_Context == NULL)
    return;

  // query the map
  ProgramMapType::iterator it = m_ProgramStorage.begin();

  while(it != m_ProgramStorage.end() )
  {
    // query the program build status
    cl_build_status status;
    unsigned int query = clGetProgramBuildInfo( it->second, m_Devices[0], CL_PROGRAM_BUILD_STATUS, sizeof(cl_int), &status, NULL );

    MITK_DEBUG << "Quering status for " << it->first << std::endl;

    // remove program if no succesfull build
    // we need to pay attention to the increment of the iterator when erasing current element
    if( status != CL_BUILD_SUCCESS )
    {
      MITK_DEBUG << " +-- Build failed " << std::endl;
      m_ProgramStorage.erase( it++ );
    }
    else
    {
      ++it;
    }
  }
}

void OclResourceServiceImpl::RemoveProgram(const std::string& name)
{
  ProgramMapType::iterator it = m_ProgramStorage.find(name);

  if( it != m_ProgramStorage.end() )
  {
    m_ProgramStorage.erase(it);
  }
  else
  {
    MITK_WARN("OpenCL.ResourceService") << "Program name [" <<name <<"] passed for deletion not found.";
  }
}

unsigned int OclResourceServiceImpl::GetMaximumImageSize(unsigned int dimension, cl_mem_object_type _imagetype)
{
  if(m_Context==NULL)
    return 0;

  unsigned int retValue = 0;

  switch(dimension)
  {
  case 0:
    if ( _imagetype == CL_MEM_OBJECT_IMAGE2D)
      clGetDeviceInfo( m_Devices[0], CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof( cl_uint ), &retValue, NULL );
    else
      clGetDeviceInfo( m_Devices[0], CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof( cl_uint ), &retValue, NULL );

    break;
  case 1:
    if ( _imagetype == CL_MEM_OBJECT_IMAGE2D)
      clGetDeviceInfo( m_Devices[0], CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof( cl_uint ), &retValue, NULL );
    else
      clGetDeviceInfo( m_Devices[0], CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof( cl_uint ), &retValue, NULL );
    break;
  case 2:
    if ( _imagetype == CL_MEM_OBJECT_IMAGE3D)
      clGetDeviceInfo( m_Devices[0], CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof( cl_uint ), &retValue, NULL);
    break;
  default:
    MITK_WARN << "Could not recieve info. Desired dimension or object type does not exist. ";
    break;
  }

  return retValue;
}

void OclResourceServiceImpl::CreateContext()
{
  cl_int clErr = 0;
  size_t szParmDataBytes;
  cl_platform_id cpPlatform;
  cl_device_id m_cdDevice;

  try{
    clErr = oclGetPlatformID( &cpPlatform);
    CHECK_OCL_ERR( clErr );

    clErr = clGetDeviceIDs( cpPlatform, CL_DEVICE_TYPE_GPU, 1, &m_cdDevice, NULL);
    CHECK_OCL_ERR( clErr );

    this->m_Context = clCreateContext( 0, 1, &m_cdDevice, NULL, NULL, &clErr);
    CHECK_OCL_ERR( clErr );

    // get the info size
    clErr = clGetContextInfo(m_Context, CL_CONTEXT_DEVICES, 0,NULL, &szParmDataBytes );
    this->m_Devices = (cl_device_id*) malloc(szParmDataBytes);

    // get device info
    clErr = clGetContextInfo(m_Context, CL_CONTEXT_DEVICES, szParmDataBytes, m_Devices, NULL);
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
