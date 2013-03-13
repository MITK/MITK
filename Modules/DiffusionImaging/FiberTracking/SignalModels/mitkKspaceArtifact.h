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

    KspaceArtifact()
        : m_T2(2000)
        , m_TE(100)
        , m_Tinhom(50)
        , m_LineReadoutTime(1)
    {
    }
    ~KspaceArtifact(){}

    typedef typename itk::FFTRealToComplexConjugateImageFilter< ScalarType, 2 >::OutputImageType ComplexSliceType;

    /** Adds artifact according to model to the input slice. Has to be implemented in subclass. **/
    virtual typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice) = 0;

    // all times stored internally in µ seconds, input in milliseconds
    void SetTline(double LineReadoutTime){ m_LineReadoutTime=LineReadoutTime; }
    void SetTE(double TE){ m_TE=TE; }
    void SetT2(double T2){ m_T2=T2; }
    void SetTinhom(unsigned int Tinhom){ m_Tinhom=Tinhom; }

protected:

    double    m_Tinhom;
    double    m_T2;
    double    m_TE;
    double    m_LineReadoutTime;
};

}

#endif

