/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKCESTGenericDICOMReaderService_H
#define MITKCESTGenericDICOMReaderService_H

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that auto selects (using the mitk::DICOMFileReaderSelector) the best DICOMFileReader from
  the DICOM module and loads the CEST relevant meta data from a provided cest_meta.json file or
  provided from the user as reader options.
  */
  class CESTDICOMManualReaderService : public BaseDICOMReaderService
  {
  public:
    CESTDICOMManualReaderService(const CustomMimeType& mimeType, const std::string& description);

    /** Uses the AbstractFileReader Read function and add extra steps for CEST meta data */
    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

    Options GetOptions() const override;
    us::Any GetOption(const std::string& name) const override;

  protected:
    CESTDICOMManualReaderService(const CESTDICOMManualReaderService&) = default;
    CESTDICOMManualReaderService& operator=(const CESTDICOMManualReaderService&) = delete;

    std::string GetCESTMetaFilePath() const;
    std::string GetTRECFilePath() const;
    std::string GetLISTFilePath() const;

    mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

  private:
    CESTDICOMManualReaderService* Clone() const override;
  };

  DICOMTagPath DICOM_IMAGING_FREQUENCY_PATH();
}

#endif // MITKCESTGenericDICOMReaderService_H
