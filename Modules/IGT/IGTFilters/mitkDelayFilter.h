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

#ifndef MITKDELAYFILTER_H
#define MITKDELAYFILTER_H

#include "MitkIGTExports.h"
#include "mitkBaseDataSource.h"

namespace mitk {

class MitkIGT_EXPORT DelayFilter : public mitk::BaseDataSource
{
    /**
     * @sa itk::ProcessObject::MakeOutput(DataObjectPointerArraySizeType)
     */
    virtual DataObjectPointer MakeOutput ( DataObjectPointerArraySizeType idx );

    /**
     * @sa itk::ProcessObject::MakeOutput(const DataObjectIdentifierType&)
     */
    virtual DataObjectPointer MakeOutput(const DataObjectIdentifierType& name);

protected:
    virtual void GenerateData();
};

} // namespace mitk

#endif // MITKDELAYFILTER_H
