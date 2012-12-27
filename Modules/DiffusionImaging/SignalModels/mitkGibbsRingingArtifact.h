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

#ifndef _MITK_GibbsRingingArtifact_H
#define _MITK_GibbsRingingArtifact_H

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
class GibbsRingingArtifact : public KspaceArtifact< ScalarType >
{
public:

    GibbsRingingArtifact();
    ~GibbsRingingArtifact();

    typedef typename KspaceArtifact< ScalarType >::ComplexSliceType ComplexSliceType;

    /** Adds Gibbs ringing to the input slice (by zero padding). **/
    typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice);

    void SetKspaceCropping(int factor){ m_KspaceCropping=factor; }
    int GetKspaceCropping(){ return m_KspaceCropping; }

protected:

    int  m_KspaceCropping;

};

#include "mitkGibbsRingingArtifact.cpp"

}

#endif

