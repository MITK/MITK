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

  AcquisitionType(FiberfoxParameters* parameters)
  {
    m_Parameters = parameters;
    kxMax = static_cast<int>(m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(0));
    kyMax = static_cast<int>(m_Parameters->m_SignalGen.m_CroppedRegion.GetSize(1));
  }
  virtual ~AcquisitionType(){}

  virtual float GetTimeFromMaxEcho(const itk::Index< 2 >& index) = 0;               ///< Time from maximum echo intensity in milliseconds
  virtual float GetRedoutTime(const itk::Index< 2 >& index) = 0;                    ///< Time passed since readout started in milliseconds
  virtual float GetTimeFromRf(const itk::Index< 2 >& index) = 0;                    ///< Time passed since RF pulse was applied in milliseconds
  virtual itk::Index< 2 > GetActualKspaceIndex(const itk::Index< 2 >& index) = 0;    ///< Transfer simple image iterator index to desired k-space index (depends on k-space readout scheme)
  virtual void AdjustEchoTime() = 0;                                          ///< Depending on the k-space readout scheme and acquisition parameters the minimum TE varies. This has to be checked and adjusted in this method.
  itk::Index< 2 > GetSymmetricIndex(const itk::Index< 2 >& index)
  {
    itk::Index< 2 > sym;
    sym[0] = kxMax-index[0]-1;
    sym[1] = kyMax-index[1]-1;
    return sym;
  }

protected:

  float                       m_NegTEhalf;  ///< negative time to read half the k-space (needed to calculate the ms from the maximum echo); THIS IS NOT THE WELL KNOWN TE/2 SCANNER PARAMETER
  FiberfoxParameters*         m_Parameters;
  itk::Size< 2 >              m_Size;

  float dt;   // time to read one k-space voxe
  int kxMax;
  int kyMax;
};

}

#endif

