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

#include "mitkDiffusionIOMimeTypes.h"
#include "mitkIOMimeTypes.h"
#include <itksys/SystemTools.hxx>
#include <itkNrrdImageIO.h>
#include <itkMetaDataDictionary.h>
#include <itkMetaDataObject.h>
#include <mitkLogMacros.h>

namespace mitk
{

std::vector<CustomMimeType*> DiffusionIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(DWI_NRRD_MIMETYPE().Clone());
  mimeTypes.push_back(DWI_NIFTI_MIMETYPE().Clone());
  mimeTypes.push_back(DTI_MIMETYPE().Clone());
  mimeTypes.push_back(QBI_MIMETYPE().Clone());

  mimeTypes.push_back(FIBERBUNDLE_VTK_MIMETYPE().Clone());
  mimeTypes.push_back(FIBERBUNDLE_TRK_MIMETYPE().Clone());

  mimeTypes.push_back(CONNECTOMICS_MIMETYPE().Clone());

  mimeTypes.push_back(PLANARFIGURECOMPOSITE_MIMETYPE().Clone());

  return mimeTypes;
}

// Mime Types

CustomMimeType DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE()
{
  CustomMimeType mimeType(PLANARFIGURECOMPOSITE_MIMETYPE_NAME());
  std::string category = "Planar Figure Composite";
  mimeType.SetComment("Planar Figure Composite");
  mimeType.SetCategory(category);
  mimeType.AddExtension("pfc");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_VTK_MIMETYPE_NAME());
  std::string category = "VTK Fibers";
  mimeType.SetComment("VTK Fibers");
  mimeType.SetCategory(category);
  mimeType.AddExtension("fib");
//  mimeType.AddExtension("vtk");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_TRK_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_TRK_MIMETYPE_NAME());
  std::string category = "TrackVis Fibers";
  mimeType.SetComment("TrackVis Fibers");
  mimeType.SetCategory(category);
  mimeType.AddExtension("trk");
  return mimeType;
}

DiffusionIOMimeTypes::DiffusionImageNrrdMimeType::DiffusionImageNrrdMimeType()
  : CustomMimeType(DWI_NRRD_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Image";
  this->SetCategory(category);
  this->SetComment("Diffusion Weighted Images");

  this->AddExtension("dwi");
  this->AddExtension("hdwi");
  this->AddExtension("nrrd");
}

bool DiffusionIOMimeTypes::DiffusionImageNrrdMimeType::AppliesTo(const std::string &path) const
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

DiffusionIOMimeTypes::DiffusionImageNrrdMimeType* DiffusionIOMimeTypes::DiffusionImageNrrdMimeType::Clone() const
{
  return new DiffusionImageNrrdMimeType(*this);
}


DiffusionIOMimeTypes::DiffusionImageNrrdMimeType DiffusionIOMimeTypes::DWI_NRRD_MIMETYPE()
{
  return DiffusionImageNrrdMimeType();
}

DiffusionIOMimeTypes::DiffusionImageNiftiMimeType::DiffusionImageNiftiMimeType()
  : CustomMimeType(DWI_NIFTI_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Image";
  this->SetCategory(category);
  this->SetComment("Diffusion Weighted Images");
  this->AddExtension("fsl");
  this->AddExtension("fslgz");
  this->AddExtension("nii");
  this->AddExtension("nii.gz");
}

bool DiffusionIOMimeTypes::DiffusionImageNiftiMimeType::AppliesTo(const std::string &path) const
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

DiffusionIOMimeTypes::DiffusionImageNiftiMimeType* DiffusionIOMimeTypes::DiffusionImageNiftiMimeType::Clone() const
{
  return new DiffusionImageNiftiMimeType(*this);
}


DiffusionIOMimeTypes::DiffusionImageNiftiMimeType DiffusionIOMimeTypes::DWI_NIFTI_MIMETYPE()
{
  return DiffusionImageNiftiMimeType();
}

CustomMimeType DiffusionIOMimeTypes::DTI_MIMETYPE()
{
  CustomMimeType mimeType(DTI_MIMETYPE_NAME());
  std::string category = "Tensor Images";
  mimeType.SetComment("Diffusion Tensor Images");
  mimeType.SetCategory(category);
  mimeType.AddExtension("dti");
  mimeType.AddExtension("hdti");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::QBI_MIMETYPE()
{
  CustomMimeType mimeType(QBI_MIMETYPE_NAME());
  std::string category = "Q-Ball Images";
  mimeType.SetComment("Diffusion Q-Ball Images");
  mimeType.SetCategory(category);
  mimeType.AddExtension("qbi");
  mimeType.AddExtension("hqbi");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE()
{
  CustomMimeType mimeType(CONNECTOMICS_MIMETYPE_NAME());
  std::string category = "Connectomics Networks";
  mimeType.SetComment("Connectomics Networks Files");
  mimeType.SetCategory(category);
  mimeType.AddExtension("cnf");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::CONNECTOMICS_MATRIX_MIMETYPE()
{
  CustomMimeType mimeType(CONNECTOMICS_MATRIX_MIMETYPE_NAME());
  std::string category = "Connectomics Networks export";
  mimeType.SetComment("Connectomics Matrix Files");
  mimeType.SetCategory(category);
  mimeType.AddExtension("mat");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::CONNECTOMICS_LIST_MIMETYPE()
{
  CustomMimeType mimeType(CONNECTOMICS_LIST_MIMETYPE_NAME());
  std::string category = "Connectomics Networks export";
  mimeType.SetComment("Connectomics Connection Lists");
  mimeType.SetCategory(category);
  mimeType.AddExtension("txt");
  return mimeType;
}

// Names
std::string DiffusionIOMimeTypes::DWI_NRRD_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dwi";
  return name;
}

std::string DiffusionIOMimeTypes::DWI_NIFTI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".fsl";
  return name;
}

std::string DiffusionIOMimeTypes::DTI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dti";
  return name;
}

std::string DiffusionIOMimeTypes::QBI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".qbi";
  return name;
}

std::string DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.vtk";
  return name;
}

std::string DiffusionIOMimeTypes::FIBERBUNDLE_TRK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.trk";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".cnf";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MATRIX_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".mat";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_LIST_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".txt";
  return name;
}

std::string DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".pfc";
  return name;
}

// Descriptions
std::string DiffusionIOMimeTypes::FIBERBUNDLE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Fiberbundles";
  return description;
}

std::string DiffusionIOMimeTypes::DWI_NRRD_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Weighted Images";
  return description;
}

std::string DiffusionIOMimeTypes::DWI_NIFTI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Weighted Images";
  return description;
}

std::string DiffusionIOMimeTypes::DTI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Tensor Images";
  return description;
}

std::string DiffusionIOMimeTypes::QBI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Q-Ball Images";
  return description;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Connectomics Networks";
  return description;
}

std::string DiffusionIOMimeTypes::PLANARFIGURECOMPOSITE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Planar Figure Composite";
  return description;
}

}
