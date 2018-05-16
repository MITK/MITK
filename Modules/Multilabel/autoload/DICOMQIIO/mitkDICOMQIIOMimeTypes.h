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

#ifndef MITKDICOMQIIOMIMETYPES_H
#define MITKDICOMQIIOMIMETYPES_H

#include "mitkCustomMimeType.h"

#include <string>

namespace mitk
{
  /// Provides the custom mime types for dicom segmentation objects loaded with DCMQI
  class MitkDICOMQIIOMimeTypes
  {
  public:
    /** Mime type that parses dicom files to determine whether they are dicom segmentation objects.
    */
    class MitkDICOMQIMimeType : public CustomMimeType
    {
    public:
      MitkDICOMQIMimeType();
      bool AppliesTo(const std::string &path) const override;
      MitkDICOMQIMimeType *Clone() const override;
    };

    static MitkDICOMQIMimeType DICOMQI_MIMETYPE();
    static std::string DICOMQI_MIMETYPE_NAME();

    // Get all Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkDICOMQIIOMimeTypes();
    MitkDICOMQIIOMimeTypes(const MitkDICOMQIIOMimeTypes &);
  };
}

#endif // MITKDICOMQIIOMIMETYPES_H
