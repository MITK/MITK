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

#ifndef __mitkTractographyForestWriter_h
#define __mitkTractographyForestWriter_h

#include <mitkAbstractFileWriter.h>

#include "mitkFiberBundle.h"
#include <vtkPolyDataWriter.h>



namespace mitk
{

/**
 * Writes random forests used for fiber tractography
 * @ingroup Process
 */
class TractographyForestWriter : public mitk::AbstractFileWriter
{
public:


    TractographyForestWriter();
    TractographyForestWriter(const TractographyForestWriter & other);
    TractographyForestWriter * Clone() const override;
    ~TractographyForestWriter() override;

    using mitk::AbstractFileWriter::Write;
    void Write() override;

};


} // end of namespace mitk

#endif //__mitkFiberBundleWriter_h
