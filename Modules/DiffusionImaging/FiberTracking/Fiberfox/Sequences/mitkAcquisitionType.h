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

#ifndef _MITK_KspaceReadout_H
#define _MITK_KspaceReadout_H

#include <mitkFiberfoxParameters.h>
#include <itkSize.h>

namespace mitk {

/**
  * \brief Abstract class for k-space acquisiton type (k-space trajectory and echo placement)
  *
  */
class AcquisitionType
{
public:

    AcquisitionType(FiberfoxParameters<double>* parameters)
    {
        m_Parameters = parameters;
    }
    virtual ~AcquisitionType(){}

    virtual double GetTimeFromMaxEcho(itk::Index< 2 > index) = 0;               ///< Time from maximum echo intensity in milliseconds
    virtual double GetRedoutTime(itk::Index< 2 > index) = 0;                    ///< Time passed since readout started in milliseconds
    virtual itk::Index< 2 > GetActualKspaceIndex(itk::Index< 2 > index) = 0;    ///< Transfer simple image iterator index to desired k-space index (depends on k-space readout scheme)
    virtual void AdjustEchoTime() = 0;                                          ///< Depending on the k-space readout scheme and acquisition parameters the minimum TE varies. This has to be checked and adjusted in this method.

protected:

    double                       m_TEhalf;
    FiberfoxParameters<double>*  m_Parameters;
    itk::Size< 2 >               m_Size;
};

}

#endif

