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

  FastSpinEcho(FiberfoxParameters* parameters)
    : AcquisitionType(parameters)
  {
    m_LinesWithSameTime = static_cast<unsigned int>(std::ceil(static_cast<float>(kyMax)/m_Parameters->m_SignalGen.m_EchoTrainLength));

    dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

    // maximum echo at center of each line
    m_NegTEhalf = -dt*(kxMax-kxMax%2)/2;
  }
  ~FastSpinEcho() override
  {}

  // one echo per k-space line
  float GetTimeFromMaxEcho(const itk::Index< 2 >& index) override
  {
    return m_NegTEhalf + static_cast<float>(index[0])*dt;
  }

  // time since current readout pulse started
  float GetRedoutTime(const itk::Index< 2 >& index) override
  {
    return static_cast<float>(index[0])*dt;
  }

  // depends on ETL
  float GetTimeFromRf(const itk::Index< 2 >& index) override
  {
    return m_Parameters->m_SignalGen.m_tEcho*std::ceil(static_cast<float>(index[1]+1)/m_LinesWithSameTime) + GetTimeFromMaxEcho(index);
  }

  itk::Index< 2 > GetActualKspaceIndex(const itk::Index< 2 >& index) override
  {
    itk::Index< 2 > out_idx = index;
    // reverse phase
    if (!m_Parameters->m_SignalGen.m_ReversePhase)
      out_idx[1] = kyMax-1-out_idx[1];

    return index;
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

  unsigned int m_LinesWithSameTime;

};

}

#endif

