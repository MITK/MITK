/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMSegIOMimeTypes_h
#define mitkDICOMSegIOMimeTypes_h

#include <mitkCustomMimeType.h>
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

    /** \brief Return an instance of the DICOM Segmentation MIME type. */
    static MitkDICOMSEGMimeType DICOMSEG_MIMETYPE();

    /** \brief Return the name string for the DICOM Segmentation MIME type. */
    static std::string DICOMSEG_MIMETYPE_NAME();

    /** \brief Return all registered DICOM Segmentation MIME types. Caller takes ownership. */
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkDICOMSEGIOMimeTypes();
    MitkDICOMSEGIOMimeTypes(const MitkDICOMSEGIOMimeTypes &);
  };
}

#endif
