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

#ifndef MITKDICOMPMIOMIMETYPES_H
#define MITKDICOMPMIOMIMETYPES_H

#include "mitkCustomMimeType.h"
#include <MitkDICOMPMIOExports.h>


#include <string>

namespace mitk
{
  /// Provides the custom mime types for dicom qi objects loaded with DCMQI
  class MITKDICOMPMIO_EXPORT MitkDICOMPMIOMimeTypes
  {
  public:

    /** Mime type that parses dicom files to determine whether they are dicom pm objects.
    */

    class MITKDICOMPMIO_EXPORT MitkDICOMPMMimeType : public CustomMimeType
    {
    public:
      MitkDICOMPMMimeType();
      virtual bool AppliesTo(const std::string &path) const override;
      virtual MitkDICOMPMMimeType *Clone() const override;
    };

    static MitkDICOMPMMimeType DICOMPM_MIMETYPE();
    static std::string DICOMPM_MIMETYPE_NAME();

	

    // Get all Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkDICOMPMIOMimeTypes();
    MitkDICOMPMIOMimeTypes(const MitkDICOMPMIOMimeTypes &);
  };
}

#endif // MITKDICOMPMIOMIMETYPES_H
