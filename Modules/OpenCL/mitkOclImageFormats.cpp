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

#include "mitkOclImageFormats.h"

mitk::OclImageFormats::OclImageFormats()
    :m_Image2DSupport( NULL ), m_Image3DSupport( NULL ),
    m_GpuContext( NULL )
{
    //todo: what happens here?
    const unsigned int matrixSize = MAX_FORMATS * MAX_DATA_TYPES;

    this->m_Image2DSupport = new unsigned char[matrixSize];
    this->m_Image3DSupport = new unsigned char[matrixSize];

    for( unsigned int i = 0; i<matrixSize; i++ )
    {
        this->m_Image2DSupport[i] = 0;
        this->m_Image3DSupport[i] = 0;
    }
}

mitk::OclImageFormats::~OclImageFormats()
{
}

void mitk::OclImageFormats::PrintSelf()
{
    std::stringstream outputstream;
    outputstream << "Values: Read-Write(1) ReadOnly(2) , WriteOnly(4) \n";
    outputstream << "ROWS: [CL_A, CL_R, CL_RA, CL_RG, CL_RGB, CL_RGBA, CL_ARGB, CL_BGRA, CL_LUM, CL_INT] \n";

    const unsigned int matrixSize = MAX_FORMATS * MAX_DATA_TYPES;

    for( unsigned int i = 0; i<matrixSize; i++ )
    {
        outputstream << (int) this->m_Image2DSupport[i] << ", \t";
        if( (i+1) % MAX_DATA_TYPES == 0 )
            outputstream << " \n";
    }
    outputstream << "========================== \n";

    for( unsigned int i = 0; i<matrixSize; i++ )
    {
        outputstream << (int) this->m_Image3DSupport[i] << ", \t";
        if( (i+1) % MAX_DATA_TYPES == 0 )
            outputstream << " \n";
    }
    MITK_INFO << outputstream.str();
}

unsigned int mitk::OclImageFormats::GetOffset(cl_image_format format)
{
    //todo: what happens here?
    unsigned int offset = 0;

    switch( format.image_channel_order )
    {
    case CL_A:
        break;
    case CL_R:
        offset += 1 * MAX_DATA_TYPES;
        break;
    case CL_RA:
        offset += 2 * MAX_DATA_TYPES;
        break;
    case CL_RG:
        offset += 3 * MAX_DATA_TYPES;
        break;
    case CL_RGB:
        offset += 4 * MAX_DATA_TYPES;
        break;
    case CL_RGBA:
        offset += 5 * MAX_DATA_TYPES;
        break;
    case CL_ARGB:
        offset += 6 * MAX_DATA_TYPES;
        break;
    case CL_BGRA:
        offset += 7 * MAX_DATA_TYPES;
        break;
    case CL_LUMINANCE:
        offset += 8 * MAX_DATA_TYPES;
        break;
    case CL_INTENSITY:
        offset += 9 * MAX_DATA_TYPES;
        break;
    }

    switch ( format.image_channel_data_type )
    {
    case CL_SNORM_INT8:
        break;
    case CL_SNORM_INT16:
        offset += 1;
        break;
    case CL_UNORM_INT8:
        offset += 2;
        break;
    case CL_UNORM_INT16:
        offset += 3;
        break;
    case CL_SIGNED_INT8:
        offset += 4;
        break;
    case CL_SIGNED_INT16:
        offset += 5;
        break;
    case CL_SIGNED_INT32:
        offset += 6;
        break;
    case CL_UNSIGNED_INT8:
        offset += 7;
        break;
    case CL_UNSIGNED_INT16:
        offset += 8;
        break;
    case CL_UNSIGNED_INT32:
        offset += 9;
        break;
    case CL_HALF_FLOAT:
        offset += 10;
        break;
    case CL_FLOAT:
        offset += 11;
        break;
    }

    return offset;
}

bool mitk::OclImageFormats::IsFormatSupported(cl_image_format* format)
{
    bool retVal = false;

    // FIXME needs finer subdivision...
    //todo: Comment above???
    if ( this->m_Image2DSupport[ GetOffset(*format)] > 4 )
        retVal = true;

    return retVal;
}

bool mitk::OclImageFormats::GetNearestSupported(cl_image_format *inputformat, cl_image_format *outputformat)
{
    bool returnValue = false;

    // init output format
    outputformat->image_channel_data_type = inputformat->image_channel_data_type;
    outputformat->image_channel_order = inputformat->image_channel_order;

    // the input format is supported, just copy the information into out
    if( this->IsFormatSupported(inputformat) )
    {
        returnValue = true;
    }
    else
    {
        // get the 'nearest' format
        // try RGBA first
        //todo: It seems like ONLY RGBA is considered to be near?!? Either code or docu should be adapted.
        cl_image_format test;
        test.image_channel_order = CL_RGBA;
        test.image_channel_data_type = inputformat->image_channel_data_type;

        if(this->IsFormatSupported( &test) )
        {
            outputformat->image_channel_order = CL_RGBA;
        }
    }
    return returnValue;
}

void mitk::OclImageFormats::SetGPUContext(cl_context context)
{
    this->m_GpuContext = context;
    //collect available formats can now be called
    this->CollectAvailableFormats();
}

void mitk::OclImageFormats::SortFormats(cl_image_format *formats, cl_uint count, int val, int dims)
{
    //todo what happens here?
    unsigned char *target = this->m_Image2DSupport;
    if (dims == 3)
    {
        target = this->m_Image3DSupport;
    }

    for( unsigned int i=0; i<count; i++)
    {
        // each supported format => +1
        target[ this->GetOffset( formats[i] ) ]+=val;
    }
}

void mitk::OclImageFormats::CollectAvailableFormats()
{
    if( this->m_GpuContext == NULL)
    {
        mitkThrow() << "No GPU context was set! Use SetGPUContext() before calling this method!";
    }
    //todo what happens here?
    const unsigned int entries = 100;
    cl_image_format* formats = new cl_image_format[entries];

    cl_uint written = 0;
    cl_int clErr = 0;

    // GET formats for R/W, 2D
    clErr = clGetSupportedImageFormats( m_GpuContext, CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 1 );

    // GET formats for R/-, 2D
    written = 0;
    clErr = clGetSupportedImageFormats( m_GpuContext, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 2 );

    // GET formats for -/W, 2D
    written = 0;
    clErr = clGetSupportedImageFormats( m_GpuContext, CL_MEM_WRITE_ONLY, CL_MEM_OBJECT_IMAGE2D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 4 );
    //-----------------------


    // GET formats for R/W, 3D
    written = 0;
    clErr = clGetSupportedImageFormats( m_GpuContext, CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE3D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 1, 3 );

    // GET formats for R/-, 3D
    written = 0;
    clErr = clGetSupportedImageFormats( m_GpuContext, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE3D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 2, 3 );

    // GET formats for -/W, 3D
    written = 0;
    clErr = clGetSupportedImageFormats( m_GpuContext, CL_MEM_WRITE_ONLY, CL_MEM_OBJECT_IMAGE3D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 4, 3 );
}
