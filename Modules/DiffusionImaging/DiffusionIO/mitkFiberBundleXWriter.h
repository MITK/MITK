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

#ifndef __mitkFiberBundleXWriter_h
#define __mitkFiberBundleXWriter_h

#include <mitkAbstractFileWriter.h>

#include "mitkFiberBundleX.h"
#include <vtkPolyDataWriter.h>



namespace mitk
{

/**
 * Writes fiber bundles to a file
 * @ingroup Process
 */
class FiberBundleXWriter : public mitk::AbstractFileWriter
{
public:


    FiberBundleXWriter();
    FiberBundleXWriter(const FiberBundleXWriter & other);
    virtual FiberBundleXWriter * Clone() const;
    virtual ~FiberBundleXWriter();

    using mitk::AbstractFileWriter::Write;
    virtual void Write();

    static const char* XML_GEOMETRY;

    static const char* XML_MATRIX_XX;

    static const char* XML_MATRIX_XY;

    static const char* XML_MATRIX_XZ;

    static const char* XML_MATRIX_YX;

    static const char* XML_MATRIX_YY;

    static const char* XML_MATRIX_YZ;

    static const char* XML_MATRIX_ZX;

    static const char* XML_MATRIX_ZY;

    static const char* XML_MATRIX_ZZ;

    static const char* XML_ORIGIN_X;

    static const char* XML_ORIGIN_Y;

    static const char* XML_ORIGIN_Z;

    static const char* XML_SPACING_X;

    static const char* XML_SPACING_Y;

    static const char* XML_SPACING_Z;

    static const char* XML_SIZE_X;

    static const char* XML_SIZE_Y;

    static const char* XML_SIZE_Z;

    static const char* XML_FIBER_BUNDLE;

    static const char* XML_FIBER;

    static const char* XML_PARTICLE;

    static const char* XML_ID;

    static const char* XML_POS_X;

    static const char* XML_POS_Y;

    static const char* XML_POS_Z;

    static const char* VERSION_STRING;

    static const char* XML_FIBER_BUNDLE_FILE;

    static const char* XML_FILE_VERSION;

    static const char* XML_NUM_FIBERS;

    static const char* XML_NUM_PARTICLES;

    static const char* ASCII_FILE;

    static const char* FILE_NAME;

};


} // end of namespace mitk

#endif //__mitkFiberBundleXWriter_h
