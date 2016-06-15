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
    virtual bool AppliesTo(const std::string &path) const override;
    virtual DiffusionImageNrrdMimeType* Clone() const override;
  };

  class  DiffusionImageNiftiMimeType : public CustomMimeType
  {
  public:
    DiffusionImageNiftiMimeType();
    virtual bool AppliesTo(const std::string &path) const override;
    virtual DiffusionImageNiftiMimeType* Clone() const override;
  };
  // Get all Diffusion Mime Types
  static std::vector<CustomMimeType*> Get();

  // ------------------------- Image formats (ITK based) --------------------------

  static DiffusionImageNrrdMimeType DWI_NRRD_MIMETYPE();
  static DiffusionImageNiftiMimeType DWI_NIFTI_MIMETYPE();
  static CustomMimeType DTI_MIMETYPE(); // dti, hdti
  static CustomMimeType QBI_MIMETYPE(); // qbi, hqbi

  static std::string DWI_NRRD_MIMETYPE_NAME();
  static std::string DWI_NIFTI_MIMETYPE_NAME();
  static std::string DTI_MIMETYPE_NAME();
  static std::string QBI_MIMETYPE_NAME();

  static std::string DWI_NRRD_MIMETYPE_DESCRIPTION();
  static std::string DWI_NIFTI_MIMETYPE_DESCRIPTION();
  static std::string DTI_MIMETYPE_DESCRIPTION();
  static std::string QBI_MIMETYPE_DESCRIPTION();

private:

  // purposely not implemented
  DiffusionCoreIOMimeTypes();
  DiffusionCoreIOMimeTypes(const DiffusionCoreIOMimeTypes&);
};

}

#endif // MITKDiffusionCoreIOMimeTypes_H
