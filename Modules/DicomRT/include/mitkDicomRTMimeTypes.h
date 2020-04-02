/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKDicomRTMimeTypes_H
#define MITKDicomRTMimeTypes_H

#include <mitkCustomMimeType.h>
#include <mitkIDICOMTagsOfInterest.h>
#include <MitkDicomRTExports.h>

#include <array>
#include <memory>
#include <string>

namespace mitk {

class MITKDICOMRT_EXPORT DicomRTMimeTypes
{
public:

  class MITKDICOMRT_EXPORT RTDoseMimeType : public CustomMimeType
  {
  public:
    RTDoseMimeType();
    bool AppliesTo(const std::string &path) const override;

    RTDoseMimeType* Clone() const override;
  };

  class MITKDICOMRT_EXPORT RTStructMimeType : public CustomMimeType
  {
  public:
    RTStructMimeType();
    bool AppliesTo(const std::string &path) const override;
    RTStructMimeType* Clone() const override;
  };

  class MITKDICOMRT_EXPORT RTPlanMimeType : public CustomMimeType
  {
  public:
    RTPlanMimeType();
    bool AppliesTo(const std::string &path) const override;
    RTPlanMimeType* Clone() const override;
  };
  // Get all DicomRT Mime Types
  static std::array<std::unique_ptr<CustomMimeType>, 3> Get();

  static RTDoseMimeType DICOMRT_DOSE_MIMETYPE();
  static RTStructMimeType DICOMRT_STRUCT_MIMETYPE();
  static RTPlanMimeType DICOMRT_PLAN_MIMETYPE();

  static std::string DICOMRT_DOSE_MIMETYPE_NAME();
  static std::string DICOMRT_STRUCT_MIMETYPE_NAME();
  static std::string DICOMRT_PLAN_MIMETYPE_NAME();

  static std::string DICOMRT_DOSE_MIMETYPE_DESCRIPTION();
  static std::string DICOMRT_STRUCT_MIMETYPE_DESCRIPTION();
  static std::string DICOMRT_PLAN_MIMETYPE_DESCRIPTION();

  DicomRTMimeTypes() = delete;
  DicomRTMimeTypes(const DicomRTMimeTypes&) = delete;

  static mitk::IDICOMTagsOfInterest* GetDicomTagsOfInterestService();
  static bool canReadByDicomFileReader(const std::string & path);
  static std::string GetModality(const std::string & path);
};

}

#endif // MITKDicomRTMimeTypes_H
