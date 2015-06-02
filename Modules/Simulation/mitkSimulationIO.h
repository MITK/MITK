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

#ifndef mitkSimulationIO_h
#define mitkSimulationIO_h

#include <mitkAbstractFileIO.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MITKSIMULATION_EXPORT SimulationIO : public AbstractFileIO
  {
  public:
    SimulationIO();

    using AbstractFileReader::Read;
    std::vector<BaseData::Pointer> Read() override;
    ConfidenceLevel GetReaderConfidenceLevel() const override;

    void Write() override;
    ConfidenceLevel GetWriterConfidenceLevel() const override;

  private:
    SimulationIO* IOClone() const override;
  };
}

#endif
