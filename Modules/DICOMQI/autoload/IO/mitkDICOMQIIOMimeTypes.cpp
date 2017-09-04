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

#include "mitkDICOMQIIOMimeTypes.h"
#include "mitkIOMimeTypes.h"
#include <mitkDICOMDCMTKTagScanner.h>
#include <mitkDICOMFileReaderSelector.h>

#include <mitkLogMacros.h>

#include <itkGDCMImageIO.h>

#include <itksys/SystemTools.hxx>

// dcmqi
#include <dcmqi/ImageSEGConverter.h>
#include <dcmqi/JSONSegmentationMetaInformationHandler.h>

namespace mitk
{
  std::vector<CustomMimeType *> MitkDICOMQIIOMimeTypes::Get()
  {
    std::vector<CustomMimeType *> mimeTypes;

    // order matters here (descending rank for mime types)

    mimeTypes.push_back(DICOMQI_MIMETYPE().Clone());

    return mimeTypes;
  }

  // Mime Types

  MitkDICOMQIIOMimeTypes::MitkDICOMQIMimeType::MitkDICOMQIMimeType() : CustomMimeType(DICOMQI_MIMETYPE_NAME())
  {
    this->AddExtension("gdcm");
    this->AddExtension("dcm");
    this->AddExtension("DCM");
    this->AddExtension("dc3");
    this->AddExtension("DC3");
    this->AddExtension("ima");
    this->AddExtension("img");

    this->SetCategory(IOMimeTypes::CATEGORY_IMAGES());
    this->SetComment("DCMQI");
  }

  bool MitkDICOMQIIOMimeTypes::MitkDICOMQIMimeType::AppliesTo(const std::string &path) const
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
    if (dcmFileFormat.getDataset()->findAndGetOFString(DCM_Modality, modality).good())
    {
      if (modality.compare("SEG") == 0)
      {
        canRead = true;
      }
      else
      {
        canRead = false;
      }
    }

    return canRead;
  }

  MitkDICOMQIIOMimeTypes::MitkDICOMQIMimeType *MitkDICOMQIIOMimeTypes::MitkDICOMQIMimeType::Clone() const
  {
    return new MitkDICOMQIMimeType(*this);
  }

  MitkDICOMQIIOMimeTypes::MitkDICOMQIMimeType MitkDICOMQIIOMimeTypes::DICOMQI_MIMETYPE()
  {
    return MitkDICOMQIMimeType();
  }

  // Names
  std::string MitkDICOMQIIOMimeTypes::DICOMQI_MIMETYPE_NAME()
  {
    // create a unique and sensible name for this mime type
    static std::string name = IOMimeTypes::DEFAULT_BASE_NAME() + ".image.dicom.seg";
    return name;
  }
}
