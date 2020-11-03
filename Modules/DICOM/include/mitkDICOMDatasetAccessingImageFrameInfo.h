/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMDatasetAccessingImageFrameInfo_h
#define mitkDICOMDatasetAccessingImageFrameInfo_h

#include "mitkDICOMImageFrameInfo.h"
#include "mitkDICOMDatasetAccess.h"

#include "MitkDICOMExports.h"

namespace mitk
{
  /**
    \ingroup DICOMModule
    \brief Defines an abstract base class for DICOM image frame infos with data access.

    This abstract base class extends the DICOMImageFrameInfo by the DICOMDatasetAccess interface.
    This allows to directly query for tag values of a frame/file specified by the info.
    DICOMGDCMImageFrameInfo is an example for a concrete implementation.
  */
  class MITKDICOM_EXPORT DICOMDatasetAccessingImageFrameInfo : public DICOMImageFrameInfo, public DICOMDatasetAccess
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

  MITKDICOM_EXPORT mitk::DICOMImageFrameList
    ConvertToDICOMImageFrameList(const DICOMDatasetAccessingImageFrameList& input);

  MITKDICOM_EXPORT mitk::DICOMDatasetList
    ConvertToDICOMDatasetList(const DICOMDatasetAccessingImageFrameList& input);

  MITKDICOM_EXPORT mitk::DICOMDatasetAccessingImageFrameList
    ConvertToDICOMDatasetAccessingImageFrameList(const DICOMDatasetList& input);

}

#endif
