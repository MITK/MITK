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

#ifndef MITKCESTIOMIMETYPES_H
#define MITKCESTIOMIMETYPES_H

#include "mitkCustomMimeType.h"

#include <string>

namespace mitk
{
  /// Provides the custom mime types for MitkCEST
  class MitkCESTIOMimeTypes
  {
  public:
    /** Mime type that parses dicom files to determine whether they are CEST dicom files.
    *
    * Accepts dicom files that contain the substring "CEST_Rev" (via the mitk::CustomTagParser parsing)
    * in the tSequenceFileName parameter in dicom tag (0x0029, 0x1020)
    */
    class MitkCESTDicomMimeType : public CustomMimeType
    {
    public:
      MitkCESTDicomMimeType();
      virtual bool AppliesTo(const std::string &path) const override;
      virtual MitkCESTDicomMimeType *Clone() const override;
    };

    static MitkCESTDicomMimeType CEST_DICOM_MIMETYPE();
    static std::string CEST_DICOM_MIMETYPE_NAME();

    // Get all Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkCESTIOMimeTypes();
    MitkCESTIOMimeTypes(const MitkCESTIOMimeTypes &);
  };
}

#endif // MITKCESTIOMIMETYPES_H
