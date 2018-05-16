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

#include "mitkPATissueGeneratorParameters.h"

mitk::pa::TissueGeneratorParameters::TissueGeneratorParameters()
{
  m_XDim = 50;
  m_YDim = 50;
  m_ZDim = 50;
  m_VoxelSpacingInCentimeters = 1;
  m_VolumeSmoothingSigma = 0;
  m_DoVolumeSmoothing = false;
  m_UseRngSeed = false;
  m_RngSeed = 1337L;
  m_RandomizePhysicalProperties = false;
  m_RandomizePhysicalPropertiesPercentage = 0;

  m_BackgroundAbsorption = 0.1;
  m_BackgroundScattering = 15;
  m_BackgroundAnisotropy = 0.9;
  m_AirAbsorption = 0.0001;
  m_AirScattering = 1;
  m_AirAnisotropy = 1;
  m_AirThicknessInMillimeters = 0;
  m_SkinAbsorption = 0.1;
  m_SkinScattering = 15;
  m_SkinAnisotropy = 0.9;
  m_SkinThicknessInMillimeters = 0;

  m_CalculateNewVesselPositionCallback = &VesselMeanderStrategy::CalculateRandomlyDivergingPosition;
  m_MinNumberOfVessels = 0;
  m_MaxNumberOfVessels = 0;
  m_MinVesselBending = 0;
  m_MaxVesselBending = 0.1;
  m_MinVesselAbsorption = 1;
  m_MaxVesselAbsorption = 8;
  m_MinVesselRadiusInMillimeters = 1;
  m_MaxVesselRadiusInMillimeters = 3;
  m_VesselBifurcationFrequency = 25;
  m_MinVesselScattering = 15;
  m_MaxVesselScattering = 15;
  m_MinVesselAnisotropy = 0.9;
  m_MaxVesselAnisotropy = 0.9;
  m_MinVesselZOrigin = 10;
  m_MaxVesselZOrigin = 40;

  m_MCflag = 1;
  m_MCLaunchflag = 0;
  m_MCBoundaryflag = 2;
  m_MCLaunchPointX = 25;
  m_MCLaunchPointY = 25;
  m_MCLaunchPointZ = 2;
  m_MCFocusPointX = 25;
  m_MCFocusPointY = 25;
  m_MCFocusPointZ = 25;
  m_MCTrajectoryVectorX = 0;
  m_MCTrajectoryVectorY = 0;
  m_MCTrajectoryVectorZ = 1;
  m_MCRadius = 2;
  m_MCWaist = 4;
}

mitk::pa::TissueGeneratorParameters::~TissueGeneratorParameters()
{
}
