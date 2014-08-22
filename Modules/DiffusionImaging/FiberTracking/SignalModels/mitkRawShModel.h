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

    bool SetShCoefficients(vnl_vector< double > shCoefficients, ScalarType b0);
    void SetFiberDirection(GradientType fiberDirection);
    void SetGradientList(GradientListType gradientList) { this->m_GradientList = gradientList; }
    void SetFaRange(double min, double max){ m_FaRange.first = min; m_FaRange.second = max; }
    void SetAdcRange(double min, double max){ m_AdcRange.first = min; m_AdcRange.second = max; }
    void SetMaxNumKernels(unsigned int max){ m_MaxNumKernels = max; }
    unsigned int GetNumberOfKernels();
    std::pair< double, double > GetFaRange(){ return m_FaRange; }
    std::pair< double, double > GetAdcRange(){ return m_AdcRange; }
    unsigned int GetMaxNumKernels(){ return m_MaxNumKernels; }
    void Clear();
    vector< GradientType >          m_PrototypeMaxDirection;

protected:

    void Cart2Sph( GradientListType gradients );
    void RandomModel();

    std::pair< double, double >     m_AdcRange;
    std::pair< double, double >     m_FaRange;
    vector< vnl_vector< double > >  m_ShCoefficients;
    vector< ScalarType >            m_B0Signal;
    vnl_matrix<double>              m_SphCoords;
    unsigned int                    m_ShOrder;
    int                             m_ModelIndex;
    unsigned int                    m_MaxNumKernels;
};

}

#include "mitkRawShModel.cpp"

#endif

