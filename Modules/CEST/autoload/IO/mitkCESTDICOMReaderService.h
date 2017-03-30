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

#ifndef MITKCESTDICOMReaderService_H
#define MITKCESTDICOMReaderService_H

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that auto selects (using the mitk::DICOMFileReaderSelector) the best DICOMFileReader from
  the DICOMReader module and loads additional meta data for CEST data.
  */
  class CESTDICOMReaderService : public BaseDICOMReaderService
  {
  public:
    CESTDICOMReaderService();
    CESTDICOMReaderService(const std::string& description);

    /** Uses the BaseDICOMReaderService Read function and add extra steps for CEST meta data */
    virtual std::vector<itk::SmartPointer<BaseData> > Read() override;

  protected:
    /** Returns the reader instance that should be used. The decision may be based
    * one the passed relevant file list.*/
    virtual mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

  private:

    virtual CESTDICOMReaderService* Clone() const override;
  };

}

#endif // MITKDICOMSERIESREADERSERVICE_H
