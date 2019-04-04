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

  /*
                      TE
  | dt | dt | dt | ... |

  Total read time: Nvox*dt = kxMax*kyMax*dt

  */
  float half_read_time;
  SingleShotEpi(FiberfoxParameters* parameters) : AcquisitionType(parameters)
  {
    dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel
    half_read_time = (kxMax*kyMax) * dt/2;
  }
  ~SingleShotEpi() override
  {}

  // one echo per slice
  float GetTimeFromMaxEcho(const int& tick) override
  {
    float t = dt*(static_cast<float>(tick) + 0.5f) - half_read_time;
    return t;
  }

  // we simply assume that readout starts directly after the last diffusion gradient
  float GetTimeFromLastDiffusionGradient(const int& tick) override
  {
    return tick*dt + dt/2;
  }

  float GetTimeFromRf(const int& tick) override
  {
    return m_Parameters->m_SignalGen.m_tEcho + GetTimeFromMaxEcho(tick);
  }

  itk::Index< 2 > GetActualKspaceIndex(const int& tick) override
  {
    itk::Index< 2 > out_idx;
    out_idx[0] = tick % kxMax;
    out_idx[1] = tick / kxMax;

    if (!m_Parameters->m_SignalGen.m_ReversePhase)
    {
      out_idx[1] = kyMax-1-out_idx[1];  // in the not reversed case we start at the maximum k-space line

      if (out_idx[1]%2 == 1) // reverse frequency encoding direction
        out_idx[0] = kxMax-out_idx[0]-1;
    }
    else if (out_idx[1]%2) // reverse frequency encoding direction
      out_idx[0] = kxMax-out_idx[0]-1;

    return out_idx;
  }

  void AdjustEchoTime() override
  {
    if ( m_Parameters->m_SignalGen.m_tEcho < kyMax*m_Parameters->m_SignalGen.m_tLine )
    {
      m_Parameters->m_SignalGen.m_tEcho = kyMax*m_Parameters->m_SignalGen.m_tLine;
      MITK_WARN << "Echo time is too short! Time not sufficient to read slice. Automatically adjusted to " << m_Parameters->m_SignalGen.m_tEcho << " ms";
      m_Parameters->m_Misc.m_AfterSimulationMessage += "Echo time was chosen too short! Time not sufficient to read slice. Internally adjusted to " + boost::lexical_cast<std::string>(m_Parameters->m_SignalGen.m_tEcho) + " ms\n";
    }
  }

protected:

};

}

#endif

