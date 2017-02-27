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

    mimeTypes.push_back(CEST_DICOM_MIMETYPE().Clone());

    return mimeTypes;
  }

  // Mime Types

  MitkCESTIOMimeTypes::MitkCESTDicomMimeType::MitkCESTDicomMimeType() : CustomMimeType(CEST_DICOM_MIMETYPE_NAME())
  {
    this->AddExtension("gdcm");
    this->AddExtension("dcm");
    this->AddExtension("DCM");
    this->AddExtension("dc3");
    this->AddExtension("DC3");
    this->AddExtension("ima");
    this->AddExtension("img");

    this->SetCategory(IOMimeTypes::CATEGORY_IMAGES());
    this->SetComment("CEST DICOM");
  }

  bool MitkCESTIOMimeTypes::MitkCESTDicomMimeType::AppliesTo(const std::string &path) const
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
    // end fix for bug 18572

    // Ask the GDCM ImageIO class directly
    itk::GDCMImageIO::Pointer gdcmIO = itk::GDCMImageIO::New();
    canRead = gdcmIO->CanReadFile(path.c_str());

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
    mitk::DICOMImageFrameInfo *firstFrame = imageFrameList.begin()->GetPointer();

    std::string byteString = tagCache->GetTagValue(firstFrame, siemensCESTprivateTag).value;

    mitk::CustomTagParser tagParser(relevantFiles[0]);

    auto parsedPropertyList = tagParser.ParseDicomPropertyString(byteString);

    bool mapNotEmpty = parsedPropertyList->GetMap()->size() > 0;

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

  // Names
  std::string MitkCESTIOMimeTypes::CEST_DICOM_MIMETYPE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.cest";
    return name;
  }
}
