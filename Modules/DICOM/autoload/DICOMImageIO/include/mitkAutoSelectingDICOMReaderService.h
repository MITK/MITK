/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkAutoSelectingDICOMReaderService_h
#define mitkAutoSelectingDICOMReaderService_h

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that auto selects (using the mitk::DICOMFileReaderSelector) the best DICOMFileReader from
  the DICOM module.
  */
class AutoSelectingDICOMReaderService : public BaseDICOMReaderService
{
public:
  AutoSelectingDICOMReaderService();

protected:
  /** Returns the reader instance that should be used. The decision may be based
  * one the passed list of relevant files.*/
  mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

private:

  AutoSelectingDICOMReaderService* Clone() const override;
};

}

#endif
