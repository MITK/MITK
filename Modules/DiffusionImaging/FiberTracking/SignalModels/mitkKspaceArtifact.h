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

#include <FiberTrackingExports.h>
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

    enum PhaseDirections {
      X,
      Y
    };

    KspaceArtifact()
        : m_T2(2000)
        , m_T1(4000)
        , m_TE(20)
        , m_T2star(50)
        , m_PhaseDirection(Y)
    {
    }
    ~KspaceArtifact(){}

    typedef typename itk::FFTRealToComplexConjugateImageFilter< ScalarType, 2 >::OutputImageType ComplexSliceType;

    /** Adds artifact according to model to the input slice. Has to be implemented in subclass. **/
    virtual typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice) = 0;

    void SetT1(unsigned int T1){ m_T1=T1; }
    void SetT2(unsigned int T2){ m_T2=T2; }
    void SetTE(unsigned int TE){ m_TE=TE; }
    void SetT2star(unsigned int T2star){ m_T2star=T2star; }

protected:

    unsigned int    m_T2star;
    unsigned int    m_T2;
    unsigned int    m_T1;
    unsigned int    m_TE;
    PhaseDirections m_PhaseDirection;
};

}

#endif

