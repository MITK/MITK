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

    SingleShotEpi(FiberfoxParameters<double>* parameters) : AcquisitionType(parameters)
    {
        kxMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0);
        kyMax = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1);

        dt =  m_Parameters->m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

        // k-space center at maximum echo
        if ( kyMax%2==0 )
        {
            m_TEhalf = -m_Parameters->m_SignalGen.m_tLine*(kyMax-1)/2 + dt*(kxMax-(int)kxMax%2)/2;
        }
        else
            m_TEhalf = -m_Parameters->m_SignalGen.m_tLine*(kyMax-1)/2 - dt*(kxMax-(int)kxMax%2)/2;
    }
    ~SingleShotEpi()
    {}

    double GetTimeFromMaxEcho(itk::Index< 2 > index)
    {
        double t = 0;
        t = m_TEhalf + ((double)index[1]*kxMax+(double)index[0])*dt;
        return t;
    }

    double GetRedoutTime(itk::Index< 2 > index)
    {
        double t = 0;
        t = ((double)index[1]*kxMax+(double)index[0])*dt;
        return t;
    }

    itk::Index< 2 > GetActualKspaceIndex(itk::Index< 2 > index)
    {
        // reverse phase
        if (!m_Parameters->m_SignalGen.m_ReversePhase)
            index[1] = kyMax-1-index[1];

        // reverse readout direction
        if (index[1]%2 == 1)
            index[0] = kxMax-index[0]-1;

        return index;
    }

    void AdjustEchoTime()
    {
        int temp = m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)*m_Parameters->m_SignalGen.m_PartialFourier - (m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)+m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1)%2)/2;

        if ( m_Parameters->m_SignalGen.m_tEcho/2 < temp*m_Parameters->m_SignalGen.m_tLine )
        {
            m_Parameters->m_SignalGen.m_tEcho = 2*temp*m_Parameters->m_SignalGen.m_tLine;
            MITK_WARN << "Echo time is too short! Time not sufficient to read slice. Automatically adjusted to " << m_Parameters->m_SignalGen.m_tEcho << " ms";
            m_Parameters->m_Misc.m_AfterSimulationMessage += "Echo time was chosen too short! Time not sufficient to read slice. Internally adjusted to " + boost::lexical_cast<string>(m_Parameters->m_SignalGen.m_tEcho) + " ms\n";
        }
    }

protected:

    double dt;
    int kxMax;
    int kyMax;

};

}

#endif

