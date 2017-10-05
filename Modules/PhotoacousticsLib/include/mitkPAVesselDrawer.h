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

      void DrawVesselInVolume(
        VesselProperties::Pointer properties,
        InSilicoTissueVolume::Pointer volume);

    protected:
      VesselDrawer();
      virtual ~VesselDrawer();

    private:
    };

    /**
    * @brief Equal A function comparing two vessel drawer instances for beeing equal
    *
    * @param rightHandSide A vesseldrawer to be compared
    * @param leftHandSide A vesseldrawer to be compared
    * @param eps tolarence for comparison. You can use mitk::eps in most cases.
    * @param verbose flag indicating if the user wants detailed console output or not.
    * @return true, if all subsequent comparisons are true, false otherwise
    */
    MITKPHOTOACOUSTICSLIB_EXPORT bool Equal(const VesselDrawer::Pointer leftHandSide, const VesselDrawer::Pointer rightHandSide, double eps, bool verbose);
  }
}
#endif // MITKVESSELDRAWER_H
