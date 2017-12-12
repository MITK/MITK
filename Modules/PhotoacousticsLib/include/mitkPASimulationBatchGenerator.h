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

#ifndef MITKPHOTOACOUSTICSIMULATIONBATCHGENERATOR_H
#define MITKPHOTOACOUSTICSIMULATIONBATCHGENERATOR_H

#include <MitkPhotoacousticsLibExports.h>
#include <mitkCommon.h>
#include <mitkImage.h>

#include <mitkPASimulationBatchGeneratorParameters.h>

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SimulationBatchGenerator
    {
    public:
      static void WriteBatchFileAndSaveTissueVolume(
        SimulationBatchGeneratorParameters::Pointer parameters,
        mitk::Image::Pointer tissueVolume);

      static std::string CreateBatchSimulationString(
        SimulationBatchGeneratorParameters::Pointer parameter);
    protected:
      SimulationBatchGenerator();
      virtual ~SimulationBatchGenerator();

      static std::string GetVolumeNumber(
        SimulationBatchGeneratorParameters::Pointer parameters);

      static std::string GetOutputFolderName(
        SimulationBatchGeneratorParameters::Pointer parameter);
    };
  }
}
#endif // MITKPHOTOACOUSTICSIMULATIONBATCHGENERATOR_H
