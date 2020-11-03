/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkDICOMGDCMImageFrameInfo_h
#define mitkDICOMGDCMImageFrameInfo_h

#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

#include "gdcmScanner.h"

namespace mitk
{
  /**
    \ingroup DICOMModule
    \brief The dataset access implementation for DICOMITKSeriesGDCMReader, based on GDCM.

    This class combines a DICOMImageFrameInfo object with the scanning results
    from gdcm::Scanner. The scanning results will be used to implement the tag
    access methods of DICOMDatasetAccess.
  */
  class MITKDICOM_EXPORT DICOMGDCMImageFrameInfo : public DICOMDatasetAccessingImageFrameInfo
  {
    public:

      mitkClassMacro(DICOMGDCMImageFrameInfo, DICOMDatasetAccessingImageFrameInfo);
      itkFactorylessNewMacro( DICOMGDCMImageFrameInfo );
      mitkNewMacro1Param( DICOMGDCMImageFrameInfo, const std::string&);
      mitkNewMacro2Param( DICOMGDCMImageFrameInfo, const std::string&, unsigned int );
      mitkNewMacro1Param( DICOMGDCMImageFrameInfo, const DICOMImageFrameInfo::Pointer& );
      mitkNewMacro2Param( DICOMGDCMImageFrameInfo, const DICOMImageFrameInfo::Pointer&, gdcm::Scanner::TagToValue const&);

      ~DICOMGDCMImageFrameInfo() override;

      DICOMDatasetFinding GetTagValueAsString(const DICOMTag&) const override;

      FindingsListType GetTagValueAsString(const DICOMTagPath& path) const override;

      std::string GetFilenameIfAvailable() const override;

    protected:
      explicit DICOMGDCMImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo);
      DICOMGDCMImageFrameInfo(const DICOMImageFrameInfo::Pointer& frameinfo, gdcm::Scanner::TagToValue const& tagToValueMapping);
      DICOMGDCMImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);

      const gdcm::Scanner::TagToValue m_TagForValue;
  };

  typedef std::vector<DICOMGDCMImageFrameInfo::Pointer> DICOMGDCMImageFrameList;
}

#endif
