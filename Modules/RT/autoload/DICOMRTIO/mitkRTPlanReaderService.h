/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


#ifndef mitkRTPlanReaderService_h
#define mitkRTPlanReaderService_h

#include <mitkAbstractFileReader.h>

#include <vector>

#include <mitkIDICOMTagsOfInterest.h>
#include <mitkDICOMDatasetAccessingImageFrameInfo.h>

#include <usModuleContext.h>

#include <MitkDICOMRTIOExports.h>

namespace mitk
{
  /**
   * \brief Reader service for DICOM files of modality RTPLAN.
   *
   * The tags are defined in mitk::GetDefaultDICOMTagsOfInterest() in Module
   * MitkDicomReader. They are stored as TemporoSpatialStringProperty with the
   * key as their respective DICOM tags.
   *
   * \note No image information is contained in RTPLAN files.
   */
  class MITKDICOMRTIO_EXPORT RTPlanReaderService : public mitk::AbstractFileReader
  {

  public:
    /** \brief Default constructor. Registers reader for the RTPLAN MIME type. */
    RTPlanReaderService();

    /** \brief Copy constructor. */
    RTPlanReaderService(const RTPlanReaderService& other);

    using AbstractFileReader::Read;

    ~RTPlanReaderService() override;

  protected:
    /**
     * \brief Read DICOM tags from the RTPLAN file.
     *
     * Uses DICOMDCMTKTagScanner to read the tags.
     *
     * \note No image information is in RTPLAN.
     * \sa mitk::GetDefaultDICOMTagsOfInterest() for tags that are read.
     * \return A vector of BaseData objects containing the parsed tag information.
     */
    std::vector<itk::SmartPointer<BaseData>> DoRead() override;

  private:
    RTPlanReaderService* Clone() const override;

    us::ServiceRegistration<mitk::IFileReader> m_FileReaderServiceReg;
  };
}

#endif
