/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkCESTDICOMReaderService_h
#define mitkCESTDICOMReaderService_h

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that auto selects (using the mitk::DICOMFileReaderSelector) the best DICOMFileReader from
  the DICOM module and loads additional meta data for CEST data.
  */
  class CESTDICOMReaderService : public BaseDICOMReaderService
  {
  public:
    CESTDICOMReaderService();
    CESTDICOMReaderService(const std::string& description);

    /** Uses the BaseDICOMReaderService Read function and add extra steps for CEST meta data */
    using AbstractFileReader::Read;
    std::vector<itk::SmartPointer<BaseData> > Read() override;

  protected:
    /** Returns the reader instance that should be used. The decision may be based
    * one the passed list of relevant files.*/
    mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

  private:

    CESTDICOMReaderService* Clone() const override;
  };

}

#endif
