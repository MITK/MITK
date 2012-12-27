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

#ifndef _MITK_T2SmearingArtifact_H
#define _MITK_T2SmearingArtifact_H

#include <MitkDiffusionImagingExports.h>
#include <mitkKspaceArtifact.h>
#include <itkImageRegion.h>
#include <itkImageRegionIterator.h>

namespace mitk {

/**
  * \brief Class to add gibbs ringing artifact to input complex slice
  *
  */
template< class ScalarType >
class T2SmearingArtifact : public KspaceArtifact< ScalarType >
{
public:

    T2SmearingArtifact();
    ~T2SmearingArtifact();

    typedef typename KspaceArtifact< ScalarType >::ComplexSliceType ComplexSliceType;

    /** Attenuate signal according to given T2 time. **/
    typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice);

    void SetReadoutPulseLength(double t){ m_ReadoutPulseLength=t; }

protected:

    unsigned int    m_ReadoutPulseLength;

};

#include "mitkT2SmearingArtifact.cpp"

}

#endif

