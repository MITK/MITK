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
    dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

    // k-space center at maximum echo
    if ( kyMax%2==0 )
    {
      m_NegTEhalf = -m_Parameters->m_SignalGen.m_tLine*(kyMax-1)/2 + dt*(kxMax-kxMax%2)/2;
    }
    else
      m_NegTEhalf = -m_Parameters->m_SignalGen.m_tLine*(kyMax-1)/2 - dt*(kxMax-kxMax%2)/2;
  }
  ~SingleShotEpi() override
  {}

  // one echo per slice
  float GetTimeFromMaxEcho(const itk::Index< 2 >& index) override
  {
    float t = 0;
    t = m_NegTEhalf + (static_cast<float>(index[1])*kxMax+static_cast<float>(index[0]))*dt;
    return t;
  }

  float GetRedoutTime(const itk::Index< 2 >& index) override
  {
    float t = 0;
    t = (static_cast<float>(index[1])*kxMax+static_cast<float>(index[0]))*dt;
    return t;
  }

  float GetTimeFromRf(const itk::Index< 2 >& index) override
  {
    return m_Parameters->m_SignalGen.m_tEcho + GetTimeFromMaxEcho(index);
  }

  itk::Index< 2 > GetActualKspaceIndex(const itk::Index< 2 >& index) override
  {
    itk::Index< 2 > out_idx = index;
    // reverse phase
    if (!m_Parameters->m_SignalGen.m_ReversePhase)
      out_idx[1] = kyMax-1-out_idx[1];

    // reverse readout direction
    if (out_idx[1]%2 == 1)
      out_idx[0] = kxMax-out_idx[0]-1;

    return out_idx;
  }

  void AdjustEchoTime() override
  {
    auto temp = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)*m_Parameters->m_SignalGen.m_PartialFourier - (m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)+m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)%2)/2;

    if ( m_Parameters->m_SignalGen.m_tEcho/2 < temp*m_Parameters->m_SignalGen.m_tLine )
    {
      m_Parameters->m_SignalGen.m_tEcho = 2*temp*m_Parameters->m_SignalGen.m_tLine;
      MITK_WARN << "Echo time is too short! Time not sufficient to read slice. Automatically adjusted to " << m_Parameters->m_SignalGen.m_tEcho << " ms";
      m_Parameters->m_Misc.m_AfterSimulationMessage += "Echo time was chosen too short! Time not sufficient to read slice. Internally adjusted to " + boost::lexical_cast<std::string>(m_Parameters->m_SignalGen.m_tEcho) + " ms\n";
    }
  }

protected:

};

}

#endif

