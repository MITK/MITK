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

#ifndef MITKDICOMSEGIOMIMETYPES_H
#define MITKDICOMSEGIOMIMETYPES_H

#include "mitkCustomMimeType.h"
#include <MitkDICOMSegIOExports.h>


#include <string>

namespace mitk
{
  /// Provides the custom mime types for dicom qi objects loaded with DCMQI
  class MITKDICOMSEGIO_EXPORT MitkDICOMSEGIOMimeTypes
  {
  public:
    /** Mime type that parses dicom files to determine whether they are dicom segmentation objects.
    */
    class  MITKDICOMSEGIO_EXPORT MitkDICOMSEGMimeType : public CustomMimeType
    {
    public:
      MitkDICOMSEGMimeType();
      bool AppliesTo(const std::string &path) const override;
      MitkDICOMSEGMimeType *Clone() const override;
    };

    static MitkDICOMSEGMimeType DICOMSEG_MIMETYPE();
    static std::string DICOMSEG_MIMETYPE_NAME();

	

    // Get all Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkDICOMSEGIOMimeTypes();
    MitkDICOMSEGIOMimeTypes(const MitkDICOMSEGIOMimeTypes &);
  };
}

#endif // MITKDICOMSEGIOMIMETYPES_H
