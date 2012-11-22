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

//mitk
#include <mitkCommon.h>
#include <mitkLogMacros.h>

//itk
#include <itkLightObject.h>
#include <itkObjectFactory.h>

//ocl
#include "mitkOclUtils.h"
#include <MitkOclExports.h>

#define MAX_FORMATS 10
#define MAX_DATA_TYPES 12

namespace mitk
{

/**
      @class oclImageFormats
      @brief A class handling the (un)supported OpenCL Image formats.

      This class checks whether a format is supported or not and acts accordingly.
      */
class MitkOcl_EXPORT OclImageFormats : public itk::LightObject
{
public:
    mitkClassMacro(OclImageFormats, itk::LightObject);
    itkNewMacro(Self);

    /** @brief Checks if format supported.
      @param format The image format to be checked for support.
      @return True if the parameter is supported.
      */
    bool IsFormatSupported(cl_image_format* format);

    /** @brief Finds one supported image format similar to the given format.

      In case the input format is supported, the output format will contain the same value.

      @param inputformat The (unsupported) image format
      @param outputformat A supported image format

      @return True if a supported format was changed.
      */
    bool GetNearestSupported(cl_image_format* inputformat, cl_image_format* outputformat);

    /**
     * @brief SetGPUContext Set the GPU context. Must be called before using this class!
     * @param context GPU context in cl format.
     */
    void SetGPUContext( cl_context context );


protected:
    /** @brief Get and store all available infos
      * @throw Throws an mitk::Exception if the GPU context was not defined.
      */
    void CollectAvailableFormats();

    /**
 * @brief OclImageFormats Constructor
 */
    OclImageFormats();

    /** @brief Destructor (default) */
    virtual ~OclImageFormats();

private:

    /**   @brief Store the given format into the table
   *            Todo: what table? Where? Why is it called sort?
        @param formats A pointer to an array of image formats
                Todo: What formats?
        @param count Size of the formats array (i.e. how many formats are to be sorted).
        @param val an intern value to distinguish between read/write/readwrite type
                Todo: what is val?
        @param dim Specifies the target image dimension (default IMAGE_2D).
    */
    void SortFormats( cl_image_format* formats, cl_uint count, int val=1 , int dims = 2);

    /** @brief Get the byte offset to the image support fields.
                Todo: What means support field? What is this offset used for? In what unit is it returned?*/
    unsigned int GetOffset( cl_image_format format );

    /** @brief Print all supported formats (in a table) for both 2D and 3D IMAGE */
    void PrintSelf();

    /** An array to hold the information about IMAGE_2D supported formats. */
    unsigned char* m_Image2DSupport;

    /** An array to hold the information about IMAGE_3D supported formats. */
    unsigned char* m_Image3DSupport;

    /** the OpenCL context */
    cl_context m_GpuContext;
};

}

#endif // __mitkOclImageFormats_h
