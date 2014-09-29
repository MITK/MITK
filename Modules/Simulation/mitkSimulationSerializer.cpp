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

#include "mitkSimulation.h"
#include "mitkSimulationSerializer.h"
#include "mitkSimulationWriter.h"

MITK_REGISTER_SERIALIZER(SimulationSerializer)

mitk::SimulationSerializer::SimulationSerializer()
{
}

mitk::SimulationSerializer::~SimulationSerializer()
{
}

std::string mitk::SimulationSerializer::Serialize()
{
  const Simulation* simulation = dynamic_cast<const Simulation*>(m_Data.GetPointer());

  if (simulation == NULL)
  {
    MITK_ERROR << " Invalid simulation object cannot be serialized!";
    return "";
  }

  std::string filename(this->GetUniqueFilenameInWorkingDirectory());
  filename += "_" + m_FilenameHint + ".scn";

  std::string fullPath = m_WorkingDirectory;
  fullPath += "/" + filename;

  try
  {
    SimulationWriter::Pointer writer = SimulationWriter::New();
    writer->SetFileName(fullPath.c_str());
    writer->SetInput(const_cast<Simulation*>(simulation));
    writer->Write();
  }
  catch (const std::exception& e)
  {
    MITK_ERROR << " Error serializing simulation object to " << fullPath << ": " << e.what();
    return "";
  }

  return filename;
}
