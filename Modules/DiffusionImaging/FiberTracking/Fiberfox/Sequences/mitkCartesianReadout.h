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

#ifndef _MITK_CartesianReadout_H
#define _MITK_CartesianReadout_H

#include <mitkAcquisitionType.h>

namespace mitk {

/**
  * \brief Realizes EPI readout: one echo, maximum intensity in the k-space center, zig-zag trajectory
  *
  */
class CartesianReadout : public AcquisitionType
{
public:

    CartesianReadout(FiberfoxParameters<double>& parameters) : AcquisitionType(parameters)
    {
        kxMax = m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(0);
        kyMax = m_Parameters.m_SignalGen.m_CroppedRegion.GetSize(1);

        dt =  m_Parameters.m_SignalGen.m_tLine/kxMax;  // time to read one k-space voxel

        // maximum echo at center of each line
        m_TEhalf = -dt*(kxMax-(int)kxMax%2)/2;
    }
    ~CartesianReadout()
    {}

    double GetTimeFromMaxEcho(itk::Index< 2 > index)
    {
        double t = 0;
        t = m_TEhalf + (double)index[0]*dt;
        return t;
    }

    double GetRedoutTime(itk::Index< 2 > index)
    {
        double t = 0;
        t = (double)index[0]*dt;
        return t;
    }

    itk::Index< 2 > GetActualKspaceIndex(itk::Index< 2 > index)
    {
        // reverse phase
        if (!m_Parameters.m_SignalGen.m_ReversePhase)
            index[1] = kyMax-1-index[1];

        return index;
    }

protected:

    double dt;
    int kxMax;
    int kyMax;

};

}

#endif

