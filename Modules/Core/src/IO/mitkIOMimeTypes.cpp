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

#include "itkGDCMImageIO.h"

namespace mitk {

IOMimeTypes::DicomMimeType::DicomMimeType()
  : CustomMimeType(DICOM_MIMETYPE_NAME())
{
  this->AddExtension("gdcm");
  this->AddExtension("dcm");
  this->AddExtension("DCM");
  this->AddExtension("dc3");
  this->AddExtension("DC3");
  this->AddExtension("ima");
  this->AddExtension("img");

  this->SetCategory(CATEGORY_IMAGES());
  this->SetComment("DICOM");
}

bool IOMimeTypes::DicomMimeType::AppliesTo(const std::string &path) const
{
  if (CustomMimeType::AppliesTo(path)) return true;
  // Ask the GDCM ImageIO class directly
  itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
  return gdcmIO->CanReadFile(path.c_str());
}

IOMimeTypes::DicomMimeType* IOMimeTypes::DicomMimeType::Clone() const
{
  return new DicomMimeType(*this);
}

std::vector<CustomMimeType*> IOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(NRRD_MIMETYPE().Clone());
  mimeTypes.push_back(NIFTI_MIMETYPE().Clone());

  mimeTypes.push_back(VTK_IMAGE_MIMETYPE().Clone());
  mimeTypes.push_back(VTK_PARALLEL_IMAGE_MIMETYPE().Clone());
  mimeTypes.push_back(VTK_IMAGE_LEGACY_MIMETYPE().Clone());

  mimeTypes.push_back(DICOM_MIMETYPE().Clone());

  mimeTypes.push_back(VTK_POLYDATA_MIMETYPE().Clone());
  mimeTypes.push_back(VTK_PARALLEL_POLYDATA_MIMETYPE().Clone());
  mimeTypes.push_back(VTK_POLYDATA_LEGACY_MIMETYPE().Clone());

  mimeTypes.push_back(STEREOLITHOGRAPHY_MIMETYPE().Clone());

  mimeTypes.push_back(RAW_MIMETYPE().Clone());
  mimeTypes.push_back(POINTSET_MIMETYPE().Clone());
  return mimeTypes;
}

CustomMimeType IOMimeTypes::VTK_IMAGE_MIMETYPE()
{
  CustomMimeType mimeType(VTK_IMAGE_NAME());
  mimeType.AddExtension("vti");
  mimeType.SetCategory(CATEGORY_IMAGES());
  mimeType.SetComment("VTK Image");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_IMAGE_LEGACY_MIMETYPE()
{
  CustomMimeType mimeType(VTK_IMAGE_LEGACY_NAME());
  mimeType.AddExtension("vtk");
  mimeType.SetCategory(CATEGORY_IMAGES());
  mimeType.SetComment("VTK Legacy Image");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_PARALLEL_IMAGE_MIMETYPE()
{
  CustomMimeType mimeType(VTK_PARALLEL_IMAGE_NAME());
  mimeType.AddExtension("pvti");
  mimeType.SetCategory(CATEGORY_IMAGES());
  mimeType.SetComment("VTK Parallel Image");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_POLYDATA_MIMETYPE()
{
  CustomMimeType mimeType(VTK_POLYDATA_NAME());
  mimeType.AddExtension("vtp");
  mimeType.SetCategory(CATEGORY_SURFACES());
  mimeType.SetComment("VTK PolyData");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_POLYDATA_LEGACY_MIMETYPE()
{
  CustomMimeType mimeType(VTK_POLYDATA_LEGACY_NAME());
  mimeType.AddExtension("vtk");
  mimeType.SetCategory(CATEGORY_SURFACES());
  mimeType.SetComment("VTK Legacy PolyData");
  return mimeType;
}

CustomMimeType IOMimeTypes::VTK_PARALLEL_POLYDATA_MIMETYPE()
{
  CustomMimeType mimeType(VTK_PARALLEL_POLYDATA_NAME());
  mimeType.AddExtension("pvtp");
  mimeType.SetCategory(CATEGORY_SURFACES());
  mimeType.SetComment("VTK Parallel PolyData");
  return mimeType;
}

CustomMimeType IOMimeTypes::STEREOLITHOGRAPHY_MIMETYPE()
{
  CustomMimeType mimeType(STEREOLITHOGRAPHY_NAME());
  mimeType.AddExtension("stl");
  mimeType.SetCategory(CATEGORY_SURFACES());
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

std::string IOMimeTypes::CATEGORY_IMAGES()
{
  static std::string cat = "Images";
  return cat;
}

std::string IOMimeTypes::CATEGORY_SURFACES()
{
  static std::string cat = "Surfaces";
  return cat;
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

IOMimeTypes::DicomMimeType IOMimeTypes::DICOM_MIMETYPE()
{
  return DicomMimeType();
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

std::string IOMimeTypes::DICOM_MIMETYPE_NAME()
{
  static std::string name = DEFAULT_BASE_NAME() + ".image.dicom";
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
