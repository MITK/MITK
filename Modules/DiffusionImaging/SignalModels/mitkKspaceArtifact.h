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

#ifndef _MITK_KspaceArtifact_H
#define _MITK_KspaceArtifact_H

#include <MitkDiffusionImagingExports.h>
#include <itkImage.h>
#include <itkFFTRealToComplexConjugateImageFilter.h>

namespace mitk {

/**
  * \brief Abstract class for diffusion noise models
  *
  */

template< class ScalarType >
class KspaceArtifact
{
public:

    KspaceArtifact(){ m_T2=90; }
    ~KspaceArtifact(){}

    typedef typename itk::FFTRealToComplexConjugateImageFilter< ScalarType, 2 >::OutputImageType ComplexSliceType;

    /** Adds artifact according to model to the input slice. Has to be implemented in subclass. **/
    virtual typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice) = 0;

    void SetRelaxationT2(unsigned int T2){ m_T2=T2; }

protected:

    unsigned int  m_T2;

};

}

#endif

