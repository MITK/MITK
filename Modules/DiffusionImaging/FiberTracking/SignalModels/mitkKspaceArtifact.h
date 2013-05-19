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
#include <itkVnlForwardFFTImageFilter.h>

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
        : m_TE(100)
        , m_Tinhom(50)
        , m_LineReadoutTime(1)
    {
    }
    ~KspaceArtifact(){}

    typedef itk::Image< ScalarType, 2 > WorkImageType;
    typedef typename itk::VnlForwardFFTImageFilter< WorkImageType >::OutputImageType ComplexSliceType;

    /** Adds artifact according to model to the input slice. Has to be implemented in subclass. **/
    virtual typename ComplexSliceType::Pointer AddArtifact(typename ComplexSliceType::Pointer slice) = 0;

    void SetTline(double LineReadoutTime){ m_LineReadoutTime=LineReadoutTime; }
    void SetTE(double TE){ m_TE=TE; }
    void SetTinhom(unsigned int Tinhom){ m_Tinhom=Tinhom; }

protected:

    double    m_Tinhom;
    double    m_TE;
    double    m_LineReadoutTime;
};

}

#endif

