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

#ifndef MITKCLASSICDICOMSERIESREADERSERVICE_H
#define MITKCLASSICDICOMSERIESREADERSERVICE_H

#include <mitkBaseDICOMReaderService.h>

namespace mitk {

  /**
  Service wrapper that directly returns an instance of ClassicDICOMSeriesReader class from
  the DICOMReader module.
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

#endif // MITKCLASSICDICOMSERIESREADERSERVICE_H
