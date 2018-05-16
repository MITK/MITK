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

#ifndef mitkDICOMDatasetAccessingImageFrameInfo_h
#define mitkDICOMDatasetAccessingImageFrameInfo_h

#include "mitkDICOMImageFrameInfo.h"
#include "mitkDICOMDatasetAccess.h"

#include "MitkDICOMReaderExports.h"

namespace mitk
{
  /**
    \ingroup DICOMReaderModule
    \brief Defines an abstract base class for DICOM image frame infos with data access.

    This abstract base class extends the DICOMImageFrameInfo by the DICOMDatasetAccess interface.
    This allows to directly query for tag values of a frame/file specified by the info.
    DICOMGDCMImageFrameInfo is an example for a concrete implementation.
  */
  class MITKDICOMREADER_EXPORT DICOMDatasetAccessingImageFrameInfo : public DICOMImageFrameInfo, public DICOMDatasetAccess
  {
    public:
      mitkClassMacro(DICOMDatasetAccessingImageFrameInfo, DICOMImageFrameInfo);

  protected:
      DICOMDatasetAccessingImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);
      ~DICOMDatasetAccessingImageFrameInfo() override;

    private:
      DICOMDatasetAccessingImageFrameInfo(const DICOMDatasetAccessingImageFrameInfo::Pointer& frameinfo);
      Self& operator = (const Self& frameinfo);
  };


  typedef std::vector<DICOMDatasetAccessingImageFrameInfo::Pointer> DICOMDatasetAccessingImageFrameList;

  MITKDICOMREADER_EXPORT mitk::DICOMImageFrameList
    ConvertToDICOMImageFrameList(const DICOMDatasetAccessingImageFrameList& input);

  MITKDICOMREADER_EXPORT mitk::DICOMDatasetList
    ConvertToDICOMDatasetList(const DICOMDatasetAccessingImageFrameList& input);

  MITKDICOMREADER_EXPORT mitk::DICOMDatasetAccessingImageFrameList
    ConvertToDICOMDatasetAccessingImageFrameList(const DICOMDatasetList& input);

}

#endif
