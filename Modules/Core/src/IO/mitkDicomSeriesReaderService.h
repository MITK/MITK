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

#ifndef MITKDICOMSERIESREADERSERVICE_H
#define MITKDICOMSERIESREADERSERVICE_H

#include <mitkAbstractFileReader.h>

namespace mitk {

class DicomSeriesReaderService : public AbstractFileReader
{
public:
  DicomSeriesReaderService();

  using AbstractFileReader::Read;
  virtual std::vector<itk::SmartPointer<BaseData> > Read();

private:

  virtual DicomSeriesReaderService* Clone() const;
};

}

#endif // MITKDICOMSERIESREADERSERVICE_H
