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

#ifndef __mitkFiberBundleDicomWriter_h
#define __mitkFiberBundleDicomWriter_h

#include <mitkAbstractFileWriter.h>
#include "mitkFiberBundle.h"

namespace mitk
{

/**
 * Writes fiber bundles to a DICOM file
 * @ingroup Process
 */
class FiberBundleDicomWriter : public mitk::AbstractFileWriter
{
public:


    FiberBundleDicomWriter();
    FiberBundleDicomWriter(const FiberBundleDicomWriter & other);
    FiberBundleDicomWriter * Clone() const override;
    ~FiberBundleDicomWriter() override;

    using mitk::AbstractFileWriter::Write;
    void Write() override;
};


} // end of namespace mitk

#endif //__mitkFiberBundleWriter_h
