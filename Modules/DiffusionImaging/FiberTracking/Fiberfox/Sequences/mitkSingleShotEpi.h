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

#include <mitkKspaceReadout.h>

namespace mitk {

/**
  * \brief Realizes EPI readout
  *
  */
class SingleShotEpi : public KspaceReadout
{
public:

    SingleShotEpi(FiberfoxParameters<double>& parameters) : KspaceReadout(parameters)
    {
        kxMax = m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(0);
        kyMax = m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1);

        dt =  m_Parameters.m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

        // k-space center at maximum echo
        if ( kyMax%2==0 )
        {
            m_TEhalf = -m_Parameters.m_SignalGen.m_tLine*(kyMax-1)/2 + dt*(kxMax-(int)kxMax%2)/2;
        }
        else
            m_TEhalf = -m_Parameters.m_SignalGen.m_tLine*(kyMax-1)/2 - dt*(kxMax-(int)kxMax%2)/2;
    }
    ~SingleShotEpi(){}

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
        if (!m_Parameters.m_SignalGen.m_ReversePhase)
            index[1] = kyMax-1-index[1];

        // reverse readout direction
        if (index[1]%2 == 1)
            index[0] = kxMax-index[0]-1;

        return index;
    }

protected:

    double dt;
    int kxMax;
    int kyMax;

};

}

#endif

