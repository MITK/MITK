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

#include "MitkDICOMReaderExports.h"

namespace mitk
{

/**
  \ingroup DICOMReaderModule
  \brief Interface to datasets that is presented to sorting classes such as DICOMDatasetSorter.

  Minimal interface to hide actual implementation, which might rely on GDCM.
*/
class MitkDICOMReader_EXPORT DICOMDatasetAccess
{
  public:

    /// \brief Return a filename if possible.
    /// If DICOM is not read from file but from somewhere else (network, database), we might not have files.
    virtual std::string GetFilenameIfAvailable() const = 0;

    /// \brief Return the raw value of the tag as a string
    virtual std::string GetTagValueAsString(const mitk::DICOMTag& tag) const = 0;

    virtual ~DICOMDatasetAccess() {};
};


typedef std::vector<DICOMDatasetAccess*> DICOMDatasetList;

}

#endif
