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

#ifndef mitkSimulationSerializer_h
#define mitkSimulationSerializer_h

#include <mitkBaseDataSerializer.h>
#include <MitkSimulationExports.h>

namespace mitk
{
  class MitkSimulation_EXPORT SimulationSerializer : public BaseDataSerializer
  {
  public:
    mitkClassMacro(SimulationSerializer, BaseDataSerializer);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    std::string Serialize();

  protected:
    SimulationSerializer();
    ~SimulationSerializer();
  };
}

#endif
