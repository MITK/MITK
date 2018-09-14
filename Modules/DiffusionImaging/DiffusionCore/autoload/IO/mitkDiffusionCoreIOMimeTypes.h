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

#ifndef MITKDiffusionCoreIOMimeTypes_H
#define MITKDiffusionCoreIOMimeTypes_H

#include "mitkCustomMimeType.h"
#include <string>

namespace mitk {

class DiffusionCoreIOMimeTypes
{
public:

  class  DiffusionImageNrrdMimeType : public CustomMimeType
  {
  public:
    DiffusionImageNrrdMimeType();
    bool AppliesTo(const std::string &path) const override;
    DiffusionImageNrrdMimeType* Clone() const override;
  };

  class  DiffusionImageNiftiMimeType : public CustomMimeType
  {
  public:
    DiffusionImageNiftiMimeType();
    bool AppliesTo(const std::string &path) const override;
    DiffusionImageNiftiMimeType* Clone() const override;
  };

  class  DiffusionImageFslMimeType : public CustomMimeType
  {
  public:
    DiffusionImageFslMimeType();
    bool AppliesTo(const std::string &path) const override;
    DiffusionImageFslMimeType* Clone() const override;
  };

  class  DiffusionImageDicomMimeType : public CustomMimeType
  {
  public:
    DiffusionImageDicomMimeType();
    bool AppliesTo(const std::string &path) const override;
    DiffusionImageDicomMimeType* Clone() const override;
  };

  class  PeakImageMimeType : public CustomMimeType
  {
  public:
    PeakImageMimeType();
    bool AppliesTo(const std::string &path) const override;
    PeakImageMimeType* Clone() const override;
  };

  class  SHImageMimeType : public CustomMimeType
  {
  public:
    SHImageMimeType();
    bool AppliesTo(const std::string &path) const override;
    SHImageMimeType* Clone() const override;
  };

  // Get all Diffusion Mime Types
  static std::vector<CustomMimeType*> Get();

  // ------------------------- Image formats (ITK based) --------------------------

  static DiffusionImageNrrdMimeType DWI_NRRD_MIMETYPE();
  static DiffusionImageNiftiMimeType DWI_NIFTI_MIMETYPE();
  static DiffusionImageFslMimeType DWI_FSL_MIMETYPE();
  static DiffusionImageDicomMimeType DWI_DICOM_MIMETYPE();
  static PeakImageMimeType PEAK_MIMETYPE();
  static CustomMimeType DTI_MIMETYPE(); // dti
  static CustomMimeType ODF_MIMETYPE(); // odf, qbi
  static SHImageMimeType SH_MIMETYPE(); // spherical harmonics coefficients

  static std::string PEAK_MIMETYPE_NAME();
  static std::string DWI_NRRD_MIMETYPE_NAME();
  static std::string DWI_NIFTI_MIMETYPE_NAME();
  static std::string DWI_FSL_MIMETYPE_NAME();
  static std::string DWI_DICOM_MIMETYPE_NAME();
  static std::string DTI_MIMETYPE_NAME();
  static std::string ODF_MIMETYPE_NAME();
  static std::string SH_MIMETYPE_NAME();

  static std::string PEAK_MIMETYPE_DESCRIPTION();
  static std::string DWI_NRRD_MIMETYPE_DESCRIPTION();
  static std::string DWI_NIFTI_MIMETYPE_DESCRIPTION();
  static std::string DWI_FSL_MIMETYPE_DESCRIPTION();
  static std::string DWI_DICOM_MIMETYPE_DESCRIPTION();
  static std::string DTI_MIMETYPE_DESCRIPTION();
  static std::string ODF_MIMETYPE_DESCRIPTION();
  static std::string SH_MIMETYPE_DESCRIPTION();

private:

  // purposely not implemented
  DiffusionCoreIOMimeTypes();
  DiffusionCoreIOMimeTypes(const DiffusionCoreIOMimeTypes&);
};

}

#endif // MITKDiffusionCoreIOMimeTypes_H
