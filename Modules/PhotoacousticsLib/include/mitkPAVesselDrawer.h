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
      virtual ~VesselDrawer();

    private:
    };

    }
}
#endif // MITKVESSELDRAWER_H
