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
#include <MitkDICOMQIExports.h>


#include <string>

namespace mitk
{
  /// Provides the custom mime types for dicom qi objects loaded with DCMQI
  class MITKDICOMQI_EXPORT MitkDICOMQIIOMimeTypes
  {
  public:
    /** Mime type that parses dicom files to determine whether they are dicom segmentation objects.
    */
    class  MITKDICOMQI_EXPORT MitkDICOMSEGMimeType : public CustomMimeType
    {
    public:
      MitkDICOMSEGMimeType();
      bool AppliesTo(const std::string &path) const override;
      MitkDICOMSEGMimeType *Clone() const override;
    };

    static MitkDICOMSEGMimeType DICOMSEG_MIMETYPE();
    static std::string DICOMSEG_MIMETYPE_NAME();

    /** Mime type that parses dicom files to determine whether they are dicom pm objects.
    */
    /*
    class MitkDICOMPMMimeType : public CustomMimeType
    {
    public:
      MitkDICOMPMMimeType();
      virtual bool AppliesTo(const std::string &path) const override;
      virtual MitkDICOMPMMimeType *Clone() const override;
    };

    static MitkDICOMPMMimeType DICOMPM_MIMETYPE();
    static std::string DICOMPM_MIMETYPE_NAME();
    */


    // Get all Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkDICOMQIIOMimeTypes();
    MitkDICOMQIIOMimeTypes(const MitkDICOMQIIOMimeTypes &);
  };
}

#endif // MITKDICOMQIIOMIMETYPES_H
