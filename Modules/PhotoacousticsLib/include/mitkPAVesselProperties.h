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

#ifndef MITKPhotoacousticVesselParameters_H
#define MITKPhotoacousticVesselParameters_H

#include <MitkPhotoacousticsLibExports.h>

#include <mitkPAVector.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT VesselProperties : public itk::Object
    {
    public:
      mitkClassMacroItkParent(VesselProperties, itk::Object)
        itkFactorylessNewMacro(Self)
        mitkNewMacro1Param(Self, Self::Pointer)

        itkGetMacro(PositionVector, Vector::Pointer)
        itkGetMacro(DirectionVector, Vector::Pointer)
        itkGetMacro(RadiusInVoxel, double)
        itkGetMacro(AbsorptionCoefficient, double)
        itkGetMacro(ScatteringCoefficient, double)
        itkGetMacro(AnisotopyCoefficient, double)
        itkGetMacro(BifurcationFrequency, double)

        itkSetMacro(PositionVector, Vector::Pointer)
        itkSetMacro(DirectionVector, Vector::Pointer)
        itkSetMacro(RadiusInVoxel, double)
        itkSetMacro(AbsorptionCoefficient, double)
        itkSetMacro(ScatteringCoefficient, double)
        itkSetMacro(AnisotopyCoefficient, double)
        itkSetMacro(BifurcationFrequency, double)

    protected:
      VesselProperties();
      VesselProperties(Self::Pointer other);
      ~VesselProperties() override;

    private:
      Vector::Pointer m_PositionVector;
      Vector::Pointer m_DirectionVector;
      double m_RadiusInVoxel;
      double m_AbsorptionCoefficient;
      double m_ScatteringCoefficient;
      double m_AnisotopyCoefficient;
      double m_BifurcationFrequency;
    };

    /**
    * @brief Equal A function comparing two VesselProperty instances for beeing equal
    *
    * @param rightHandSide A VesselProperty to be compared
    * @param leftHandSide A Vesselproperty to be compared
    * @param eps tolarence for comparison. You can use mitk::eps in most cases.
    * @param verbose flag indicating if the user wants detailed console output or not.
    * @return true, if all subsequent comparisons are true, false otherwise
    */
    MITKPHOTOACOUSTICSLIB_EXPORT bool Equal(const VesselProperties::Pointer leftHandSide, const VesselProperties::Pointer rightHandSide, double eps, bool verbose);
  }
}
#endif // MITKPhotoacousticVesselParameters_H
