/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKPHOTOACOUSTICSimulationBatchGENERATORPARAMETERS_H
#define MITKPHOTOACOUSTICSimulationBatchGENERATORPARAMETERS_H

#include <MitkPhotoacousticsLibExports.h>
#include <mitkPAVesselMeanderStrategy.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT SimulationBatchGeneratorParameters : public itk::Object
    {
    public:
      mitkClassMacroItkParent(SimulationBatchGeneratorParameters, itk::Object);
        itkFactorylessNewMacro(Self);
        itkSetMacro(VolumeIndex, unsigned int);
        itkSetMacro(NrrdFilePath, std::string);
        itkSetMacro(TissueName, std::string);
        itkSetMacro(BinaryPath, std::string);
        itkSetMacro(NumberOfPhotons, long);
        itkSetMacro(YOffsetLowerThresholdInCentimeters, double);
        itkSetMacro(YOffsetUpperThresholdInCentimeters, double);
        itkSetMacro(YOffsetStepInCentimeters, double);
        itkGetMacro(VolumeIndex, unsigned int);

        itkGetMacro(NrrdFilePath, std::string);

        itkGetMacro(TissueName, std::string);

        itkGetMacro(BinaryPath, std::string);
        itkGetMacro(NumberOfPhotons, long);
        itkGetMacro(YOffsetLowerThresholdInCentimeters, double);
        itkGetMacro(YOffsetUpperThresholdInCentimeters, double);
        itkGetMacro(YOffsetStepInCentimeters, double);

    protected:
      SimulationBatchGeneratorParameters();
      ~SimulationBatchGeneratorParameters() override;

    private:

      unsigned int m_VolumeIndex;
      std::string m_NrrdFilePath;
      std::string m_TissueName;
      std::string m_BinaryPath;
      long m_NumberOfPhotons;
      double m_YOffsetLowerThresholdInCentimeters;
      double m_YOffsetUpperThresholdInCentimeters;
      double m_YOffsetStepInCentimeters;
    };
  }
}
#endif // MITKPHOTOACOUSTICSimulationBatchGENERATORPARAMETERS_H
