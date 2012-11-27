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

#include "mitkOclUtils.h"
#include "mitkLogMacros.h"
#include <cstdio>
#include <cstring>


unsigned int iDivUp(unsigned int dividend, unsigned int divisor){
  return (dividend % divisor == 0) ? (dividend / divisor) : (dividend / divisor + 1);
}

cl_int oclGetPlatformID(cl_platform_id* selectedPlatform)
{
  cl_uint num_platforms = 0;
  cl_platform_id* clPlatformIDs;
  cl_int ciErrNum = 0;

  ciErrNum = clGetPlatformIDs( 0, NULL, &num_platforms);
  if ( ciErrNum != CL_SUCCESS)
  {
    MITK_ERROR<<" Error " << ciErrNum << " in clGetPlatformIDs() \n";
    throw std::bad_exception();
  }
  else
  {
    clPlatformIDs = new cl_platform_id[num_platforms];

    ciErrNum = clGetPlatformIDs( num_platforms, clPlatformIDs, NULL);
    if(ciErrNum == CL_SUCCESS)
    {
      *selectedPlatform = clPlatformIDs[0];
    }
  }

  return CL_SUCCESS;

}

void oclPrintMemObjectInfo(cl_mem memobj)
{
  cl_int clErr = 0;

  MITK_INFO << "Examining cl_mem object: " << memobj
            << "\n------------------\n";

  // CL_MEM_TYPE
  cl_mem_object_type objtype;
  clErr = clGetMemObjectInfo( memobj, CL_MEM_TYPE, sizeof(cl_mem_object_type),&objtype, NULL);
  CHECK_OCL_ERR( clErr );

  switch(objtype)
  {
  case CL_MEM_OBJECT_BUFFER:
    MITK_INFO << "CL_MEM_TYPE \t" << "BUFFER_OBJ" << "\n";
    break;
  case CL_MEM_OBJECT_IMAGE2D:
    MITK_INFO << "CL_MEM_TYPE \t" << "2D IMAGE" << "\n";
    break;
  case CL_MEM_OBJECT_IMAGE3D:
    MITK_INFO << "CL_MEM_TYPE \t" << "3D IMAGE" << "\n";
    break;
  default:
    MITK_INFO << "CL_MEM_TYPE \t" << "[could not resolve]" << "\n";
    break;
  }

  // CL_MEM_FLAGS
  cl_mem_flags flags;
  clErr = clGetMemObjectInfo( memobj, CL_MEM_FLAGS, sizeof(cl_mem_flags),&flags, NULL);
  CHECK_OCL_ERR( clErr );

  switch(flags)
  {
  case CL_MEM_READ_ONLY:
    MITK_INFO << "CL_MEM_FLAGS \t" << "CL_MEM_READ_ONLY" << "\n";
    break;
  case CL_MEM_WRITE_ONLY:
    MITK_INFO << "CL_MEM_FLAGS \t" << "CL_MEM_WRITE_ONLY" << "\n";
    break;
  case CL_MEM_READ_WRITE:
    MITK_INFO << "CL_MEM_FLAGS \t" << "CL_MEM_READ_WRITE" << "\n";
    break;
  default:
    MITK_INFO << "CL_MEM_FLAGS \t" << "not resolved, " << flags << "\n";
    break;
  }

  // get CL_MEM_SIZE
  size_t memsize;
  clErr = clGetMemObjectInfo( memobj, CL_MEM_SIZE, sizeof(memsize),&memsize, NULL);
  CHECK_OCL_ERR( clErr );

  MITK_INFO << "CL_MEM_SIZE \t" << memsize << "\n";

  // get CL_MEM_HOST_PTR
  float *hostptr;
  clErr = clGetMemObjectInfo( memobj, CL_MEM_HOST_PTR, sizeof(void*), (void*) &hostptr, NULL);
  CHECK_OCL_ERR( clErr );

  MITK_INFO << "CL_MEM_HOST_PTR \t" << hostptr << "\n";

  // get CL_CONTEXT
  cl_context gpuctxt;
  clErr = clGetMemObjectInfo( memobj, CL_MEM_CONTEXT, sizeof(cl_context), &gpuctxt, NULL);
  CHECK_OCL_ERR( clErr );

  MITK_INFO << "CL_CONTEXT \t\t" << gpuctxt << "\n";

  // get CL_MEM_REFERENCE_COUNT
  cl_uint refs;
  clErr = clGetMemObjectInfo( memobj, CL_MEM_REFERENCE_COUNT, sizeof(cl_uint), &refs, NULL);
  CHECK_OCL_ERR(clErr);

  MITK_INFO << "CL_REF_COUNT \t" << refs << "\n";

  MITK_INFO << "================== \n" << std::endl;
}


char* oclLoadProgramSource( const char* srcFilename, const char* srcPreamble, size_t* srcLength)
{
  FILE* pFileStream = NULL;
  size_t szSourceLength;

  MITK_INFO<< "Loading GPU SourceCode from " << srcFilename <<" \n";

#ifdef _WIN32
  if(fopen_s(&pFileStream, srcFilename, "rb") != 0)
  {
    return NULL;
    printf("Failed to open file \n");

  }
#else           // Linux version
  pFileStream = fopen(srcFilename, "rb");
  if(pFileStream == 0)
  {
    return NULL;
  }
#endif

  size_t szPreambleLength = strlen(srcPreamble);

  // get the length of the source code
  fseek(pFileStream, 0, SEEK_END);
  szSourceLength = ftell(pFileStream);
  fseek(pFileStream, 0, SEEK_SET);

  // allocate a buffer for the source code string and read it in
  char* cSourceString = (char *)malloc(szSourceLength + szPreambleLength + 1);
  memcpy(cSourceString, srcPreamble, szPreambleLength);
  if (fread((cSourceString) + szPreambleLength, szSourceLength, 1, pFileStream) != 1)
  {
    fclose(pFileStream);
    free(cSourceString);
    return 0;
  }

  // close the file and return the total length of the combined (preamble + source) string
  fclose(pFileStream);
  if(srcLength != 0)
  {
    *srcLength = szSourceLength + szPreambleLength;
  }
  cSourceString[szSourceLength + szPreambleLength] = '\0';

  return cSourceString;
}

void oclPrintDeviceInfo(cl_device_id device)
{
  char device_string[1024];

  clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(device_string), &device_string, NULL);
  MITK_INFO("ocl.log")<< " Device : " << device_string;

  // CL_DEVICE_INFO
  cl_device_type type;
  clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(type), &type, NULL);
  if( type & CL_DEVICE_TYPE_CPU )
    MITK_INFO("ocl.log")<<"  CL_DEVICE_TYPE: CL_DEVICE_TYPE_CPU";
  if( type & CL_DEVICE_TYPE_GPU )
    MITK_INFO("ocl.log")<<"  CL_DEVICE_TYPE: CL_DEVICE_TYPE_GPU";
  if( type & CL_DEVICE_TYPE_ACCELERATOR )
    MITK_INFO("ocl.log")<<"  CL_DEVICE_TYPE: CL_DEVICE_TYPE_ACCELERATOR";
  if( type & CL_DEVICE_TYPE_DEFAULT )
    MITK_INFO("ocl.log")<<"  CL_DEVICE_TYPE: CL_DEVICE_TYPE_DEFAULT";

  // CL_DEVICE_MAX_COMPUTE_UNITS
  cl_uint compute_units;
  clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(compute_units), &compute_units, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_MAX_COMPUTE_UNITS:" << compute_units;

  // CL_DEVICE_MAX_WORK_GROUP_SIZE
  size_t workitem_size[3];
  clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(workitem_size), &workitem_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_MAX_WORK_ITEM_SIZES:\t"<< workitem_size[0]<< workitem_size[1]<< workitem_size[2];

  // CL_DEVICE_MAX_WORK_GROUP_SIZE
  size_t workgroup_size;
  clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(workgroup_size), &workgroup_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_MAX_WORK_GROUP_SIZE:" << workgroup_size;

  // CL_DEVICE_MAX_CLOCK_FREQUENCY
  cl_uint clock_frequency;
  clGetDeviceInfo(device, CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(clock_frequency), &clock_frequency, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_MAX_CLOCK_FREQUENCY:"<< clock_frequency / 1000;

  // CL_DEVICE_IMAGE_SUPPORT
  cl_bool image_support;
  clGetDeviceInfo(device, CL_DEVICE_IMAGE_SUPPORT, sizeof(image_support), &image_support, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_IMAGE_SUPPORT:\t" << image_support;

  // CL_DEVICE_GLOBAL_MEM_SIZE
  cl_ulong mem_size;
  clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_GLOBAL_MEM_SIZE:\t\t"<<(unsigned int)(mem_size / (1024 * 1024))<<"Mbytes";

  // CL_DEVICE_LOCAL_MEM_SIZE
  clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(mem_size), &mem_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_LOCAL_MEM_SIZE:\t\t"<<  (unsigned int)(mem_size / (1024)) <<"KByte\n";

  //check for image support properties
  clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_WIDTH, sizeof(workgroup_size), &workgroup_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_IMAGE2D_MAX_WIDTH:\t" << workgroup_size;

  clGetDeviceInfo(device, CL_DEVICE_IMAGE2D_MAX_HEIGHT, sizeof(workgroup_size), &workgroup_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_IMAGE2D_MAX_HEIGHT:\t" << workgroup_size;

  clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_WIDTH, sizeof(workgroup_size), &workgroup_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_IMAGE3D_MAX_WIDTH:\t" << workgroup_size;

  clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_HEIGHT, sizeof(workgroup_size), &workgroup_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_IMAGE3D_MAX_HEIGHT:\t" << workgroup_size;

  clGetDeviceInfo(device, CL_DEVICE_IMAGE3D_MAX_DEPTH, sizeof(workgroup_size), &workgroup_size, NULL);
  MITK_INFO("ocl.log")<<"  CL_DEVICE_IMAGE3D_MAX_DEPTH:\t" << workgroup_size;


  // CL_DEVICE_QUEUE_PROPERTIES
  cl_command_queue_properties queue_properties;
  clGetDeviceInfo(device, CL_DEVICE_QUEUE_PROPERTIES, sizeof(queue_properties), &queue_properties, NULL);
  if( queue_properties & CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE )
    MITK_INFO("ocl.log")<<"  CL_DEVICE_QUEUE_PROPERTIES:\t\t"<< "CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE";
  if( queue_properties & CL_QUEUE_PROFILING_ENABLE )
    MITK_INFO("ocl.log")<<"  CL_DEVICE_QUEUE_PROPERTIES:\t\t"<< "CL_QUEUE_PROFILING_ENABLE";

}

std::string GetOclErrorAsString( int _clErr )
{
  std::string returnString("CL_SUCCESS\n");

  switch(_clErr)
  {
  case CL_DEVICE_NOT_FOUND:
    returnString =  "CL_DEVICE_NOT_FOUND\n";
    break;
  case CL_DEVICE_NOT_AVAILABLE:
    returnString =  "CL_DEVICE_NOT_AVAILABLE\n";
    break;
    /*case CL_DEVICE_COMPILER_NOT_AVAILABLE:
    returnString =  "CL_DEVICE_COMPILER_NOT_AVAILABLE\n";
    break;  */
  case CL_MEM_OBJECT_ALLOCATION_FAILURE :
    returnString =  "CL_MEM_OBJECT_ALLOCATION_FAILURE\n";
    break;
  case CL_OUT_OF_RESOURCES:
    returnString =  "CL_OUT_OF_RESOURCES\n";
    break;
  case CL_OUT_OF_HOST_MEMORY:
    returnString =  "CL_OUT_OF_HOST_MEMORY\n";
    break;
  case CL_PROFILING_INFO_NOT_AVAILABLE:
    returnString =  "CL_PROFILING_INFO_NOT_AVAILABLE\n";
    break;
  case CL_MEM_COPY_OVERLAP:
    returnString =  "CL_MEM_COPY_OVERLAP\n";
    break;
  case CL_IMAGE_FORMAT_MISMATCH:
    returnString =  "CL_IMAGE_FORMAT_MISMATCH\n";
    break;
  case CL_IMAGE_FORMAT_NOT_SUPPORTED:
    returnString =  "CL_IMAGE_FORMAT_NOT_SUPPORTED\n";
    break;
  case CL_BUILD_PROGRAM_FAILURE:
    returnString =  "CL_BUILD_PROGRAM_FAILURE\n";
    break;
  case CL_MAP_FAILURE:
    returnString =  "CL_MAP_FAILURE\n";
    break;
  case CL_INVALID_VALUE:
    returnString =  "CL_INVALID_VALUE\n";
    break;
  case CL_INVALID_DEVICE_TYPE:
    returnString =  "CL_INVALID_DEVICE_TYPE\n";
    break;
  case CL_INVALID_PLATFORM:
    returnString =  "CL_INVALID_PLATFORM\n";
    break;
  case CL_INVALID_DEVICE:
    returnString =  "CL_INVALID_DEVICE\n";
    break;
  case CL_INVALID_CONTEXT :
    returnString =  "CL_INVALID_CONTEXT\n";
    break;
  case CL_INVALID_QUEUE_PROPERTIES:
    returnString =  "CL_INVALID_QUEUE_PROPERTIES\n";
    break;
  case CL_INVALID_COMMAND_QUEUE:
    returnString =  "CL_INVALID_COMMAND_QUEUE\n";
    break;
  case CL_INVALID_HOST_PTR:
    returnString =  "CL_INVALID_HOST_PTR\n";
    break;
  case CL_INVALID_MEM_OBJECT:
    returnString =  "CL_INVALID_MEM_OBJECT\n";
    break;
  case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
    returnString =  "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n";
    break;
  case CL_INVALID_IMAGE_SIZE:
    returnString =  "CL_INVALID_IMAGE_SIZE\n";
    break;
  case CL_INVALID_SAMPLER :
    returnString =  "CL_INVALID_SAMPLER\n";
    break;
  case CL_INVALID_BINARY:
    returnString =  "CL_INVALID_BINARY\n";
    break;
  case CL_INVALID_BUILD_OPTIONS:
    returnString =  "CL_INVALID_BUILD_OPTIONS\n";
    break;
  case CL_INVALID_PROGRAM:
    returnString =  "CL_INVALID_PROGRAM\n";
    break;
  case CL_INVALID_PROGRAM_EXECUTABLE:
    returnString =  "CL_INVALID_PROGRAM_EXECUTABLE\n";
    break;
  case CL_INVALID_KERNEL_NAME:
    returnString =  "CL_INVALID_KERNEL_NAME\n";
    break;
  case CL_INVALID_KERNEL_DEFINITION:
    returnString =  "CL_INVALID_KERNEL_DEFINITION\n";
    break;
  case CL_INVALID_KERNEL :
    returnString =  "CL_INVALID_KERNEL\n";
    break;
  case CL_INVALID_ARG_INDEX :
    returnString =  "CL_INVALID_ARG_INDEX\n";
    break;
  case CL_INVALID_ARG_VALUE :
    returnString =  "CL_INVALID_ARG_VALUE\n";
    break;
  case CL_INVALID_ARG_SIZE :
    returnString =  "CL_INVALID_ARG_SIZE\n";
    break;
  case CL_INVALID_KERNEL_ARGS :
    returnString =  "CL_INVALID_KERNEL_ARGS\n";
    break;
  case CL_INVALID_WORK_DIMENSION:
    returnString =  "CL_INVALID_WORK_DIMENSION\n";
    break;
  case CL_INVALID_WORK_GROUP_SIZE:
    returnString =  "CL_INVALID_WORK_GROUP_SIZE\n";
    break;
  case CL_INVALID_WORK_ITEM_SIZE:
    returnString =  "CL_INVALID_WORK_ITEM_SIZE\n";
    break;
  case CL_INVALID_GLOBAL_OFFSET:
    returnString =  "CL_INVALID_GLOBAL_OFFSET\n";
    break;
  case CL_INVALID_EVENT_WAIT_LIST:
    returnString =  "CL_INVALID_EVENT_WAIT_LIST\n";
    break;
  case CL_INVALID_EVENT:
    returnString =  "CL_INVALID_EVENT\n";
    break;
  case CL_INVALID_OPERATION:
    returnString =  "CL_INVALID_OPERATION\n";
    break;
  case CL_INVALID_GL_OBJECT:
    returnString =  "CL_INVALID_GL_OBJECT\n";
    break;
  case CL_INVALID_BUFFER_SIZE :
    returnString =  "CL_INVALID_BUFFER_SIZE\n";
    break;
  case CL_INVALID_MIP_LEVEL :
    returnString =  "CL_INVALID_MIP_LEVEL\n";
    break;
  default:
    break;
  }

  return returnString;
}

void GetOclError(int _clErr)
{
  if(_clErr == CL_SUCCESS )
    MITK_WARN << "Called GetOclErr() with no error value: [CL_SUCCESS]";
  else
    MITK_ERROR << GetOclErrorAsString(_clErr);
}

bool oclCheckError(int _err, const char* filepath, int lineno)
{
  if (_err)
  {
    MITK_ERROR<< "OpenCL Error at " << filepath <<":"<< lineno;
    GetOclError(_err);

    return 0;
  }

  return 1;
}

void GetSupportedImageFormats(cl_context _context, cl_mem_object_type _type)
{
  const unsigned int entries = 500;
  cl_image_format* formats = new cl_image_format[entries];

  cl_uint _written = 0;

  // OpenCL constant to catch error IDs
  cl_int ciErr1;
  // Get list of supported image formats for READ_ONLY access
  ciErr1 = clGetSupportedImageFormats( _context, CL_MEM_READ_ONLY, _type, entries, formats, &_written);
  CHECK_OCL_ERR(ciErr1);

  MITK_INFO << "Supported Image Formats, Image: CL_MEM_READ_ONLY \n";

  for (unsigned int i=0; i<_written; i++)
  {
    MITK_INFO<< "ChannelType: " << GetImageTypeAsString(formats[i].image_channel_data_type) << "| ChannelOrder: "<< GetImageTypeAsString(formats[i].image_channel_order) <<"\n";
  }

  _written = 0;

  // Get list of supported image formats for READ_WRITE access
  ciErr1 = clGetSupportedImageFormats( _context, CL_MEM_READ_WRITE, _type, entries, formats, &_written);
  CHECK_OCL_ERR(ciErr1);

  MITK_INFO << "Supported Image Formats, Image: CL_MEM_READ_WRITE (found: " << _written <<") \n";

  for (unsigned int i=0; i<_written; i++)
  {
    MITK_INFO<< "ChannelType: " << GetImageTypeAsString(formats[i].image_channel_data_type) << "| ChannelOrder: "<< GetImageTypeAsString(formats[i].image_channel_order) <<"\n";
  }

  _written = 0;

  // Get list of supported image formats for WRITE_ONLY access
  ciErr1 = clGetSupportedImageFormats( _context, CL_MEM_WRITE_ONLY, _type, entries, formats, &_written);
  CHECK_OCL_ERR(ciErr1);

  MITK_INFO << "Supported Image Formats, Image: CL_MEM_WRITE_ONLY (found: " << _written <<") \n";

  for (unsigned int i=0; i<_written; i++)
  {
    MITK_INFO<< "ChannelType: " << GetImageTypeAsString(formats[i].image_channel_data_type) << "| ChannelOrder: "<< GetImageTypeAsString(formats[i].image_channel_order) <<"\n";
  }
}

std::string GetImageTypeAsString( const unsigned int _in)
{
  switch(_in)
  {
  case CL_R:
    return "CL_R ";
    break;
  case CL_A:
    return "CL_A ";
    break;
  case CL_RG:
    return "CL_RG ";
    break;
  case CL_RA:
    return "CL_RA ";
    break;
  case CL_RGB:
    return "CL_RGB ";
    break;
  case CL_RGBA:
    return "CL_RGBA ";
    break;
  case CL_BGRA:
    return "CL_BGRA ";
    break;
  case CL_ARGB:
    return "CL_ARGB ";
    break;
  case CL_INTENSITY:
    return "CL_INTENSITY ";
    break;
  case CL_LUMINANCE:
    return "CL_LUMINANCE ";
    break;
  case CL_SNORM_INT8:
    return "CL_SNORM_INT8 ";
    break;
  case CL_SNORM_INT16:
    return "CL_SNORM_INT16 ";
    break;
  case CL_UNORM_INT8:
    return "CL_UNORM_INT8 ";
    break;
  case CL_UNORM_INT16:
    return "CL_UNORM_INT16 ";
    break;
  case CL_UNORM_SHORT_565:
    return "CL_UNORM_SHORT_565 ";
    break;
  case CL_UNORM_SHORT_555:
    return "CL_UNORM_SHORT_555 ";
    break;
  case CL_UNORM_INT_101010:
    return "CL_UNORM_INT_101010 ";
    break;
  case CL_SIGNED_INT8:
    return "CL_SIGNED_INT8 ";
    break;
  case CL_SIGNED_INT16:
    return "CL_SIGNED_INT16 ";
    break;
  case CL_SIGNED_INT32:
    return "CL_SIGNED_INT32 ";
    break;
  case CL_UNSIGNED_INT8:
    return "CL_UNSIGNED_INT8 ";
    break;
  case CL_UNSIGNED_INT16:
    return "CL_UNSIGNED_INT16 ";
    break;
  case CL_UNSIGNED_INT32:
    return "CL_UNSIGNED_INT32 ";
    break;
  case CL_HALF_FLOAT:
    return "CL_HALF_FLOAT ";
    break;
  case CL_FLOAT:
    return "CL_FLOAT ";
    break;
  default:
    return "--";
    break;
  }
}



void oclLogBinary(cl_program clProg, cl_device_id clDev)
{
  // Grab the number of devices associated with the program
  cl_uint num_devices;
  clGetProgramInfo(clProg, CL_PROGRAM_NUM_DEVICES, sizeof(cl_uint), &num_devices, NULL);

  // Grab the device ids
  cl_device_id* devices = (cl_device_id*) malloc(num_devices * sizeof(cl_device_id));
  clGetProgramInfo(clProg, CL_PROGRAM_DEVICES, num_devices * sizeof(cl_device_id), devices, 0);

  // Grab the sizes of the binaries
  size_t* binary_sizes = (size_t*)malloc(num_devices * sizeof(size_t));
  clGetProgramInfo(clProg, CL_PROGRAM_BINARY_SIZES, num_devices * sizeof(size_t), binary_sizes, NULL);

  // Now get the binaries
  char** ptx_code = (char**)malloc(num_devices * sizeof(char*));
  for( unsigned int i=0; i<num_devices; ++i)
  {
    ptx_code[i] = (char*)malloc(binary_sizes[i]);
  }
  clGetProgramInfo(clProg, CL_PROGRAM_BINARIES, 0, ptx_code, NULL);

  // Find the index of the device of interest
  unsigned int idx = 0;
  while((idx < num_devices) && (devices[idx] != clDev))
  {
    ++idx;
  }

  // If the index is associated, log the result
  if( idx < num_devices )
  {
    MITK_INFO<< "\n ---------------- \n Program Binary: \n -----------------------\n";
    MITK_INFO<< ptx_code[idx];
  }

  free( devices );
  free( binary_sizes );
  for(unsigned int i=0; i<num_devices; ++i)
  {
    free(ptx_code[i]);
  }
  free( ptx_code );
}

void oclLogBuildInfo(cl_program clProg, cl_device_id clDev)
{
  char cBuildLog[10240];

  clGetProgramBuildInfo(clProg, clDev, CL_PROGRAM_BUILD_LOG, sizeof(cBuildLog), cBuildLog, NULL);
  MITK_INFO<< "\n Program Build Log: \n -----------------------\n";
  MITK_INFO<< cBuildLog;
}
