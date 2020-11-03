/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPhotoacousticPhantomTissueGenerator_h
#define mitkPhotoacousticPhantomTissueGenerator_h

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
    class MITKPHOTOACOUSTICSLIB_EXPORT PhantomTissueGenerator final
    {
    public:

      /**
       * @brief GenerateInSilicoData This method will return a InSilicoTissueVolume created in terms of the given parameters.
       * @param parameters
       * @return
       */
      static InSilicoTissueVolume::Pointer GeneratePhantomData(TissueGeneratorParameters::Pointer parameters);

    private:
      PhantomTissueGenerator();
      virtual ~PhantomTissueGenerator();
    };
  }
}
#endif
