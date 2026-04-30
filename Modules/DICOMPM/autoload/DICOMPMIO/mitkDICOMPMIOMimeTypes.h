/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMPMIOMimeTypes_h
#define mitkDICOMPMIOMimeTypes_h

#include <mitkCustomMimeType.h>
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
      bool AppliesTo(const std::string &path) const override;
      MitkDICOMPMMimeType *Clone() const override;
    };

    /** \brief Return an instance of the DICOM Parametric Map MIME type. */
    static MitkDICOMPMMimeType DICOMPM_MIMETYPE();

    /** \brief Return the name string for the DICOM Parametric Map MIME type. */
    static std::string DICOMPM_MIMETYPE_NAME();

    /** \brief Return all registered DICOM PM MIME types. Caller takes ownership. */
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkDICOMPMIOMimeTypes();
    MitkDICOMPMIOMimeTypes(const MitkDICOMPMIOMimeTypes &);
  };
}

#endif
