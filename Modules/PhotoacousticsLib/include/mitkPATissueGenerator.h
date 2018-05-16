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

#ifndef mitkPhotoacousticTissueGenerator_h
#define mitkPhotoacousticTissueGenerator_h

#include <mitkBaseData.h>
#include <vector>
#include <random>
#include <chrono>

#include <MitkPhotoacousticsLibExports.h>
#include <mitkPATissueGeneratorParameters.h>

#include "mitkPAVesselTree.h"
#include "mitkPAInSilicoTissueVolume.h"

#include "mitkCommon.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT InSilicoTissueGenerator final
    {
    public:

      /**
       * @brief GenerateInSilicoData This method will return a InSilicoTissueVolume created in terms of the given parameters.
       * @param parameters
       * @return
       */
      static InSilicoTissueVolume::Pointer GenerateInSilicoData(TissueGeneratorParameters::Pointer parameters);

    private:
      InSilicoTissueGenerator();
      virtual ~InSilicoTissueGenerator();
    };
  }
}
#endif
