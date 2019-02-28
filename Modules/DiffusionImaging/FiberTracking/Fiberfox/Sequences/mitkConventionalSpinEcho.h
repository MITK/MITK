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

#ifndef _MITK_ConventionalSpinEcho_H
#define _MITK_ConventionalSpinEcho_H

#include <mitkAcquisitionType.h>

namespace mitk {

/**
  * \brief Conventional spin echo sequence. Cartesian readout. One echo and one excitation per k-space line.
  *
  */
class ConventionalSpinEcho : public AcquisitionType
{
public:

  float half_read_time;
  ConventionalSpinEcho(FiberfoxParameters* parameters) : AcquisitionType(parameters)
  {
    dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

    half_read_time = kxMax * dt/2;
  }
  ~ConventionalSpinEcho() override
  {}

  // one echo per k-space line
  float GetTimeFromMaxEcho(const int& tick) override
  {
    float t = dt*(tick % kxMax + 0.5f) - half_read_time;
    return t;
  }

  float GetTimeFromLastDiffusionGradient(const int& tick) override
  {
    return (tick % kxMax)*dt;
  }

  // time from max-echo + TE
  float GetTimeFromRf(const int& tick) override
  {
    return m_Parameters->m_SignalGen.m_tEcho + GetTimeFromMaxEcho(tick);
  }

  itk::Index< 2 > GetActualKspaceIndex(const int& tick) override
  {
    itk::Index< 2 > out_idx;
    out_idx[0] = tick % kxMax;
    out_idx[1] = tick / kxMax;

    // reverse phase
    if (!m_Parameters->m_SignalGen.m_ReversePhase)
      out_idx[1] = kyMax-1-out_idx[1];

    return out_idx;
  }

  void AdjustEchoTime() override
  {
    if ( m_Parameters->m_SignalGen.m_tEcho < m_Parameters->m_SignalGen.m_tLine )
    {
      m_Parameters->m_SignalGen.m_tEcho = m_Parameters->m_SignalGen.m_tLine;
      MITK_WARN << "Echo time is too short! Time not sufficient to read slice. Automatically adjusted to " << m_Parameters->m_SignalGen.m_tEcho << " ms";
      m_Parameters->m_Misc.m_AfterSimulationMessage += "Echo time was chosen too short! Time not sufficient to read slice. Internally adjusted to " + boost::lexical_cast<std::string>(m_Parameters->m_SignalGen.m_tEcho) + " ms\n";
    }
  }

protected:

};

}

#endif

