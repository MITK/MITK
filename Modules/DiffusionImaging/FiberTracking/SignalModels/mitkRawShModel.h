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

#ifndef _MITK_RawShModel_H
#define _MITK_RawShModel_H

#include <mitkDiffusionSignalModel.h>

namespace mitk {

/**
  * \brief The spherical harmonic representation of a prototype diffusion weighted MR signal is used to obtain the direction dependent signal.
  *
  */

template< class ScalarType >
class RawShModel : public DiffusionSignalModel< ScalarType >
{
public:

    RawShModel();
    ~RawShModel();

    typedef typename DiffusionSignalModel< ScalarType >::PixelType          PixelType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientType       GradientType;
    typedef typename DiffusionSignalModel< ScalarType >::GradientListType   GradientListType;
    typedef itk::Matrix< double, 3, 3 >                                     MatrixType;

    /** Actual signal generation **/
    PixelType SimulateMeasurement();
    ScalarType SimulateMeasurement(unsigned int dir);

    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; Cart2Sph(); }
    bool SetShCoefficients(vnl_vector< double > shCoefficients);
    void SetFiberDirection(GradientType fiberDirection);
    void SetB0Signal(ScalarType signal){ m_B0Signal = signal; }

protected:

    void Cart2Sph();

    vnl_vector< double >            m_ShCoefficients;
    GradientType                    m_PrototypeMaxDirection;
    vnl_matrix<double>              m_SphCoords;
    int                             m_ShOrder;
    GradientListType                m_RotatedGradientList;
    ScalarType                      m_B0Signal;
};

}

#include "mitkRawShModel.cpp"

#endif

