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

mitk::OclImageFormats::OclImageFormats( cl_context ctxt)
    :m_Image2DSupport( NULL ), m_Image3DSupport( NULL ),
    m_gpuContext(ctxt)
{
    const unsigned int matrixSize = MAX_FORMATS * MAX_DATA_TYPES;

    this->m_Image2DSupport = new unsigned char[matrixSize];
    this->m_Image3DSupport = new unsigned char[matrixSize];

    for( unsigned int i = 0; i<matrixSize; i++ )
    {
        this->m_Image2DSupport[i] = 0;
        this->m_Image3DSupport[i] = 0;
    }

    this->CollectAvailableFormats();

}

mitk::OclImageFormats::~OclImageFormats()
{

}

void mitk::OclImageFormats::PrintSelf()
{
    std::cout << "Values: Read-Write(1) ReadOnly(2) , WriteOnly(4) \n";
    std::cout << "ROWS: [CL_A, CL_R, CL_RA, CL_RG, CL_RGB, CL_RGBA, CL_ARGB, CL_BGRA, CL_LUM, CL_INT]" << std::endl;

    const unsigned int matrixSize = MAX_FORMATS * MAX_DATA_TYPES;

    for( unsigned int i = 0; i<matrixSize; i++ )
    {
        std::cout << (int) this->m_Image2DSupport[i] << ", \t";
        if( (i+1) % MAX_DATA_TYPES == 0 )
            std::cout << " \n";
    }
    std::cout << "========================== \n";

    for( unsigned int i = 0; i<matrixSize; i++ )
    {
        std::cout << (int) this->m_Image3DSupport[i] << ", \t";
        if( (i+1) % MAX_DATA_TYPES == 0 )
            std::cout << " \n";
    }

}

unsigned int mitk::OclImageFormats::GetOffset(cl_image_format fmt)
{
    unsigned int offset = 0;

    switch( fmt.image_channel_order )
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

    switch ( fmt.image_channel_data_type )
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

bool mitk::OclImageFormats::IsFormatSupported(cl_image_format *fmt)
{
    bool retVal = false;

    // FIXME needs finer subdivision...
    if ( this->m_Image2DSupport[ GetOffset(*fmt)] > 4 )
        retVal = true;

    return retVal;
}

bool mitk::OclImageFormats::GetNearestSupported(cl_image_format *in, cl_image_format *out)
{
    bool retVal = false;

    // init output format
    out->image_channel_data_type = in->image_channel_data_type;
    out->image_channel_order = in->image_channel_order;

    // the input format is supported, just copy the information into out
    if( this->IsFormatSupported(in) )
    {
        retVal = true;
    }
    else
    {
        // get the 'nearest' format
        // try RGBA first
        cl_image_format test;
        test.image_channel_order = CL_RGBA;
        test.image_channel_data_type = in->image_channel_data_type;

        if(this->IsFormatSupported( &test) )
        {
            out->image_channel_order = CL_RGBA;
        }

    }

    return retVal;

}

void mitk::OclImageFormats::SortFormats(cl_image_format *formats, cl_uint count, int val, int dims)
{
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
    const unsigned int entries = 100;
    cl_image_format* formats = new cl_image_format[entries];

    cl_uint written = 0;
    cl_int clErr = 0;

    // GET formats for R/W, 2D
    clErr = clGetSupportedImageFormats( m_gpuContext, CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE2D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 1 );

    // GET formats for R/-, 2D
    written = 0;
    clErr = clGetSupportedImageFormats( m_gpuContext, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE2D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 2 );

    // GET formats for -/W, 2D
    written = 0;
    clErr = clGetSupportedImageFormats( m_gpuContext, CL_MEM_WRITE_ONLY, CL_MEM_OBJECT_IMAGE2D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 4 );
    //-----------------------


    // GET formats for R/W, 3D
    written = 0;
    clErr = clGetSupportedImageFormats( m_gpuContext, CL_MEM_READ_WRITE, CL_MEM_OBJECT_IMAGE3D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 1, 3 );

    // GET formats for R/-, 3D
    written = 0;
    clErr = clGetSupportedImageFormats( m_gpuContext, CL_MEM_READ_ONLY, CL_MEM_OBJECT_IMAGE3D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 2, 3 );

    // GET formats for -/W, 3D
    written = 0;
    clErr = clGetSupportedImageFormats( m_gpuContext, CL_MEM_WRITE_ONLY, CL_MEM_OBJECT_IMAGE3D, entries, formats, &written);
    CHECK_OCL_ERR( clErr );

    this->SortFormats( formats, written, 4, 3 );

}
