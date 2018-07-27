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
