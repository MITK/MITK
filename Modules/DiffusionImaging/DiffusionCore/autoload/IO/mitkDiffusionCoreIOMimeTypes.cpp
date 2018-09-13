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
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkCustomMimeType.h>
#include <itkGDCMImageIO.h>
#include <itkNiftiImageIO.h>

namespace mitk
{

std::vector<CustomMimeType*> DiffusionCoreIOMimeTypes::Get()
{
  std::vector<CustomMimeType*> mimeTypes;

  // order matters here (descending rank for mime types)

  mimeTypes.push_back(DWI_NRRD_MIMETYPE().Clone());
  mimeTypes.push_back(DWI_NIFTI_MIMETYPE().Clone());
  mimeTypes.push_back(DWI_FSL_MIMETYPE().Clone());
  mimeTypes.push_back(DWI_DICOM_MIMETYPE().Clone());
  mimeTypes.push_back(DTI_MIMETYPE().Clone());
  mimeTypes.push_back(ODF_MIMETYPE().Clone());
  mimeTypes.push_back(PEAK_MIMETYPE().Clone());
  mimeTypes.push_back(SH_MIMETYPE().Clone());

  return mimeTypes;
}

// Mime Types

DiffusionCoreIOMimeTypes::DiffusionImageNrrdMimeType::DiffusionImageNrrdMimeType()
  : CustomMimeType(DWI_NRRD_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Images";
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

  itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
  // Simple NRRD files should only be considered for this mime type if they contain
  // corresponding tags
  if( io->CanReadFile(path.c_str()))
  {
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
  std::string category = "Diffusion Weighted Images";
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
    std::string base_path = itksys::SystemTools::GetFilenamePath(path);
    std::string base = this->GetFilenameWithoutExtension(path);
    std::string filename = base;
    if (!base_path.empty())
    {
        base = base_path + "/" + base;
        base_path += "/";
    }

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

    // hack for HCP data
    if ( filename=="data" && itksys::SystemTools::FileExists(std::string(base_path + "bvec").c_str()) && itksys::SystemTools::FileExists(std::string(base_path + "bval").c_str()) )
    {
        return canRead;
    }

    if ( filename=="data" && itksys::SystemTools::FileExists(std::string(base_path + "bvecs").c_str()) && itksys::SystemTools::FileExists(std::string(base_path + "bvals").c_str()) )
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
  std::string category = "Diffusion Weighted Images";
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
  if (ext == ".fsl" || ext == ".fslgz")
  {
      std::string base_path = itksys::SystemTools::GetFilenamePath(path);
      std::string base = this->GetFilenameWithoutExtension(path);
      if (!base_path.empty())
          base = base_path + "/" + base;

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

    if (itksys::SystemTools::FileExists(std::string(base + ext + ".bvec").c_str())
      && itksys::SystemTools::FileExists(std::string(base + ext + ".bval").c_str())
      )
    {
      return canRead;
    }

    if (itksys::SystemTools::FileExists(std::string(base + ext + ".bvecs").c_str())
      && itksys::SystemTools::FileExists(std::string(base + ext + ".bvals").c_str())
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


DiffusionCoreIOMimeTypes::DiffusionImageDicomMimeType::DiffusionImageDicomMimeType()
  : CustomMimeType(DWI_DICOM_MIMETYPE_NAME())
{
  std::string category = "Diffusion Weighted Images";
  this->SetCategory(category);
  this->SetComment("Diffusion Weighted Images");

  this->AddExtension("gdcm");
  this->AddExtension("dcm");
  this->AddExtension("DCM");
  this->AddExtension("dc3");
  this->AddExtension("DC3");
  this->AddExtension("ima");
  this->AddExtension("img");
}

bool DiffusionCoreIOMimeTypes::DiffusionImageDicomMimeType::AppliesTo(const std::string &path) const
{
  itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
  bool canRead = gdcmIO->CanReadFile(path.c_str());

  if (!canRead)
    return canRead;

  mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();
  mitk::DICOMTag ImageTypeTag(0x0008, 0x0008);
  mitk::DICOMTag SeriesDescriptionTag(0x0008, 0x103E);

  mitk::StringList relevantFiles;
  relevantFiles.push_back(path);

  scanner->AddTag(ImageTypeTag);
  scanner->AddTag(SeriesDescriptionTag);
  scanner->SetInputFiles(relevantFiles);
  scanner->Scan();
  mitk::DICOMTagCache::Pointer tagCache = scanner->GetScanCache();

  mitk::DICOMImageFrameList imageFrameList = mitk::ConvertToDICOMImageFrameList(tagCache->GetFrameInfoList());
  mitk::DICOMImageFrameInfo *firstFrame = imageFrameList.begin()->GetPointer();

  std::string byteString = tagCache->GetTagValue(firstFrame, ImageTypeTag).value;
  if (byteString.empty())
    return false;

  std::string byteString2 = tagCache->GetTagValue(firstFrame, SeriesDescriptionTag).value;
  if (byteString2.empty())
    return false;

  if (byteString.find("DIFFUSION")==std::string::npos && byteString2.find("diff")==std::string::npos)
    return false;
//  if (byteString.find("NONE")==std::string::npos)
//    return false;

  return canRead;
}

DiffusionCoreIOMimeTypes::DiffusionImageDicomMimeType* DiffusionCoreIOMimeTypes::DiffusionImageDicomMimeType::Clone() const
{
  return new DiffusionImageDicomMimeType(*this);
}


DiffusionCoreIOMimeTypes::DiffusionImageDicomMimeType DiffusionCoreIOMimeTypes::DWI_DICOM_MIMETYPE()
{
  return DiffusionImageDicomMimeType();
}


DiffusionCoreIOMimeTypes::PeakImageMimeType::PeakImageMimeType() : CustomMimeType(PEAK_MIMETYPE_NAME())
{
  std::string category = "Peak Image";
  this->SetCategory(category);
  this->SetComment("Peak Image");

  this->AddExtension("nrrd");
  this->AddExtension("nii");
  this->AddExtension("nii.gz");
  this->AddExtension("peak");
}

bool DiffusionCoreIOMimeTypes::PeakImageMimeType::AppliesTo(const std::string &path) const
{
  std::string ext = itksys::SystemTools::GetFilenameExtension(path);
  if (ext==".peak")
    return true;

  try
  {
    itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
    if ( io->CanReadFile( path.c_str() ) )
    {
      io->SetFileName( path.c_str() );
      io->ReadImageInformation();
      if ( io->GetPixelType() == itk::ImageIOBase::SCALAR && io->GetNumberOfDimensions()==4 && io->GetDimensions(3)%3==0)
        return true;
    }
  }
  catch(...)
  {}

  try
  {
    itk::NiftiImageIO::Pointer io = itk::NiftiImageIO::New();
    if ( io->CanReadFile( path.c_str() ) )
    {
      io->SetFileName( path.c_str() );
      io->ReadImageInformation();
      if ( io->GetPixelType() == itk::ImageIOBase::SCALAR && io->GetNumberOfDimensions()==4 && io->GetDimensions(3)%3==0)
        return true;
    }
  }
  catch(...)
  {}

  return false;
}

DiffusionCoreIOMimeTypes::PeakImageMimeType* DiffusionCoreIOMimeTypes::PeakImageMimeType::Clone() const
{
  return new PeakImageMimeType(*this);
}


DiffusionCoreIOMimeTypes::PeakImageMimeType DiffusionCoreIOMimeTypes::PEAK_MIMETYPE()
{
  return PeakImageMimeType();
}


DiffusionCoreIOMimeTypes::SHImageMimeType::SHImageMimeType() : CustomMimeType(SH_MIMETYPE_NAME())
{
  std::string category = "SH Image";
  this->SetCategory(category);
  this->SetComment("SH Image");

  this->AddExtension("nii.gz");
  this->AddExtension("nii");
  this->AddExtension("nrrd");
  this->AddExtension("shi");
}

bool DiffusionCoreIOMimeTypes::SHImageMimeType::AppliesTo(const std::string &path) const
{
  std::string ext = itksys::SystemTools::GetFilenameExtension(path);
  if (ext==".shi")
    return true;

  {
    try
    {
      itk::NrrdImageIO::Pointer io = itk::NrrdImageIO::New();
      if (io->CanReadFile(path.c_str()))
      {
        io->SetFileName(path.c_str());
        io->ReadImageInformation();
        if (io->GetPixelType() == itk::ImageIOBase::SCALAR && io->GetNumberOfDimensions() == 4)
        {
          switch (io->GetDimensions(3))
          {
          case 6:
            return true;
            break;
          case 15:
            return true;
            break;
          case 28:
            return true;
            break;
          case 45:
            return true;
            break;
          case 66:
            return true;
            break;
          case 91:
            return true;
            break;
          default:
            return false;
          }
        }
      }
    }
    catch(...)
    {}
  }

  {
    itk::NiftiImageIO::Pointer io = itk::NiftiImageIO::New();
    if ( io->CanReadFile( path.c_str() ) )
    {
      io->SetFileName( path.c_str() );
      io->ReadImageInformation();
      if ( io->GetPixelType() == itk::ImageIOBase::SCALAR && io->GetNumberOfDimensions()==4)
      {
        switch (io->GetDimensions(3))
        {
        case 6:
          return true;
          break;
        case 15:
          return true;
          break;
        case 28:
          return true;
          break;
        case 45:
          return true;
          break;
        case 66:
          return true;
          break;
        case 91:
          return true;
          break;
        default :
          return false;
        }
      }
    }
  }

  return false;
}

DiffusionCoreIOMimeTypes::SHImageMimeType* DiffusionCoreIOMimeTypes::SHImageMimeType::Clone() const
{
  return new SHImageMimeType(*this);
}


DiffusionCoreIOMimeTypes::SHImageMimeType DiffusionCoreIOMimeTypes::SH_MIMETYPE()
{
  return SHImageMimeType();
}

CustomMimeType DiffusionCoreIOMimeTypes::DTI_MIMETYPE()
{
  CustomMimeType mimeType(DTI_MIMETYPE_NAME());
  std::string category = "Tensor Image";
  mimeType.SetComment("Diffusion Tensor Image");
  mimeType.SetCategory(category);
  mimeType.AddExtension("dti");
  return mimeType;
}

CustomMimeType DiffusionCoreIOMimeTypes::ODF_MIMETYPE()
{
  CustomMimeType mimeType(ODF_MIMETYPE_NAME());
  std::string category = "ODF Image";
  mimeType.SetComment("Diffusion ODF Image");
  mimeType.SetCategory(category);
  mimeType.AddExtension("odf");
  mimeType.AddExtension("qbi"); // legacy support
  return mimeType;
}

// Names
std::string DiffusionCoreIOMimeTypes::PEAK_MIMETYPE_NAME()
{
  static std::string name ="ODF_PEAKS";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DWI_NRRD_MIMETYPE_NAME()
{
  static std::string name = "DWI_NRRD";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DWI_NIFTI_MIMETYPE_NAME()
{
  static std::string name = "DWI_NIFTI";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DWI_FSL_MIMETYPE_NAME()
{
  static std::string name = "DWI_FSL";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DWI_DICOM_MIMETYPE_NAME()
{
  static std::string name = "DWI_DICOM";
  return name;
}

std::string DiffusionCoreIOMimeTypes::DTI_MIMETYPE_NAME()
{
  static std::string name = "DT_IMAGE";
  return name;
}

std::string DiffusionCoreIOMimeTypes::ODF_MIMETYPE_NAME()
{
  static std::string name = "ODF_IMAGE";
  return name;
}

std::string DiffusionCoreIOMimeTypes::SH_MIMETYPE_NAME()
{
  static std::string name = "SH_IMAGE";
  return name;
}

// Descriptions
std::string DiffusionCoreIOMimeTypes::PEAK_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Peak Image";
  return description;
}

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

std::string DiffusionCoreIOMimeTypes::DWI_DICOM_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Weighted Images";
  return description;
}

std::string DiffusionCoreIOMimeTypes::DTI_MIMETYPE_DESCRIPTION()
{
  static std::string description = "Diffusion Tensor Image";
  return description;
}

std::string DiffusionCoreIOMimeTypes::ODF_MIMETYPE_DESCRIPTION()
{
  static std::string description = "ODF Image";
  return description;
}

std::string DiffusionCoreIOMimeTypes::SH_MIMETYPE_DESCRIPTION()
{
  static std::string description = "SH Image";
  return description;
}

}
