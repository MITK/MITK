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


#ifndef mitkDicomRTPlanReader_h
#define mitkDicomRTPlanReader_h

#include "mitkAbstractFileReader.h"

#include <string>
#include <vector>

#include "mitkImage.h"
#include "mitkIDICOMTagsOfInterest.h"
#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

#include <usModuleContext.h>

#include "MitkDicomRTExports.h"

namespace mitk
{
    /**
    * \brief RTPlanReader reads DICOM files of modality RTPLAN.
      \details The tags are defined in mitk::GetDefaultDICOMTagsOfInterest(). They are stored as TemporoSpatialStringProperty.
      with the key as their respective DICOM tags.
      \note No image information is in RTPLAN.
    */
    class MITKDICOMRT_EXPORT RTPlanReader : public mitk::AbstractFileReader
  {

  public:
      typedef std::vector<mitk::DICOMDatasetAccess::FindingsListType> FindingsListVectorType;
      RTPlanReader();
      RTPlanReader(const RTPlanReader& other);

      using AbstractFileReader::Read;
      virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

      virtual ~RTPlanReader();

  private:
    RTPlanReader* Clone() const override;
    FindingsListVectorType ExtractPathsOfInterest(const DICOMTagPathList& pathsOfInterest, const DICOMDatasetAccessingImageFrameList& frames) const;
    void SetProperties(Image::Pointer image, const FindingsListVectorType& findings) const;
    mitk::IDICOMTagsOfInterest* GetDicomTagsOfInterestService() const;

    us::ServiceRegistration<mitk::IFileReader> m_FileReaderServiceReg;
    IDICOMTagsOfInterest* m_DICOMTagsOfInterestService;
  };
}

#endif
