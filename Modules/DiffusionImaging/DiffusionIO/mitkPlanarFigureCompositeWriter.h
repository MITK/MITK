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

#ifndef __mitkPlanarFigureCompositeWriter_h
#define __mitkPlanarFigureCompositeWriter_h

#include <mitkAbstractFileWriter.h>

namespace mitk
{

/**
 * Writes fiber bundles to a file
 * @ingroup Process
 */
class PlanarFigureCompositeWriter : public mitk::AbstractFileWriter
{
public:


    PlanarFigureCompositeWriter();
    PlanarFigureCompositeWriter(const PlanarFigureCompositeWriter & other);
    PlanarFigureCompositeWriter * Clone() const override;
    ~PlanarFigureCompositeWriter() override;

    using mitk::AbstractFileWriter::Write;
    void Write() override;

};


} // end of namespace mitk

#endif //__mitkFiberBundleWriter_h
