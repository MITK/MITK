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

#ifndef mitkDICOMDatasetAccess_h
#define mitkDICOMDatasetAccess_h

#include "mitkDICOMTag.h"

#include "DICOMReaderExports.h"

namespace mitk
{

class DICOMReader_EXPORT DICOMDatasetAccess
{
  public:

    virtual std::string GetFilenameIfAvailable() const = 0;
    virtual std::string GetTagValueAsString(const mitk::DICOMTag& tag) const = 0;

    virtual ~DICOMDatasetAccess() {};
};


typedef std::vector<DICOMDatasetAccess*> DICOMDatasetList;

}

#endif
