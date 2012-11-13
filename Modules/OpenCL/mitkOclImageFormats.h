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

#ifndef __mitkOclImageFormats_h
#define __mitkOclImageFormats_h

#include "mitkCommon.h"
#include "mitkLogMacros.h"
#include "mitkOclUtils.h"

#include <MitkOclExports.h>

#define MAX_FORMATS 10
#define MAX_DATA_TYPES 12

namespace mitk
{

/**
      @class oclImageFormats
      @brief A class handling the (un)supported OpenCL Image formats.

      */
class MitkOcl_EXPORT OclImageFormats
{
public:
  /** @brief Constructor */
  OclImageFormats( cl_context ctxt );

  /** @brief Destructor (default) */
  virtual ~OclImageFormats();

  /** @brief Check if format supported

      @param fmt - the image format to be checked for support

      @return true if the parameter is supported
      */
  bool IsFormatSupported( cl_image_format *fmt);

  /** @brief Finds one supported image format nearest to the given format

      In case the format in is supported, the out format will contain the same value

      @param in the (unsupported) image format
      @param out a supported image format

      @return true if a supported format was changed
      */
  bool GetNearestSupported( cl_image_format *in, cl_image_format *out);


protected:
  /** @brief Get and store all available infos */
  void CollectAvailableFormats();

private:

  /** Store the given format into the table

        @param formats an pointer to an array of image formats
        @param count size of the formats array
        @param val an intern value to distinguish between read/write/readwrite type
        @param dim (default IMAGE_2D) specifiy the target image dimension
    */
  void SortFormats( cl_image_format* formats, cl_uint count, int val=1 , int dims = 2);

  /** @brief return the byte offset to the image support fields */
  unsigned int GetOffset( cl_image_format fmt);

  /** @brief Print all supported formats (in a table) for both 2D and 3D IMAGE */
  void PrintSelf();

  /** an array to hold the information about IMAGE_2D supported formats */
  unsigned char* m_Image2DSupport;

  /** an array to hold the information about IMAGE_3D supported formats */
  unsigned char* m_Image3DSupport;

  /** the OpenCL context */
  cl_context m_gpuContext;

};

}

#endif // __mitkOclImageFormats_h
