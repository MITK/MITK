/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkClassicDICOMSeriesReaderService_h
#define mitkClassicDICOMSeriesReaderService_h

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that directly returns an instance of ClassicDICOMSeriesReader class from
  the DICOM module.
  */
class ClassicDICOMSeriesReaderService : public BaseDICOMReaderService
{
public:
  ClassicDICOMSeriesReaderService();

protected:
  /** Returns a ClassicDICOMSeriesReader instance.*/
  mitk::DICOMFileReader::Pointer GetReader(const mitk::StringList& relevantFiles) const override;

private:
  ClassicDICOMSeriesReaderService* Clone() const override;

};

}

#endif
