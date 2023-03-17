/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCESTIOMimeTypes_h
#define mitkCESTIOMimeTypes_h

#include "mitkIOMimeTypes.h"

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
    class MitkCESTDicomMimeType : public IOMimeTypes::BaseDicomMimeType
    {
    public:
      MitkCESTDicomMimeType();
      bool AppliesTo(const std::string &path) const override;
      MitkCESTDicomMimeType *Clone() const override;
    };

    static MitkCESTDicomMimeType CEST_DICOM_MIMETYPE();
    static std::string CEST_DICOM_MIMETYPE_NAME();

    /** Mime type that indicated generic CEST dicom files.
    *
    * The mime type assumes that dicom files that have a CEST_META.json file in the
    * same directory are CEST DICOMs and relevant for this mime type.
    */
    class MitkCESTDicomWithMetaFileMimeType : public IOMimeTypes::BaseDicomMimeType
    {
    public:
      MitkCESTDicomWithMetaFileMimeType();
      bool AppliesTo(const std::string& path) const override;
      MitkCESTDicomWithMetaFileMimeType* Clone() const override;
    };

    static MitkCESTDicomWithMetaFileMimeType CEST_DICOM_WITH_META_FILE_MIMETYPE();
    static std::string CEST_DICOM_WITH_META_FILE_NAME();

    /** Mime type that indicated dicom files that can be potantially read as Generic
    * CEST but have *NO* CEST meta information.
    *
    * The mime type is used to offer the manual CEST loading for all DICOM images with
    * low priority if no CEST meta file is present.
    */
    class MitkCESTDicomWOMetaFileMimeType : public IOMimeTypes::BaseDicomMimeType
    {
    public:
      MitkCESTDicomWOMetaFileMimeType();
      bool AppliesTo(const std::string& path) const override;
      MitkCESTDicomWOMetaFileMimeType* Clone() const override;
    };

    static MitkCESTDicomWOMetaFileMimeType CEST_DICOM_WITHOUT_META_FILE_MIMETYPE();
    static std::string CEST_DICOM_WITHOUT_META_FILE_NAME();

    // Get all Mime Types
    static std::vector<CustomMimeType *> Get();

  private:
    // purposely not implemented
    MitkCESTIOMimeTypes();
    MitkCESTIOMimeTypes(const MitkCESTIOMimeTypes &);
  };
}

#endif
