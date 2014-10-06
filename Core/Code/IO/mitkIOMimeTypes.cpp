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

#include "mitkIOMimeTypes.h"

#include "mitkCustomMimeType.h"

namespace mitk {

std::vector<CustomMimeType> IOMimeTypes::Get()
{
  std::vector<CustomMimeType> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(NRRD_MIMETYPE());
  mimeTypes.push_back(NIFTI_MIMETYPE());

  mimeTypes.push_back(VTK_IMAGE_MIMETYPE());
  mimeTypes.push_back(VTK_PARALLEL_IMAGE_MIMETYPE());
  mimeTypes.push_back(VTK_IMAGE_LEGACY_MIMETYPE());

  mimeTypes.push_back(VTK_POLYDATA_MIMETYPE());
  mimeTypes.push_back(VTK_PARALLEL_POLYDATA_MIMETYPE());
  mimeTypes.push_back(VTK_POLYDATA_LEGACY_MIMETYPE());

  mimeTypes.push_back(STEREOLITHOGRAPHY_MIMETYPE());

  mimeTypes.push_back(RAW_MIMETYPE());
  mimeTypes.push_back(POINTSET_MIMETYPE());
  return mimeTypes;
}

CustomMimeType IOMimeTypes::VTK_IMAGE_MIMETYPE()
{
  CustomMimeType mimeType(VTK_IMAGE_NAME());
  mimeType.AddExtension("vti");
  mimeType.SetCategory("Images");
  mimeType.SetComment("VTK Image");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_IMAGE_LEGACY_MIMETYPE()
{
  CustomMimeType mimeType(VTK_IMAGE_LEGACY_NAME());
  mimeType.AddExtension("vtk");
  mimeType.SetCategory("Images");
  mimeType.SetComment("VTK Legacy Image");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_PARALLEL_IMAGE_MIMETYPE()
{
  CustomMimeType mimeType(VTK_PARALLEL_IMAGE_NAME());
  mimeType.AddExtension("pvti");
  mimeType.SetCategory("Images");
  mimeType.SetComment("VTK Parallel Image");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_POLYDATA_MIMETYPE()
{
  CustomMimeType mimeType(VTK_POLYDATA_NAME());
  mimeType.AddExtension("vtp");
  mimeType.SetCategory("Surfaces");
  mimeType.SetComment("VTK PolyData");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_POLYDATA_LEGACY_MIMETYPE()
{
  CustomMimeType mimeType(VTK_POLYDATA_LEGACY_NAME());
  mimeType.AddExtension("vtk");
  mimeType.SetCategory("Surfaces");
  mimeType.SetComment("VTK Legacy PolyData");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_PARALLEL_POLYDATA_MIMETYPE()
{
  CustomMimeType mimeType(VTK_PARALLEL_POLYDATA_NAME());
  mimeType.AddExtension("pvtp");
  mimeType.SetCategory("Surfaces");
  mimeType.SetComment("VTK Parallel PolyData");
  return mimeType;
}

CustomMimeType IOMimeTypes::STEREOLITHOGRAPHY_MIMETYPE()
{
  CustomMimeType mimeType(STEREOLITHOGRAPHY_NAME());
  mimeType.AddExtension("stl");
  mimeType.SetCategory("Surfaces");
  mimeType.SetComment("Stereolithography");
  return mimeType;
}

std::string IOMimeTypes::STEREOLITHOGRAPHY_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".stl";
  return name;
}

std::string IOMimeTypes::DEFAULT_BASE_NAME()
{
  static std::string name = "application/vnd.mitk";
  return name;
}

std::string IOMimeTypes::VTK_IMAGE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".vtk.image";
  return name;
}

std::string IOMimeTypes::VTK_IMAGE_LEGACY_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".vtk.image.legacy";
  return name;
}

std::string IOMimeTypes::VTK_PARALLEL_IMAGE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".vtk.parallel.image";
  return name;
}

std::string IOMimeTypes::VTK_POLYDATA_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".vtk.polydata";
  return name;
}

std::string IOMimeTypes::VTK_POLYDATA_LEGACY_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".vtk.polydata.legacy";
  return name;
}

std::string IOMimeTypes::VTK_PARALLEL_POLYDATA_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".vtk.parallel.polydata";
  return name;
}

CustomMimeType IOMimeTypes::NRRD_MIMETYPE()
{
  CustomMimeType mimeType(NRRD_MIMETYPE_NAME());
  mimeType.AddExtension("nrrd");
  mimeType.AddExtension("nhdr");
  mimeType.SetCategory("Images");
  mimeType.SetComment("NRRD");
  return mimeType;
}

CustomMimeType IOMimeTypes::NIFTI_MIMETYPE()
{
  CustomMimeType mimeType(NIFTI_MIMETYPE_NAME());
  mimeType.AddExtension("nii");
  mimeType.AddExtension("nii.gz");
  mimeType.AddExtension("hdr");
  mimeType.AddExtension("img");
  mimeType.AddExtension("img.gz");
  mimeType.AddExtension("img.gz");
  mimeType.AddExtension("nia");
  mimeType.SetCategory("Images");
  mimeType.SetComment("Nifti");
  return mimeType;
}

CustomMimeType IOMimeTypes::RAW_MIMETYPE()
{
  CustomMimeType mimeType(RAW_MIMETYPE_NAME());
  mimeType.AddExtension("raw");
  mimeType.SetCategory("Images");
  mimeType.SetComment("Raw data");
  return mimeType;
}

std::string IOMimeTypes::NRRD_MIMETYPE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".image.nrrd";
  return name;
}

std::string IOMimeTypes::NIFTI_MIMETYPE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".image.nifti";
  return name;
}

std::string IOMimeTypes::RAW_MIMETYPE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".image.raw";
  return name;
}

CustomMimeType IOMimeTypes::POINTSET_MIMETYPE()
{
  CustomMimeType mimeType(POINTSET_MIMETYPE_NAME());
  mimeType.AddExtension("mps");
  mimeType.SetCategory("Point Sets");
  mimeType.SetComment("MITK Point Set");
  return mimeType;
}

std::string IOMimeTypes::POINTSET_MIMETYPE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".pointset";
  return name;
}

}
