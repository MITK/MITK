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

#ifndef MITKDIFFUSIONIOMIMETYPES_H
#define MITKDIFFUSIONIOMIMETYPES_H

#include "mitkCustomMimeType.h"
#include <MitkDiffusionIOExports.h>

#include <string>

namespace mitk {

class DiffusionIOMimeTypes
{
public:

  class MITKDIFFUSIONIO_EXPORT DiffusionImageNrrdMimeType : public CustomMimeType
  {
  public:
    DiffusionImageNrrdMimeType();
    virtual bool AppliesTo(const std::string &path) const override;
    virtual DiffusionImageNrrdMimeType* Clone() const override;
  };

  class MITKDIFFUSIONIO_EXPORT DiffusionImageNiftiMimeType : public CustomMimeType
  {
  public:
    DiffusionImageNiftiMimeType();
    virtual bool AppliesTo(const std::string &path) const override;
    virtual DiffusionImageNiftiMimeType* Clone() const override;
  };
  // Get all Diffusion Mime Types
  static std::vector<CustomMimeType*> Get();

  // ------------------------------ VTK formats ----------------------------------

  static CustomMimeType FIBERBUNDLE_VTK_MIMETYPE();
  static std::string FIBERBUNDLE_VTK_MIMETYPE_NAME();

  static std::string FIBERBUNDLE_MIMETYPE_DESCRIPTION();

  // ------------------------------ TrackVis formats ----------------------------------

  static CustomMimeType FIBERBUNDLE_TRK_MIMETYPE();
  static std::string FIBERBUNDLE_TRK_MIMETYPE_NAME();


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

  // ------------------------------ MITK formats ----------------------------------

  static CustomMimeType CONNECTOMICS_MIMETYPE(); // cnf

  static std::string CONNECTOMICS_MIMETYPE_NAME();

  static std::string CONNECTOMICS_MIMETYPE_DESCRIPTION();

  static CustomMimeType PLANARFIGURECOMPOSITE_MIMETYPE();

  static std::string PLANARFIGURECOMPOSITE_MIMETYPE_NAME();

  static std::string PLANARFIGURECOMPOSITE_MIMETYPE_DESCRIPTION();

private:

  // purposely not implemented
  DiffusionIOMimeTypes();
  DiffusionIOMimeTypes(const DiffusionIOMimeTypes&);
};

}

#endif // MITKDIFFUSIONIOMIMETYPES_H
