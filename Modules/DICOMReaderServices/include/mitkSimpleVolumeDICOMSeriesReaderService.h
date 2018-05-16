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
  /** Returns the reader instance that should be used. The descission may be based
  * one the passed relevant file list.*/
  mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

private:
  SimpleVolumeDICOMSeriesReaderService* Clone() const override;

};

}

#endif // MITKSIMPLEVOLUMEDICOMSERIESREADERSERVICE_H
