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

#ifndef MITKDicomRTIOMimeTypes_H
#define MITKDicomRTIOMimeTypes_H

#include "mitkCustomMimeType.h"
#include <mitkIDICOMTagsOfInterest.h>

#include <string>

namespace mitk {

class DicomRTIOMimeTypes
{
public:

  class  RTDoseMimeType : public CustomMimeType
  {
  public:
    RTDoseMimeType();
    bool AppliesTo(const std::string &path) const override;

    RTDoseMimeType* Clone() const override;
  };
  
  class  RTStructMimeType : public CustomMimeType
  {
  public:
    RTStructMimeType();
    bool AppliesTo(const std::string &path) const override;
    RTStructMimeType* Clone() const override;
  };

  class  RTPlanMimeType : public CustomMimeType
  {
  public:
    RTPlanMimeType();
    bool AppliesTo(const std::string &path) const override;
    RTPlanMimeType* Clone() const override;
  };
  // Get all DicomRT Mime Types
  static std::vector<CustomMimeType*> Get();

  static RTDoseMimeType DICOMRT_DOSE_MIMETYPE();
  static RTStructMimeType DICOMRT_STRUCT_MIMETYPE();
  static RTPlanMimeType DICOMRT_PLAN_MIMETYPE();

  static std::string DICOMRT_DOSE_MIMETYPE_NAME();
  static std::string DICOMRT_STRUCT_MIMETYPE_NAME();
  static std::string DICOMRT_PLAN_MIMETYPE_NAME();

  static std::string DICOMRT_DOSE_MIMETYPE_DESCRIPTION();
  static std::string DICOMRT_STRUCT_MIMETYPE_DESCRIPTION();
  static std::string DICOMRT_PLAN_MIMETYPE_DESCRIPTION();

  DicomRTIOMimeTypes() = delete;
  DicomRTIOMimeTypes(const DicomRTIOMimeTypes&) = delete;
  
  static mitk::IDICOMTagsOfInterest* GetDicomTagsOfInterestService();
  static bool canReadByDicomFileReader(const std::string & path);
  static std::string GetModality(const std::string & path);
};

}

#endif // MITKDicomRTIOMimeTypes_H
