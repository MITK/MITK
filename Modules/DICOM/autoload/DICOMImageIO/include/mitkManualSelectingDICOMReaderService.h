/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkManualSelectingDICOMReaderService_h
#define mitkManualSelectingDICOMReaderService_h

#include <mitkBaseDICOMReaderService.h>
#include <mitkDICOMFileReaderSelector.h>

namespace mitk {

  /**
  Service wrapper that offers a manual selection of the DICOMFileReader configuration that should be used.
  */
class ManualSelectingDICOMReaderService : public BaseDICOMReaderService
{
public:
  ManualSelectingDICOMReaderService();

protected:
  /** Returns the reader instance that should be used. The decision may be based
  * one the passed list of relevant files.*/
  mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

private:

  ManualSelectingDICOMReaderService* Clone() const override;

  DICOMFileReaderSelector::Pointer m_Selector;
};

}

#endif
