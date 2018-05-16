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

#ifndef MITKPHOTOACOUSTICTISSUEGENERATORPARAMETERS_H
#define MITKPHOTOACOUSTICTISSUEGENERATORPARAMETERS_H

#include <MitkPhotoacousticsLibExports.h>
#include <mitkPAVesselMeanderStrategy.h>

//Includes for smart pointer usage
#include "mitkCommon.h"
#include "itkLightObject.h"

namespace mitk {
  namespace pa {
    class MITKPHOTOACOUSTICSLIB_EXPORT TissueGeneratorParameters : public itk::Object
    {
    public:
      mitkClassMacroItkParent(TissueGeneratorParameters, itk::Object)
        itkFactorylessNewMacro(Self)

        /**
         * Callback function definition of a VesselMeanderStrategy
         */
        typedef void (VesselMeanderStrategy::*CalculateNewVesselPositionCallback)
        (Vector::Pointer, Vector::Pointer, double, std::mt19937*);

      itkGetMacro(XDim, int)
        itkGetMacro(YDim, int)
        itkGetMacro(ZDim, int)
        itkGetMacro(VoxelSpacingInCentimeters, double)
        itkGetMacro(VolumeSmoothingSigma, double)
        itkGetMacro(DoVolumeSmoothing, bool)
        itkGetMacro(UseRngSeed, bool)
        itkGetMacro(RngSeed, long)
        itkGetMacro(RandomizePhysicalProperties, bool)
        itkGetMacro(RandomizePhysicalPropertiesPercentage, double)

        itkGetMacro(BackgroundAbsorption, double)
        itkGetMacro(BackgroundScattering, double)
        itkGetMacro(BackgroundAnisotropy, double)
        itkGetMacro(AirAbsorption, double)
        itkGetMacro(AirScattering, double)
        itkGetMacro(AirAnisotropy, double)
        itkGetMacro(AirThicknessInMillimeters, double)
        itkGetMacro(SkinAbsorption, double)
        itkGetMacro(SkinScattering, double)
        itkGetMacro(SkinAnisotropy, double)
        itkGetMacro(SkinThicknessInMillimeters, double)

        itkGetMacro(CalculateNewVesselPositionCallback, CalculateNewVesselPositionCallback)
        itkGetMacro(MinNumberOfVessels, int)
        itkGetMacro(MaxNumberOfVessels, int)
        itkGetMacro(MinVesselBending, double)
        itkGetMacro(MaxVesselBending, double)
        itkGetMacro(MinVesselAbsorption, double)
        itkGetMacro(MaxVesselAbsorption, double)
        itkGetMacro(MinVesselRadiusInMillimeters, double)
        itkGetMacro(MaxVesselRadiusInMillimeters, double)
        itkGetMacro(VesselBifurcationFrequency, int)
        itkGetMacro(MinVesselScattering, double)
        itkGetMacro(MaxVesselScattering, double)
        itkGetMacro(MinVesselAnisotropy, double)
        itkGetMacro(MaxVesselAnisotropy, double)
        itkGetMacro(MinVesselZOrigin, double)
        itkGetMacro(MaxVesselZOrigin, double)

        itkGetMacro(MCflag, double)
        itkGetMacro(MCLaunchflag, double)
        itkGetMacro(MCBoundaryflag, double)
        itkGetMacro(MCLaunchPointX, double)
        itkGetMacro(MCLaunchPointY, double)
        itkGetMacro(MCLaunchPointZ, double)
        itkGetMacro(MCFocusPointX, double)
        itkGetMacro(MCFocusPointY, double)
        itkGetMacro(MCFocusPointZ, double)
        itkGetMacro(MCTrajectoryVectorX, double)
        itkGetMacro(MCTrajectoryVectorY, double)
        itkGetMacro(MCTrajectoryVectorZ, double)
        itkGetMacro(MCRadius, double)
        itkGetMacro(MCWaist, double)

        itkSetMacro(XDim, int)
        itkSetMacro(YDim, int)
        itkSetMacro(ZDim, int)
        itkSetMacro(VoxelSpacingInCentimeters, double)
        itkSetMacro(VolumeSmoothingSigma, double)
        itkSetMacro(DoVolumeSmoothing, bool)
        itkSetMacro(UseRngSeed, bool)
        itkSetMacro(RngSeed, long)
        itkSetMacro(RandomizePhysicalProperties, bool)
        itkSetMacro(RandomizePhysicalPropertiesPercentage, double)

        itkSetMacro(BackgroundAbsorption, double)
        itkSetMacro(BackgroundScattering, double)
        itkSetMacro(BackgroundAnisotropy, double)
        itkSetMacro(AirAbsorption, double)
        itkSetMacro(AirScattering, double)
        itkSetMacro(AirAnisotropy, double)
        itkSetMacro(AirThicknessInMillimeters, double)
        itkSetMacro(SkinAbsorption, double)
        itkSetMacro(SkinScattering, double)
        itkSetMacro(SkinAnisotropy, double)
        itkSetMacro(SkinThicknessInMillimeters, double)

        itkSetMacro(CalculateNewVesselPositionCallback, CalculateNewVesselPositionCallback)
        itkSetMacro(MinNumberOfVessels, int)
        itkSetMacro(MaxNumberOfVessels, int)
        itkSetMacro(MinVesselBending, double)
        itkSetMacro(MaxVesselBending, double)
        itkSetMacro(MinVesselAbsorption, double)
        itkSetMacro(MaxVesselAbsorption, double)
        itkSetMacro(MinVesselRadiusInMillimeters, double)
        itkSetMacro(MaxVesselRadiusInMillimeters, double)
        itkSetMacro(VesselBifurcationFrequency, int)
        itkSetMacro(MinVesselScattering, double)
        itkSetMacro(MaxVesselScattering, double)
        itkSetMacro(MinVesselAnisotropy, double)
        itkSetMacro(MaxVesselAnisotropy, double)
        itkSetMacro(MinVesselZOrigin, double)
        itkSetMacro(MaxVesselZOrigin, double)

        itkSetMacro(MCflag, double)
        itkSetMacro(MCLaunchflag, double)
        itkSetMacro(MCBoundaryflag, double)
        itkSetMacro(MCLaunchPointX, double)
        itkSetMacro(MCLaunchPointY, double)
        itkSetMacro(MCLaunchPointZ, double)
        itkSetMacro(MCFocusPointX, double)
        itkSetMacro(MCFocusPointY, double)
        itkSetMacro(MCFocusPointZ, double)
        itkSetMacro(MCTrajectoryVectorX, double)
        itkSetMacro(MCTrajectoryVectorY, double)
        itkSetMacro(MCTrajectoryVectorZ, double)
        itkSetMacro(MCRadius, double)
        itkSetMacro(MCWaist, double)

    protected:
      TissueGeneratorParameters();
      ~TissueGeneratorParameters() override;

    private:

      int m_XDim;
      int m_YDim;
      int m_ZDim;
      double m_VoxelSpacingInCentimeters;
      double m_VolumeSmoothingSigma;
      bool m_DoVolumeSmoothing;
      bool m_UseRngSeed;
      long m_RngSeed;
      bool m_RandomizePhysicalProperties;
      double m_RandomizePhysicalPropertiesPercentage;

      double m_BackgroundAbsorption;
      double m_BackgroundScattering;
      double m_BackgroundAnisotropy;
      double m_AirAbsorption;
      double m_AirScattering;
      double m_AirAnisotropy;
      double m_AirThicknessInMillimeters;
      double m_SkinAbsorption;
      double m_SkinScattering;
      double m_SkinAnisotropy;
      double m_SkinThicknessInMillimeters;

      CalculateNewVesselPositionCallback m_CalculateNewVesselPositionCallback;
      int m_MinNumberOfVessels;
      int m_MaxNumberOfVessels;
      double m_MinVesselBending;
      double m_MaxVesselBending;
      double m_MinVesselAbsorption;
      double m_MaxVesselAbsorption;
      double m_MinVesselRadiusInMillimeters;
      double m_MaxVesselRadiusInMillimeters;
      int m_VesselBifurcationFrequency;
      double m_MinVesselScattering;
      double m_MaxVesselScattering;
      double m_MinVesselAnisotropy;
      double m_MaxVesselAnisotropy;
      double m_MinVesselZOrigin;
      double m_MaxVesselZOrigin;

      double m_MCflag;
      double m_MCLaunchflag;
      double m_MCBoundaryflag;
      double m_MCLaunchPointX;
      double m_MCLaunchPointY;
      double m_MCLaunchPointZ;
      double m_MCFocusPointX;
      double m_MCFocusPointY;
      double m_MCFocusPointZ;
      double m_MCTrajectoryVectorX;
      double m_MCTrajectoryVectorY;
      double m_MCTrajectoryVectorZ;
      double m_MCRadius;
      double m_MCWaist;
    };
  }
}
#endif // MITKPHOTOACOUSTICTISSUEGENERATORPARAMETERS_H
