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

#ifndef __mitkOclImage_h
#define __mitkOclImage_h

#define GPU_DATA 0
#define CPU_DATA 1

#define MAX_DIMS 20

#include <mitkImage.h>
#include "MitkOpenCLExports.h"

#include "mitkOclBaseData.h"
#include "mitkOclImageFormats.h"
#include "mitkOpenCLActivator.h"

#include <mitkException.h>


#define SHORT_IM mitk::MakeScalarPixelType<short>()
#define FLOAT_IM mitk::MakeScalarPixelType<float>()

namespace mitk {

/*!
 * \brief Class implementing the image format for GPU Image Processing
  *
  *  The class holds a pointer to the mitk::Image stored in RAM and performs an
  *  on-demand-copy to the graphics memory. It is the basic data structure for all
  *  mitk::oclImageToImageFilter classes
  *  @throw This class may throw an ImageTypeIsNotSupportedByGPU, if the image
  *  format is supported by the GPU.
  */
class MitkOpenCL_EXPORT OclImage : public OclBaseData
{
public:
  mitkClassMacro(OclImage, OclBaseData);
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /*!  \brief Copies the RAM-stored data to GMEM */
  virtual int TransferDataToGPU(cl_command_queue);

  /*! \brief Copies the in GMEM stored data to RAM */
  virtual void* TransferDataToCPU(cl_command_queue);

  /*! \brief Returns the pointer to the referenced mitk::Image */
  Image::Pointer GetMITKImage()
  {
    return m_Image;
  }

  /*! \brief Checks whether gpuImage is a valid clImage object

    when an oclImage gets created by an image to image filter, the output image is created
    by clCreateBuffer() because it is not in general possible to write to clImage directly
    */
  cl_mem GetGPUImage(cl_command_queue);

  /** Returns the pointer to the GPU buffer */
  cl_mem GetGPUBuffer()
  {
    return this->m_gpuImage;
  }

  /** Create the GPU buffer for image
      *
      */
  cl_mem CreateGPUImage(unsigned int, unsigned int, unsigned int, unsigned int);

  /** \brief Returns the status of the image buffer
     *
     * @param _type The flag to specify the buffer type ( GPU / CPU )
     */
  bool IsModified(int _type);

  /** \brief Set the modified flag for one of the buffers
     *
     * @param _type The flag to specify the buffer type ( GPU / CPU )
     */
  void Modified(int _type);

  /** @brief Initialize the internal variable of type mitk::Image.
      */
  void InitializeMITKImage();

  /** \brief Initialze the OclImage with an mitkImage. */
  void InitializeByMitkImage(mitk::Image::Pointer _image);

  /*! \brief returns the specified image dimension size */
  int GetDimension(int) const;

  /*! \brief returns the dimensionality of the image */
  int GetDimension() const
  {
    return this->m_dim;
  }

  /*! \brief returns the pointer to the array of image sizes */
  unsigned int* GetDimensions(){ return this->m_Dims; }

  /*! \brief returns the spacing of the image for specified dimension */
  float GetSpacing(int);

  /*! \brief Returns the image offset (needed for WorldToIndex Transform */
  void GetOffset(float*) const;

  /** @brief Set the pixel type for the image to be used
      */
  void SetPixelType(const cl_image_format*);

  short GetBytesPerPixel() const
  {
    return this->m_BpE;
  }

  /** @brief Get the currently used pixel type

      @returns OpenCL Image Format struct
      */
  const cl_image_format* GetPixelType() const
  {
    return &(this->m_proposedFormat);
  }

  /** @brief Set the image dimensions through an unsigned int array */
  void SetDimensions(unsigned int* Dims);

  /** @brief Set the dimensionality of the image */
  void SetDimension(unsigned short dim);

protected:
  /*! \brief Constructor */
  OclImage();

  /** @brief Destructor */
  virtual ~OclImage();

  /*! GMEM Image buffer */
  cl_mem m_gpuImage;

  /*! GPU Context the Image Buffer was created in, needed for access */
  cl_context m_context;

  /*! GMEM Buffer Size */
  unsigned int m_bufferSize;

private:

  cl_image_format ConvertPixelTypeToOCLFormat();

  bool m_gpuModified;
  bool m_cpuModified;

  /*! Reference to mitk::Image */
  Image::Pointer m_Image;

  unsigned short m_dim;

  unsigned int* m_Dims;

  unsigned short m_BpE;

  cl_int AllocateGPUImage();

  /** Bool flag to signalize if the proposed format is supported on currend HW.
      For value 'false', the transfer kernel has to be called to fit the data to
      the supported format */
  bool m_formatSupported;

  /** Supported format, defined using oclImageFormats */
  cl_image_format m_supportedFormat;

  /** Proposed format for image */
  cl_image_format m_proposedFormat;
};

/**
 * @brief The ImageTypeIsNotSupportedByGPU class specialized exception class for unsupported
 * image formats. If this exception is thrown, try other graphics device.
 */
class ImageTypeIsNotSupportedByGPU : public Exception
{
public:
  mitkExceptionClassMacro(ImageTypeIsNotSupportedByGPU,Exception)
};

}
#endif //__mitkOclImage_h
