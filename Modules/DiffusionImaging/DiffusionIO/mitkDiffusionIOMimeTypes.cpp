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
#include <dcmtk/dcmtract/trctractographyresults.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <itkGDCMImageIO.h>

namespace mitk
{

std::vector<CustomMimeType*> DiffusionIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(FIBERBUNDLE_VTK_MIMETYPE().Clone());
  mimeTypes.push_back(FIBERBUNDLE_TRK_MIMETYPE().Clone());
  mimeTypes.push_back(FIBERBUNDLE_TCK_MIMETYPE().Clone());
  mimeTypes.push_back(FIBERBUNDLE_DICOM_MIMETYPE().Clone());

  mimeTypes.push_back(CONNECTOMICS_MIMETYPE().Clone());
  mimeTypes.push_back(TRACTOGRAPHYFOREST_MIMETYPE().Clone());

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


CustomMimeType DiffusionIOMimeTypes::TRACTOGRAPHYFOREST_MIMETYPE()
{
  CustomMimeType mimeType(TRACTOGRAPHYFOREST_MIMETYPE_NAME());
  std::string category = "Tractography Forest";
  mimeType.SetComment("Tractography Forest");
  mimeType.SetCategory(category);
  mimeType.AddExtension("rf");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_VTK_MIMETYPE_NAME());
  std::string category = "VTK Fibers";
  mimeType.SetComment("VTK Fibers");
  mimeType.SetCategory(category);
  mimeType.AddExtension("fib");
  mimeType.AddExtension("vtk");
  return mimeType;
}

CustomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_TCK_MIMETYPE()
{
  CustomMimeType mimeType(FIBERBUNDLE_TCK_MIMETYPE_NAME());
  std::string category = "MRtrix Fibers";
  mimeType.SetComment("MRtrix Fibers");
  mimeType.SetCategory(category);
  mimeType.AddExtension("tck");
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

DiffusionIOMimeTypes::FiberBundleDicomMimeType::FiberBundleDicomMimeType()
  : CustomMimeType(FIBERBUNDLE_DICOM_MIMETYPE_NAME())
{
  std::string category = "DICOM Fibers";
  this->SetCategory(category);
  this->SetComment("DICOM Fibers");

  this->AddExtension("dcm");
  this->AddExtension("DCM");
  this->AddExtension("gdcm");
  this->AddExtension("dc3");
  this->AddExtension("DC3");
  this->AddExtension("ima");
  this->AddExtension("img");
}

bool DiffusionIOMimeTypes::FiberBundleDicomMimeType::AppliesTo(const std::string &path) const
{
  try
  {
    std::ifstream myfile;
    myfile.open (path, std::ios::binary);
//    myfile.seekg (128);
    char *buffer = new char [128];
    myfile.read (buffer,128);
    myfile.read (buffer,4);
    if (std::string(buffer).compare("DICM")!=0)
    {
      delete[] buffer;
      return false;
    }
    delete[] buffer;

    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
    mitk::DICOMTag SOPInstanceUID(0x0008, 0x0016);

    mitk::StringList relevantFiles;
    relevantFiles.push_back(path);

    scanner->AddTag(SOPInstanceUID);
    scanner->SetInputFiles(relevantFiles);
    scanner->Scan();
    mitk::DICOMTagCache::Pointer tagCache = scanner->GetScanCache();

    mitk::DICOMImageFrameList imageFrameList = mitk::ConvertToDICOMImageFrameList(tagCache->GetFrameInfoList());
    if (imageFrameList.empty())
      return false;

    mitk::DICOMImageFrameInfo *firstFrame = imageFrameList.begin()->GetPointer();

    std::string tag_value = tagCache->GetTagValue(firstFrame, SOPInstanceUID).value;
    if (tag_value.empty()) {
      return false;
    }

    if (tag_value.compare(UID_TractographyResultsStorage)!=0)
      return false;

    return true;
  }
  catch (std::exception& e)
  {
    MITK_INFO << e.what();
  }
  return false;
}

DiffusionIOMimeTypes::FiberBundleDicomMimeType* DiffusionIOMimeTypes::FiberBundleDicomMimeType::Clone() const
{
  return new FiberBundleDicomMimeType(*this);
}


DiffusionIOMimeTypes::FiberBundleDicomMimeType DiffusionIOMimeTypes::FIBERBUNDLE_DICOM_MIMETYPE()
{
  return FiberBundleDicomMimeType();
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
std::string DiffusionIOMimeTypes::FIBERBUNDLE_VTK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.vtk";
  return name;
}

std::string DiffusionIOMimeTypes::FIBERBUNDLE_TCK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.tck";
  return name;
}

std::string DiffusionIOMimeTypes::FIBERBUNDLE_TRK_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.trk";
  return name;
}

std::string DiffusionIOMimeTypes::FIBERBUNDLE_DICOM_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".FiberBundle.dcm";
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

std::string DiffusionIOMimeTypes::TRACTOGRAPHYFOREST_MIMETYPE_NAME()
{
  static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".rf";
  return name;
}

// Descriptions
std::string DiffusionIOMimeTypes::FIBERBUNDLE_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Fiberbundles";
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

std::string DiffusionIOMimeTypes::TRACTOGRAPHYFOREST_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Tractography Forest";
  return description;
}

}
