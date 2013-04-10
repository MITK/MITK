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

#ifndef _MITK_SignalDecay_H
#define _MITK_SignalDecay_H

#include <FiberTrackingExports.h>
#include <mitkKspaceArtifact.h>
#include <itkImageRegion.h>
#include <itkImageRegionIterator.h>

namespace mitk {

/**
  * \brief Class to add gibbs ringing artifact to input complex slice
  *
  */
template< class ScalarType >
class SignalDecay : public KspaceArtifact< ScalarType >
{
public:

    SignalDecay();
    ~SignalDecay();

    typedef typename KspaceArtifact< ScalarType >::ComplexSliceType ComplexSliceType;

    /** Attenuate signal according to given relaxation times. **/
    typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice);

protected:

};

#include "mitkSignalDecay.cpp"

}

#endif

