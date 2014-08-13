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

#ifndef mitkDICOMGDCMImageFrameInfo_h
#define mitkDICOMGDCMImageFrameInfo_h

#include "mitkDICOMImageFrameInfo.h"
#include "mitkDICOMDatasetAccess.h"

#include "gdcmScanner.h"

namespace mitk
{
  /**
    \ingroup DICOMReaderModule
    \brief The dataset access implementation for DICOMITKSeriesGDCMReader, based on GDCM.

    This class combines a DICOMImageFrameInfo object with the scanning results
    from gdcm::Scanner. The scanning results will be used to implement the tag
    access methods of DICOMDatasetAccess.
  */
  class MitkDICOMReader_EXPORT DICOMGDCMImageFrameInfo : public itk::LightObject, public DICOMDatasetAccess
  {
    public:

      mitkClassMacro(DICOMGDCMImageFrameInfo, itk::LightObject);
      itkNewMacro( DICOMGDCMImageFrameInfo );
      mitkNewMacro1Param( DICOMGDCMImageFrameInfo, const std::string&);
      mitkNewMacro2Param( DICOMGDCMImageFrameInfo, const std::string&, unsigned int );
      mitkNewMacro1Param( DICOMGDCMImageFrameInfo, DICOMImageFrameInfo::Pointer);
      mitkNewMacro2Param( DICOMGDCMImageFrameInfo, DICOMImageFrameInfo::Pointer, gdcm::Scanner::TagToValue const&);

      virtual ~DICOMGDCMImageFrameInfo();

      virtual std::string GetTagValueAsString(const DICOMTag&) const;

      std::string GetFilenameIfAvailable() const;

      /// The frame that this objects refers to
      DICOMImageFrameInfo::Pointer GetFrameInfo() const;
      /// The frame that this objects refers to
      void SetFrameInfo(DICOMImageFrameInfo::Pointer frameinfo);

    protected:

      DICOMImageFrameInfo::Pointer m_FrameInfo;

      DICOMGDCMImageFrameInfo(DICOMImageFrameInfo::Pointer frameinfo);
      DICOMGDCMImageFrameInfo(DICOMImageFrameInfo::Pointer frameinfo, gdcm::Scanner::TagToValue const& tagToValueMapping);
      DICOMGDCMImageFrameInfo(const std::string& filename = "", unsigned int frameNo = 0);

      const gdcm::Scanner::TagToValue m_TagForValue;
  };

  typedef std::vector<DICOMGDCMImageFrameInfo::Pointer> DICOMGDCMImageFrameList;
}

#endif
