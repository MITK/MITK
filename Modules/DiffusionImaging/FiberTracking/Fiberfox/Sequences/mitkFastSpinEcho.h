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

#ifndef _MITK_FastSpinEcho_H
#define _MITK_FastSpinEcho_H

#include <mitkAcquisitionType.h>

namespace mitk {

/**
  * \brief Fast spin echo sequence. Cartesian readout.
  * Echo spacing = TE
  */
class FastSpinEcho : public AcquisitionType
{
public:

  unsigned int linesWithSameTime;
  float half_read_time;
  FastSpinEcho(FiberfoxParameters* parameters)
    : AcquisitionType(parameters)
  {
    linesWithSameTime = static_cast<unsigned int>(std::ceil(static_cast<float>(kyMax)/m_Parameters->m_SignalGen.m_EchoTrainLength));

    dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

    half_read_time = kxMax * dt/2;
  }
  ~FastSpinEcho() override
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

  // depends on ETL
  float GetTimeFromRf(const int& tick) override
  {
    float echo_nr = std::ceil(static_cast<float>(tick/kxMax+1)/linesWithSameTime);
    echo_nr -= std::ceil(static_cast<float>(m_Parameters->m_SignalGen.m_EchoTrainLength)/2);
    return m_Parameters->m_SignalGen.m_tEcho + echo_nr*m_Parameters->m_SignalGen.m_tLine + GetTimeFromMaxEcho(tick);
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
    if ( m_Parameters->m_SignalGen.m_tEcho < m_Parameters->m_SignalGen.m_EchoTrainLength*m_Parameters->m_SignalGen.m_tLine )
    {
      m_Parameters->m_SignalGen.m_tEcho = m_Parameters->m_SignalGen.m_EchoTrainLength*m_Parameters->m_SignalGen.m_tLine;
      MITK_WARN << "Echo time is too short! Time not sufficient to read slice. Automatically adjusted to " << m_Parameters->m_SignalGen.m_tEcho << " ms";
      m_Parameters->m_Misc.m_AfterSimulationMessage += "Echo time was chosen too short! Time not sufficient to read slice. Internally adjusted to " + boost::lexical_cast<std::string>(m_Parameters->m_SignalGen.m_tEcho) + " ms\n";
    }
  }

protected:

};

}

#endif

