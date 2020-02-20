/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkDICOMSegIOMimeTypes.h"
#include "mitkIOMimeTypes.h"

#include <array>

#include <mitkLogMacros.h>

#include <itkGDCMImageIO.h>
#include <itksys/SystemTools.hxx>

#include <dcmtk/dcmdata/dcfilefo.h>
#include <dcmtk/dcmdata/dcdeftag.h>

namespace mitk
{
  std::vector<CustomMimeType *> MitkDICOMSEGIOMimeTypes::Get()
  {
    std::vector<CustomMimeType *> mimeTypes;

    // order matters here (descending rank for mime types)

    mimeTypes.push_back(DICOMSEG_MIMETYPE().Clone());
    return mimeTypes;
  }


  MitkDICOMSEGIOMimeTypes::MitkDICOMSEGMimeType::MitkDICOMSEGMimeType() : CustomMimeType(DICOMSEG_MIMETYPE_NAME())
  {
    this->AddExtension("dcm");
    this->SetCategory(IOMimeTypes::CATEGORY_IMAGES());
    this->SetComment("DICOM SEG");
  }


  bool MitkDICOMSEGIOMimeTypes::MitkDICOMSEGMimeType::AppliesTo(const std::string &path) const
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

    const std::size_t offset = 128;
    const std::size_t bufferSize = 4;

    std::ifstream myfile(path, std::ifstream::binary);

    if (!myfile.is_open())
      return false;

    myfile.seekg(0, std::ifstream::end);
    const auto fileSize = static_cast<std::size_t>(myfile.tellg());

    if (fileSize < offset + bufferSize)
      return false;

    myfile.seekg(offset);
    std::array<char, bufferSize> buffer;
    myfile.read(buffer.data(), bufferSize);

    if (0 != std::string(buffer.data(), bufferSize).compare("DICM"))
      return false;

    DcmFileFormat dcmFileFormat;
    OFCondition status = dcmFileFormat.loadFile(path.c_str());

    if (status.bad())
    {
      canRead = false;
    }

    if (!canRead)
    {
      return canRead;
    }

    OFString modality;
    OFString sopClassUID;
    if (dcmFileFormat.getDataset()->findAndGetOFString(DCM_Modality, modality).good() && dcmFileFormat.getDataset()->findAndGetOFString(DCM_SOPClassUID, sopClassUID).good())
    {
      if (modality.compare("SEG") == 0)
      {//atm we could read SegmentationStorage files. Other storage classes with "SEG" modality, e.g. SurfaceSegmentationStorage (1.2.840.10008.5.1.4.1.1.66.5), are not supported yet.
        if (sopClassUID.compare("1.2.840.10008.5.1.4.1.1.66.4") == 0)
        {
          canRead = true;
        }
        else
        {
          canRead = false;
        }
      }
      else
      {
        canRead = false;
      }
    }

    return canRead;
  }

  MitkDICOMSEGIOMimeTypes::MitkDICOMSEGMimeType *MitkDICOMSEGIOMimeTypes::MitkDICOMSEGMimeType::Clone() const
  {
    return new MitkDICOMSEGMimeType(*this);
  }

  MitkDICOMSEGIOMimeTypes::MitkDICOMSEGMimeType MitkDICOMSEGIOMimeTypes::DICOMSEG_MIMETYPE()
  {
    return MitkDICOMSEGMimeType();
  }

  std::string MitkDICOMSEGIOMimeTypes::DICOMSEG_MIMETYPE_NAME()
  {
    // create a unique and sensible name for this mime type
    return IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.seg";
  }
}
