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

namespace mitk
{

std::vector<CustomMimeType*> DiffusionIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(DWI_MIMETYPE().Clone());
  mimeTypes.push_back(DTI_MIMETYPE().Clone());
  mimeTypes.push_back(QBI_MIMETYPE().Clone());

  mimeTypes.push_back(FIBERBUNDLE_MIMETYPE().Clone());

  mimeTypes.push_back(CONNECTOMICS_MIMETYPE().Clone());

  return mimeTypes;
}

// Mime Types

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_MIMETYPE_NAME());
  std::string category = "Fiber Bundle File";
  mimeType.SetComment("Fiber Bundles");
  mimeType.SetCategory(category);
  mimeType.AddExtension("fib");
  mimeType.AddExtension("trk");
  //mimeType.AddExtension("vtk");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::DWI_MIMETYPE()
{
  CustomMimeType mimeType(DWI_MIMETYPE_NAME());
  std::string category = "Diffusion Weighted Image";
  mimeType.SetComment("Diffusion Weighted Images");
  mimeType.SetCategory(category);
  mimeType.AddExtension("dwi");
  mimeType.AddExtension("hdwi");
  mimeType.AddExtension("fsl");
  mimeType.AddExtension("fslgz");
  return mimeType;
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
  mimeType.SetComment("Connectomics Networks");
  mimeType.SetCategory(category);
  mimeType.AddExtension("cnf");
  return mimeType;
}

// Names
std::string DiffusionIOMimeTypes::DWI_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".dwi";
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

std::string DiffusionIOMimeTypes::FIBERBUNDLE_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".fib";
  return name;
}

std::string DiffusionIOMimeTypes::CONNECTOMICS_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".cnf";
  return name;
}

// Descriptions
std::string DiffusionIOMimeTypes::FIBERBUNDLE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Fiberbundles";
  return description;
}

std::string DiffusionIOMimeTypes::DWI_MIMETYPE_DESCRIPTION()
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

}
