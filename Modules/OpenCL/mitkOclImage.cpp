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

#include "mitkOclImage.h"
#include "mitkImageDataItem.h"
#include "mitkCommon.h"
#include "mitkLogMacros.h"

#include "mitkOclUtils.h"

#include <mitkImageReadAccessor.h>
#include <fstream>

mitk::OclImage::OclImage() : m_gpuImage(NULL), m_context(NULL), m_bufferSize(0), m_gpuModified(false), m_cpuModified(false),
  m_Image(NULL), m_dim(0), m_Dims(NULL), m_BpE(1), m_formatSupported(false)
{
}

mitk::OclImage::~OclImage()
{
  MITK_INFO << "OclImage Destructor";

  //release GMEM Image buffer
  if (m_gpuImage) clReleaseMemObject(m_gpuImage);
}


cl_mem mitk::OclImage::CreateGPUImage(unsigned int _wi, unsigned int _he, unsigned int _de,
                                      unsigned int _bpp)
{
  MITK_INFO << "CreateGPUImage call with: BPP=" << _bpp;

  this->m_Dims = new unsigned int[MAX_DIMS];

  m_Dims[0] = _wi;
  m_Dims[1] = _he;
  m_Dims[2] = _de;

  for (unsigned int i=3; i<MAX_DIMS; i++)
    m_Dims[i] = 1;

  m_bufferSize = _wi * _he * _de;

  m_BpE = _bpp;

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

  cl_context gpuContext = resources->GetContext();

  int clErr;
  m_gpuImage = clCreateBuffer( gpuContext, CL_MEM_READ_WRITE, m_bufferSize * m_BpE, NULL, &clErr);

  CHECK_OCL_ERR(clErr);

  return m_gpuImage;
}

void mitk::OclImage::InitializeByMitkImage(mitk::Image::Pointer _image)
{
  this->m_Image = _image;
  this->m_cpuModified = true;
  this->m_gpuModified = false;

  this->m_gpuImage = NULL;

  // compute the size of the GMEM buffer
  this->m_dim = this->m_Image->GetDimension();
  this->m_Dims = this->m_Image->GetDimensions();
  MITK_INFO << "Image: " << this->m_Dims[0] <<"x"<< this->m_Dims[1] <<"x"<< this->m_Dims[2];

  // get the dimensions
  this->m_bufferSize = 1;
  for (unsigned int i=0; i<this->m_dim; i++)
  {
    this->m_bufferSize *= this->m_Dims[i];
  }

  // multiply by sizeof(PixelType)
  this->m_BpE  = ( this->m_Image->GetPixelType().GetBpe() / 8);
}

bool mitk::OclImage::IsModified(int _type)
{
  if (_type) return m_cpuModified;
  else return m_gpuModified;
}

void mitk::OclImage::Modified(int _type)
{
  // defines... GPU: 0, CPU: 1
  m_cpuModified = _type;
  m_gpuModified = !_type;
}

int mitk::OclImage::TransferDataToGPU(cl_command_queue gpuComQueue)
{
  cl_int clErr = 0;

  // check whether an image present
  if (!m_Image->IsInitialized()){
    MITK_ERROR("ocl.Image") << "(mitk) Image not initialized!\n";
    return -1;
  }

  // there is a need for copy only if RAM-Data newer then GMEM data
  if (m_cpuModified)
  {
    //check the buffer
    if(m_gpuImage == NULL)
    {
      clErr = this->AllocateGPUImage();
    }

    if (m_Image->IsInitialized() &&
        (clErr == CL_SUCCESS))
    {
      const size_t origin[3] = {0, 0, 0};
      const size_t region[3] = {m_Dims[0], m_Dims[1], m_Dims[2]};

      if( this->m_formatSupported )
      {
        mitk::ImageReadAccessor accessor(m_Image);
        clErr = clEnqueueWriteImage( gpuComQueue, m_gpuImage, CL_TRUE, origin, region, 0, 0, accessor.GetData(), 0, NULL, NULL);
      }
      else
      {
        MITK_ERROR << "Selected image format currently not supported...";
      }

      CHECK_OCL_ERR(clErr);
    }
    m_gpuModified = true;
  }

  return clErr;
}

cl_int mitk::OclImage::AllocateGPUImage()
{
  cl_int clErr = 0;

  us::ServiceReference<OclResourceService> ref = GetModuleContext()->GetServiceReference<OclResourceService>();
  OclResourceService* resources = GetModuleContext()->GetService<OclResourceService>(ref);

  cl_context gpuContext = resources->GetContext();

  // initialize both proposed and supported format variables to same value
  this->m_proposedFormat = this->ConvertPixelTypeToOCLFormat();
  this->m_supportedFormat = this->m_proposedFormat;

  // test the current format for HW support
  this->m_formatSupported = resources->GetIsFormatSupported( &(this->m_supportedFormat) );

  // create an transfer kernel object in case the proposed format is not supported
  if( !(this->m_formatSupported) )
  {
    mitkThrowException(mitk::ImageTypeIsNotSupportedByGPU) << "Original format not supported on the installed graphics card.";
  }

  // create new buffer
  if( this->m_dim > 2)
  {
    //Create a 3D Image
    m_gpuImage = clCreateImage3D(gpuContext, CL_MEM_READ_ONLY, &m_supportedFormat, *(m_Dims), *(m_Dims+1), *(m_Dims+2), 0, 0, NULL, &clErr);
  }
  else
  {
    //Create a 2D Image
    m_gpuImage = clCreateImage2D(gpuContext, CL_MEM_READ_ONLY, &m_supportedFormat,  *(m_Dims), *(m_Dims+1), 0, NULL, &clErr);
  }
  CHECK_OCL_ERR(clErr);

  return clErr;
}

cl_mem mitk::OclImage::GetGPUImage(cl_command_queue gpuComQueue)
{
  // clGetMemObjectInfo()
  cl_mem_object_type memInfo;
  cl_int clErr = 0;

  // query image object info only if already initialized
  if( this->m_gpuImage )
  {
    clErr = clGetMemObjectInfo(this->m_gpuImage, CL_MEM_TYPE, sizeof(cl_mem_object_type), &memInfo, NULL );
    CHECK_OCL_ERR(clErr);
  }

  MITK_INFO << "Querying info for object, recieving: " << memInfo;

  // test if m_gpuImage CL_MEM_IMAGE_2/3D
  // if not, copy buffer to image
  if (memInfo == CL_MEM_OBJECT_BUFFER)
  {
    MITK_WARN << " Passed oclImage is a buffer-object, creating image";

    //hold a copy of the buffer gmem pointer
    cl_mem tempBuffer = this->m_gpuImage;

    const size_t origin[3] = {0, 0, 0};
    size_t region[3] = {this->m_Dims[0], this->m_Dims[1], 1};

    clErr = this->AllocateGPUImage();

    this->m_dim = 3;

    //copy last data to the image data
    clErr = clEnqueueCopyBufferToImage( gpuComQueue, tempBuffer, m_gpuImage, 0, origin, region, 0, NULL, NULL);
    CHECK_OCL_ERR(clErr);

    //release pointer
    clReleaseMemObject(tempBuffer);
  }
  return m_gpuImage;
}

void mitk::OclImage::SetPixelType(const cl_image_format *_image)
{
  this->m_proposedFormat.image_channel_data_type = _image->image_channel_data_type;
  this->m_proposedFormat.image_channel_order = _image->image_channel_order;
}

void* mitk::OclImage::TransferDataToCPU(cl_command_queue gpuComQueue)
{
  cl_int clErr = 0;

  // if image created on GPU, needs to create mitk::Image
  if( m_Image.IsNull() ){
    MITK_INFO << "Image not initialized, creating new one.";
    m_Image = mitk::Image::New();
  }

  // check buffersize/image size
  char* data = new char[m_bufferSize * m_BpE];

  // debug info
  oclPrintMemObjectInfo( m_gpuImage );

  clErr = clEnqueueReadBuffer( gpuComQueue, m_gpuImage, CL_FALSE, 0, m_bufferSize * m_BpE, data ,0, NULL, NULL);
  CHECK_OCL_ERR(clErr);

  clFlush( gpuComQueue );
  // the cpu data is same as gpu
  this->m_gpuModified = false;

  return (void*) data;
}

cl_image_format mitk::OclImage::ConvertPixelTypeToOCLFormat()
{
  cl_image_format texFormat;
  //single channel Gray-Valued Images
  texFormat.image_channel_order = CL_R;

  MITK_INFO << "Class own value: " << this->m_BpE;

  switch ( this->m_BpE )
  {
  case 1:
    texFormat.image_channel_data_type = CL_UNSIGNED_INT8;
    MITK_INFO<< "PixelType: UCHAR => CLFormat: [CL_UNORM_INT8, CL_R]";
    break;
  case 2:
    texFormat.image_channel_data_type = CL_SIGNED_INT16;
    // texFormat.image_channel_order = CL_R;
    MITK_INFO<< "PixelType: SHORT => CLFormat: [CL_SIGNED_INT16, CL_R]";
    break;
  case 4:
    texFormat.image_channel_data_type = CL_FLOAT;
    MITK_INFO<< "Choosing CL_FLOAT";
    break;
  default:
    texFormat.image_channel_data_type = CL_UNORM_INT8;
    texFormat.image_channel_order = CL_RG;
    MITK_INFO<< "Choosing (default) short: 2-Channel UCHAR";
    break;
  }

  return texFormat;
}

int mitk::OclImage::GetDimension(int idx) const
{
  if (this->m_dim > idx)
  {
    return m_Dims[idx];
  }
  else
  {
    MITK_WARN << "Trying to access non-existing dimension.";
    return 1;
  }
}

void mitk::OclImage::SetDimensions(unsigned int* Dims)
{
  m_Dims = Dims;
}

void mitk::OclImage::SetDimension(unsigned short dim)
{
  m_dim = dim;
}

float mitk::OclImage::GetSpacing(int idx)
{
  if (this->m_dim > idx)
  {
    const mitk::Vector3D imSpacing = m_Image->GetSlicedGeometry()->GetSpacing();

    return imSpacing[idx];
  }
  else
  {
    MITK_WARN << "Trying to access non-existing dimension.";
    return 1;
  }
}

void mitk::OclImage::InitializeMITKImage()
{
  this->m_Image = mitk::Image::New();
}

void mitk::OclImage::GetOffset(float* _imOffset) const
{
  itk::Vector<float, 3> result2;
  result2.Fill(0.0f);

  result2 = this->m_Image->GetGeometry()->GetIndexToWorldTransform()->GetOffset();

  _imOffset[0] = result2[0];
  _imOffset[1] = result2[1];
  _imOffset[2] = result2[2];
}
