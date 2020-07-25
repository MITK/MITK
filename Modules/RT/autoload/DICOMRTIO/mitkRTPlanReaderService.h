/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRTPlanReaderService_h
#define mitkRTPlanReaderService_h

#include "mitkAbstractFileReader.h"

#include <vector>

#include "mitkIDICOMTagsOfInterest.h"
#include "mitkDICOMDatasetAccessingImageFrameInfo.h"

#include <usModuleContext.h>

#include "MitkDICOMRTIOExports.h"

namespace mitk
{
  /**
  * \brief RTPlanReaderService reads DICOM files of modality RTPLAN.
    \details The tags are defined in mitk::GetDefaultDICOMTagsOfInterest() in Module MitkDicomReader. They are stored as TemporoSpatialStringProperty.
    with the key as their respective DICOM tags.
    \note No image information is in RTPLAN.
  */
  class MITKDICOMRTIO_EXPORT RTPlanReaderService : public mitk::AbstractFileReader
  {

  public:
    RTPlanReaderService();
    RTPlanReaderService(const RTPlanReaderService& other);

    using AbstractFileReader::Read;

    ~RTPlanReaderService() override;

  protected:
    /**
    * \brief Reads the file (only tags).
      @details DICOMDCMTKTagScanner is used to read the tags
      \note No image information is in RTPLAN.
      \sa mitk::GetDefaultDICOMTagsOfInterest() for tags that are read
    */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    RTPlanReaderService* Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_FileReaderServiceReg;
  };
}

#endif
