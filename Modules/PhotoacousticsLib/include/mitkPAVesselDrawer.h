/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKVESSELDRAWER_H
#define MITKVESSELDRAWER_H

#include "mitkVector.h"
#include "mitkPAVesselMeanderStrategy.h"
#include "mitkPAInSilicoTissueVolume.h"
#include "mitkPAVector.h"
#include "mitkPAVesselProperties.h"

#include <MitkPhotoacousticsLibExports.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT VesselDrawer : public itk::LightObject
    {
    public:
      mitkClassMacroItkParent(VesselDrawer, itk::LightObject);
      itkFactorylessNewMacro(Self);

      void ExpandAndDrawVesselInVolume(
        VesselProperties::Pointer properties,
        InSilicoTissueVolume::Pointer volume);

    protected:
      VesselDrawer();
      ~VesselDrawer() override;

    private:
    };

    }
}
#endif // MITKVESSELDRAWER_H
