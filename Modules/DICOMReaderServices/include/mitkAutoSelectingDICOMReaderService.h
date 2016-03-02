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

#ifndef MITKAUTOSELECTINGDICOMREADERSERVICE_H
#define MITKAUTOSELECTINGDICOMREADERSERVICE_H

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that auto selects (using the mitk::DICOMFileReaderSelector) the best DICOMFileReader from
  the DICOMReader module.
  */
class AutoSelectingDICOMReaderService : public BaseDICOMReaderService
{
public:
  AutoSelectingDICOMReaderService();

protected:
  /** Returns the reader instance that should be used. The descission may be based
  * one the passed relevant file list.*/
  virtual mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

private:

  virtual AutoSelectingDICOMReaderService* Clone() const override;
};

}

#endif // MITKDICOMSERIESREADERSERVICE_H
