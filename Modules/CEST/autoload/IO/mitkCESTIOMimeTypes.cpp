/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkCESTIOMimeTypes.h"
#include "mitkIOMimeTypes.h"
#include <mitkCustomTagParser.h>
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderSelector.h>

#include <mitkLogMacros.h>

#include <itkGDCMImageIO.h>

#include <itksys/SystemTools.hxx>

namespace mitk
{
  std::vector<CustomMimeType *> MitkCESTIOMimeTypes::Get()
  {
    std::vector<CustomMimeType *> mimeTypes;

    // order matters here (descending rank for mime types)

    mimeTypes.push_back(CEST_DICOM_WITH_META_FILE_MIMETYPE().Clone());
    mimeTypes.push_back(CEST_DICOM_MIMETYPE().Clone());
    mimeTypes.push_back(CEST_DICOM_WITHOUT_META_FILE_MIMETYPE().Clone());

    return mimeTypes;
  }

  // Mime Types

  MitkCESTIOMimeTypes::MitkCESTDicomMimeType::MitkCESTDicomMimeType() : IOMimeTypes::BaseDicomMimeType(CEST_DICOM_MIMETYPE_NAME())
  {
    this->SetCategory(IOMimeTypes::CATEGORY_IMAGES());
    this->SetComment("CEST DICOM");
  }

  bool MitkCESTIOMimeTypes::MitkCESTDicomMimeType::AppliesTo(const std::string &path) const
  {
    bool canRead(IOMimeTypes::BaseDicomMimeType::AppliesTo(path));

    // fix for bug 18572
    // Currently this function is called for writing as well as reading, in that case
    // the image information can of course not be read
    // This is a bug, this function should only be called for reading.
    if (!itksys::SystemTools::FileExists(path.c_str()))
    {
      return canRead;
    }
    // end fix for bug 18572

    if (!canRead)
    {
      return canRead;
    }

    mitk::DICOMDCMTKTagScanner::Pointer scanner = mitk::DICOMDCMTKTagScanner::New();

    mitk::DICOMTag siemensCESTprivateTag(0x0029, 0x1020);

    mitk::StringList relevantFiles;
    relevantFiles.push_back(path);

    scanner->AddTag(siemensCESTprivateTag);
    scanner->SetInputFiles(relevantFiles);
    scanner->Scan();
    mitk::DICOMTagCache::Pointer tagCache = scanner->GetScanCache();

    mitk::DICOMImageFrameList imageFrameList = mitk::ConvertToDICOMImageFrameList(tagCache->GetFrameInfoList());

    bool mapNotEmpty = false;

    if (!imageFrameList.empty())
    {
      mitk::DICOMImageFrameInfo* firstFrame = imageFrameList.begin()->GetPointer();

      std::string byteString = tagCache->GetTagValue(firstFrame, siemensCESTprivateTag).value;

      if (byteString.empty()) {
        return false;
      }
      mitk::CustomTagParser tagParser(relevantFiles[0]);

      auto parsedPropertyList = tagParser.ParseDicomPropertyString(byteString);

      mapNotEmpty = !parsedPropertyList->GetMap()->empty();
    }

    return mapNotEmpty;
  }

  MitkCESTIOMimeTypes::MitkCESTDicomMimeType *MitkCESTIOMimeTypes::MitkCESTDicomMimeType::Clone() const
  {
    return new MitkCESTDicomMimeType(*this);
  }

  MitkCESTIOMimeTypes::MitkCESTDicomMimeType MitkCESTIOMimeTypes::CEST_DICOM_MIMETYPE()
  {
    return MitkCESTDicomMimeType();
  }

  std::string MitkCESTIOMimeTypes::CEST_DICOM_MIMETYPE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.cest";
    return name;
  }

  MitkCESTIOMimeTypes::MitkCESTDicomWithMetaFileMimeType::MitkCESTDicomWithMetaFileMimeType() : IOMimeTypes::BaseDicomMimeType(CEST_DICOM_WITH_META_FILE_NAME())
  {
    this->SetCategory(IOMimeTypes::CATEGORY_IMAGES());
    this->SetComment("CEST DICOM");
  }

  bool MitkCESTIOMimeTypes::MitkCESTDicomWithMetaFileMimeType::AppliesTo(const std::string& path) const
  {
    bool canRead(IOMimeTypes::BaseDicomMimeType::AppliesTo(path));

    // fix for bug 18572
    // Currently this function is called for writing as well as reading, in that case
    // the image information can of course not be read
    // This is a bug, this function should only be called for reading.
    if (!itksys::SystemTools::FileExists(path.c_str()))
    {
      return canRead;
    }
    // end fix for bug 18572

    if (!canRead)
    {
      return canRead;
    }

    std::string dir = path;
    if (!itksys::SystemTools::FileIsDirectory(path))
    {
      dir = itksys::SystemTools::GetProgramPath(path);
    }

    std::string metafilePath = dir +"/" + "CEST_META.json";

    canRead = itksys::SystemTools::FileExists(metafilePath.c_str());

    return canRead;
  }

  MitkCESTIOMimeTypes::MitkCESTDicomWithMetaFileMimeType* MitkCESTIOMimeTypes::MitkCESTDicomWithMetaFileMimeType::Clone() const
  {
    return new MitkCESTDicomWithMetaFileMimeType(*this);
  }

  MitkCESTIOMimeTypes::MitkCESTDicomWithMetaFileMimeType MitkCESTIOMimeTypes::CEST_DICOM_WITH_META_FILE_MIMETYPE()
  {
    return MitkCESTDicomWithMetaFileMimeType();
  }

  std::string MitkCESTIOMimeTypes::CEST_DICOM_WITH_META_FILE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.cest.generic.meta";
    return name;
  }

  MitkCESTIOMimeTypes::MitkCESTDicomWOMetaFileMimeType::MitkCESTDicomWOMetaFileMimeType() : IOMimeTypes::BaseDicomMimeType(CEST_DICOM_WITHOUT_META_FILE_NAME())
  {
    this->SetCategory(IOMimeTypes::CATEGORY_IMAGES());
    this->SetComment("CEST DICOM");
  }

  bool MitkCESTIOMimeTypes::MitkCESTDicomWOMetaFileMimeType::AppliesTo(const std::string& path) const
  {
    bool canRead(IOMimeTypes::BaseDicomMimeType::AppliesTo(path));

    // fix for bug 18572
    // Currently this function is called for writing as well as reading, in that case
    // the image information can of course not be read
    // This is a bug, this function should only be called for reading.
    if (!itksys::SystemTools::FileExists(path.c_str()))
    {
      return canRead;
    }
    // end fix for bug 18572

    if (!canRead)
    {
      return canRead;
    }

    std::string dir = path;
    if (!itksys::SystemTools::FileIsDirectory(path))
    {
      dir = itksys::SystemTools::GetProgramPath(path);
    }

    std::string metafilePath = dir + "/" + "CEST_META.json";

    canRead = !itksys::SystemTools::FileExists(metafilePath.c_str());

    return canRead;
  }

  MitkCESTIOMimeTypes::MitkCESTDicomWOMetaFileMimeType* MitkCESTIOMimeTypes::MitkCESTDicomWOMetaFileMimeType::Clone() const
  {
    return new MitkCESTDicomWOMetaFileMimeType(*this);
  }

  MitkCESTIOMimeTypes::MitkCESTDicomWOMetaFileMimeType MitkCESTIOMimeTypes::CEST_DICOM_WITHOUT_META_FILE_MIMETYPE()
  {
    return MitkCESTDicomWOMetaFileMimeType();
  }

  std::string MitkCESTIOMimeTypes::CEST_DICOM_WITHOUT_META_FILE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.cest.generic.nometa";
    return name;
  }

}
