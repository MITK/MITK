/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKSIMPLEVOLUMEDICOMSERIESREADERSERVICE_H
#define MITKSIMPLEVOLUMEDICOMSERIESREADERSERVICE_H

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that selects a DICOMITKSeriesGDCMReader configured for a simple and non-restrictive 3D volume import.
  */
class SimpleVolumeDICOMSeriesReaderService : public BaseDICOMReaderService
{
public:
  SimpleVolumeDICOMSeriesReaderService();

protected:
  /** Returns the reader instance that should be used. The decision may be based
  * one the passed list of relevant files.*/
  mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

private:
  SimpleVolumeDICOMSeriesReaderService* Clone() const override;

};

}

#endif // MITKSIMPLEVOLUMEDICOMSERIESREADERSERVICE_H
