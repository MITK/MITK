/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
