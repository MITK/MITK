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

#ifndef _MITK_SingleShotEpi_H
#define _MITK_SingleShotEpi_H

#include <mitkAcquisitionType.h>

namespace mitk {

/**
  * \brief Realizes EPI readout: one echo, maximum intensity in the k-space center, zig-zag trajectory
  *
  */
class SingleShotEpi : public AcquisitionType
{
public:

  SingleShotEpi(FiberfoxParameters* parameters) : AcquisitionType(parameters)
  {
    kxMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0);
    kyMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1);

    dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

    // k-space center at maximum echo
    if ( kyMax%2==0 )
    {
      m_NegTEhalf = -m_Parameters->m_SignalGen.m_tLine*(kyMax-1)/2 + dt*(kxMax-(int)kxMax%2)/2;
    }
    else
      m_NegTEhalf = -m_Parameters->m_SignalGen.m_tLine*(kyMax-1)/2 - dt*(kxMax-(int)kxMax%2)/2;
  }
  ~SingleShotEpi() override
  {}

  float GetTimeFromMaxEcho(itk::Index< 2 > index) override
  {
    float t = 0;
    t = m_NegTEhalf + ((float)index[1]*kxMax+(float)index[0])*dt;
    return t;
  }

  float GetRedoutTime(itk::Index< 2 > index) override
  {
    float t = 0;
    t = ((float)index[1]*kxMax+(float)index[0])*dt;
    return t;
  }

  itk::Index< 2 > GetActualKspaceIndex(itk::Index< 2 > index) override
  {
    // reverse phase
    if (!m_Parameters->m_SignalGen.m_ReversePhase)
      index[1] = kyMax-1-index[1];

    // reverse readout direction
    if (index[1]%2 == 1)
      index[0] = kxMax-index[0]-1;

    return index;
  }

  void AdjustEchoTime() override
  {
    int temp = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)*m_Parameters->m_SignalGen.m_PartialFourier - (m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)+m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)%2)/2;

    if ( m_Parameters->m_SignalGen.m_tEcho/2 < temp*m_Parameters->m_SignalGen.m_tLine )
    {
      m_Parameters->m_SignalGen.m_tEcho = 2*temp*m_Parameters->m_SignalGen.m_tLine;
      MITK_WARN << "Echo time is too short! Time not sufficient to read slice. Automatically adjusted to " << m_Parameters->m_SignalGen.m_tEcho << " ms";
      m_Parameters->m_Misc.m_AfterSimulationMessage += "Echo time was chosen too short! Time not sufficient to read slice. Internally adjusted to " + boost::lexical_cast<std::string>(m_Parameters->m_SignalGen.m_tEcho) + " ms\n";
    }
  }

protected:

  float dt;
  int kxMax;
  int kyMax;

};

}

#endif

