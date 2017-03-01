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

#include "mitkDiffusionCoreIOMimeTypes.h"
#include "mitkIOMimeTypes.h"
#include <itksys/SystemTools.hxx>
#include <itkNrrdImageIO.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <mitkLogMacros.h>

namespace mitk
{

std::vector<CustomMimeType*> DiffusionCoreIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(DWI_NRRD_MIMETYPE().Clone());
  mimeTypes.push_back(DWI_NIFTI_MIMETYPE().Clone());
  mimeTypes.push_back(DWI_FSL_MIMETYPE().Clone());
  mimeTypes.push_back(DTI_MIMETYPE().Clone());
  mimeTypes.push_back(QBI_MIMETYPE().Clone());

  return mimeTypes;
}

// Mime Types

DiffusionCoreIOMimeTypes::DiffusionImageNrrdMimeType::DiffusionImageNrrdMimeType()
  : CustomMimeType(DWI_NRRD_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Image";
  this->SetCategory(category);
  this->SetComment("Diffusion Weighted Images");

  this->AddExtension("dwi");
  //this->AddExtension("hdwi"); // saving with detached header does not work out of the box
  this->AddExtension("nrrd");
}

bool DiffusionCoreIOMimeTypes::DiffusionImageNrrdMimeType::AppliesTo(const std::string &path) const
{
  bool canRead( CustomMimeType::AppliesTo(path) );

  // fix for bug 18572
  // Currently this function is called for writing as well as reading, in that case
  // the image information can of course not be read
  // This is a bug, this function should only be called for reading.
  if( ! itksys::SystemTools::FileExists( path.c_str() ) )
  {
    return canRead;
  }
  //end fix for bug 18572

  std::string ext = this->GetExtension( path );
  ext = itksys::SystemTools::LowerCase( ext );

  // Simple NRRD files should only be considered for this mime type if they contain
  // corresponding tags
  if( ext == ".nrrd" )
  {
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    io->SetFileName(path);
    try
    {
      io->ReadImageInformation();

      itk::MetaDataDictionary imgMetaDictionary = io->GetMetaDataDictionary();
      std::vector<std::string> imgMetaKeys = imgMetaDictionary.GetKeys();
      std::vector<std::string>::const_iterator itKey = imgMetaKeys.begin();
      std::string metaString;

      for (; itKey != imgMetaKeys.end(); itKey ++)
      {
        itk::ExposeMetaData<std::string> (imgMetaDictionary, *itKey, metaString);
        if (itKey->find("modality") != std::string::npos)
        {
          if (metaString.find("DWMRI") != std::string::npos)
          {
            return canRead;
          }
        }
      }

    }
    catch( const itk::ExceptionObject &e )
    {
      MITK_ERROR << "ITK Exception: " << e.what();
    }
    canRead = false;
  }

  return canRead;
}

DiffusionCoreIOMimeTypes::DiffusionImageNrrdMimeType* DiffusionCoreIOMimeTypes::DiffusionImageNrrdMimeType::Clone() const
{
  return new DiffusionImageNrrdMimeType(*this);
}


DiffusionCoreIOMimeTypes::DiffusionImageNrrdMimeType DiffusionCoreIOMimeTypes::DWI_NRRD_MIMETYPE()
{
  return DiffusionImageNrrdMimeType();
}

DiffusionCoreIOMimeTypes::DiffusionImageNiftiMimeType::DiffusionImageNiftiMimeType()
  : CustomMimeType(DWI_NIFTI_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Image";
  this->SetCategory(category);
  this->SetComment("Diffusion Weighted Images");
  this->AddExtension("nii.gz");
  this->AddExtension("nii");
}

bool DiffusionCoreIOMimeTypes::DiffusionImageNiftiMimeType::AppliesTo(const std::string &path) const
{
  bool canRead(CustomMimeType::AppliesTo(path));

  // fix for bug 18572
  // Currently this function is called for writing as well as reading, in that case
  // the image information can of course not be read
  // This is a bug, this function should only be called for reading.
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    return canRead;
  }
  //end fix for bug 18572

  std::string ext = this->GetExtension(path);
  ext = itksys::SystemTools::LowerCase(ext);

  // Nifti files should only be considered for this mime type if they are
  // accompanied by bvecs and bvals files defining the diffusion information
  if (ext == ".nii" || ext == ".nii.gz")
  {
    std::string base = itksys::SystemTools::GetFilenamePath(path) + "/"
      + this->GetFilenameWithoutExtension(path);

    if (itksys::SystemTools::FileExists(std::string(base + ".bvec").c_str())
      && itksys::SystemTools::FileExists(std::string(base + ".bval").c_str())
      )
    {
      return canRead;
    }

    if (itksys::SystemTools::FileExists(std::string(base + ".bvecs").c_str())
      && itksys::SystemTools::FileExists(std::string(base + ".bvals").c_str())
      )
    {
      return canRead;
    }

    canRead = false;
  }

  return canRead;
}

DiffusionCoreIOMimeTypes::DiffusionImageNiftiMimeType* DiffusionCoreIOMimeTypes::DiffusionImageNiftiMimeType::Clone() const
{
  return new DiffusionImageNiftiMimeType(*this);
}


DiffusionCoreIOMimeTypes::DiffusionImageNiftiMimeType DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE()
{
  return DiffusionImageNiftiMimeType();
}

DiffusionCoreIOMimeTypes::DiffusionImageFslMimeType::DiffusionImageFslMimeType()
  : CustomMimeType(DWI_FSL_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Image";
  this->SetCategory(category);
  this->SetComment("Diffusion Weighted Images");
  this->AddExtension("fslgz");
  this->AddExtension("fsl");
}

bool DiffusionCoreIOMimeTypes::DiffusionImageFslMimeType::AppliesTo(const std::string &path) const
{
  bool canRead(CustomMimeType::AppliesTo(path));

  // fix for bug 18572
  // Currently this function is called for writing as well as reading, in that case
  // the image information can of course not be read
  // This is a bug, this function should only be called for reading.
  if (!itksys::SystemTools::FileExists(path.c_str()))
  {
    return canRead;
  }
  //end fix for bug 18572

  std::string ext = this->GetExtension(path);
  ext = itksys::SystemTools::LowerCase(ext);

  // Nifti files should only be considered for this mime type if they are
  // accompanied by bvecs and bvals files defining the diffusion information
  if (ext == ".nii" || ext == ".nii.gz")
  {
    std::string base = itksys::SystemTools::GetFilenamePath(path) + "/"
      + this->GetFilenameWithoutExtension(path);

    if (itksys::SystemTools::FileExists(std::string(base + ".bvec").c_str())
      && itksys::SystemTools::FileExists(std::string(base + ".bval").c_str())
      )
    {
      return canRead;
    }

    if (itksys::SystemTools::FileExists(std::string(base + ".bvecs").c_str())
      && itksys::SystemTools::FileExists(std::string(base + ".bvals").c_str())
      )
    {
      return canRead;
    }

    canRead = false;
  }

  return canRead;
}

DiffusionCoreIOMimeTypes::DiffusionImageFslMimeType* DiffusionCoreIOMimeTypes::DiffusionImageFslMimeType::Clone() const
{
  return new DiffusionImageFslMimeType(*this);
}


DiffusionCoreIOMimeTypes::DiffusionImageFslMimeType DiffusionCoreIOMimeTypes::DWI_FSL_MIMETYPE()
{
  return DiffusionImageFslMimeType();
}

CustomMimeType DiffusionCoreIOMimeTypes::DTI_MIMETYPE()
{
  CustomMimeType mimeType(DTI_MIMETYPE_NAME());
  std::string category = "Tensor Images";
  mimeType.SetComment("Diffusion Tensor Images");
  mimeType.SetCategory(category);
  mimeType.AddExtension("dti");
  //mimeType.AddExtension("hdti"); // saving with detached header does not work out of the box
  return mimeType;
}

CustomMimeType DiffusionCoreIOMimeTypes::QBI_MIMETYPE()
{
  CustomMimeType mimeType(QBI_MIMETYPE_NAME());
  std::string category = "Q-Ball Images";
  mimeType.SetComment("Diffusion Q-Ball Images");
  mimeType.SetCategory(category);
  mimeType.AddExtension("qbi");
  //mimeType.AddExtension("hqbi"); // saving with detached header does not work out of the box
  return mimeType;
}

// Names
std::string DiffusionCoreIOMimeTypes::DWI_NRRD_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dwi";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".nii.gz";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DWI_FSL_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".fslgz";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DTI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dti";
  return name;
}

std::string DiffusionCoreIOMimeTypes::QBI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".qbi";
  return name;
}

// Descriptions

std::string DiffusionCoreIOMimeTypes::DWI_NRRD_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Weighted Images";
  return description;
}

std::string DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Weighted Images";
  return description;
}

std::string DiffusionCoreIOMimeTypes::DWI_FSL_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Weighted Images";
  return description;
}

std::string DiffusionCoreIOMimeTypes::DTI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Tensor Images";
  return description;
}

std::string DiffusionCoreIOMimeTypes::QBI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Q-Ball Images";
  return description;
}

}
