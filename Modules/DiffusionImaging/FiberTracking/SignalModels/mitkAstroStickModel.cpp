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
#include <vnl/vnl_cross.h>
#include <vnl/vnl_quaternion.h>
#include <mitkAstroStickModel.h>

using namespace mitk;

template< class ScalarType >
AstroStickModel< ScalarType >::AstroStickModel()
    : m_Diffusivity(0.001)
    , m_BValue(1000)
    , m_NumSticks(42)
    , m_RandomizeSticks(false)
{
    m_RandGen = ItkRandGenType::New();

    vnl_matrix_fixed<ScalarType,3,42>* sticks = itk::PointShell<42, vnl_matrix_fixed<ScalarType, 3, 42> >::DistributePointShell();
    for (unsigned int i=0; i<m_NumSticks; i++)
    {
        GradientType stick;
        stick[0] = sticks->get(0,i); stick[1] = sticks->get(1,i); stick[2] = sticks->get(2,i);
        stick.Normalize();
        m_Sticks.push_back(stick);
    }
}

template< class ScalarType >
AstroStickModel< ScalarType >::~AstroStickModel()
{

}

template< class ScalarType >
void AstroStickModel< ScalarType >::SetSeed(int s)
{
    m_RandGen->SetSeed(s);
}

template< class ScalarType >
ScalarType AstroStickModel< ScalarType >::SimulateMeasurement(int dir)
{
    ScalarType signal = 0;

    if (dir>=this->m_GradientList.size())
        return signal;

    ScalarType b = -m_BValue*m_Diffusivity;

    if (m_RandomizeSticks)
        m_NumSticks = 30 + m_RandGen->GetIntegerVariate()%31;

    GradientType g = this->m_GradientList[dir];
    ScalarType bVal = g.GetNorm(); bVal *= bVal;

    if (bVal>0.0001)
    {
        for (int j=0; j<m_NumSticks; j++)
        {
            ScalarType dot = 0;
            if(m_RandomizeSticks)
                dot = GetRandomDirection()*g;
            else
                dot = m_Sticks[j]*g;
            signal += exp( b*bVal*dot*dot );
        }
        signal /= m_NumSticks;
    }
    else
        signal = 1;

    return signal;
}

template< class ScalarType >
typename AstroStickModel< ScalarType >::GradientType AstroStickModel< ScalarType >::GetRandomDirection()
{
    GradientType vec;
    vec[0] = m_RandGen->GetNormalVariate();
    vec[1] = m_RandGen->GetNormalVariate();
    vec[2] = m_RandGen->GetNormalVariate();
    vec.Normalize();
    return vec;
}

template< class ScalarType >
typename AstroStickModel< ScalarType >::PixelType AstroStickModel< ScalarType >::SimulateMeasurement()
{
    PixelType signal;
    signal.SetSize(this->m_GradientList.size());
    ScalarType b = -m_BValue*m_Diffusivity;

    if (m_RandomizeSticks)
        m_NumSticks = 30 + m_RandGen->GetIntegerVariate()%31;

    for( unsigned int i=0; i<this->m_GradientList.size(); i++)
    {
        GradientType g = this->m_GradientList[i];
        ScalarType bVal = g.GetNorm(); bVal *= bVal;

        if (bVal>0.0001)
        {
            for (int j=0; j<m_NumSticks; j++)
            {
                ScalarType dot = 0;
                if(m_RandomizeSticks)
                    dot = GetRandomDirection()*g;
                else
                    dot = m_Sticks[j]*g;
                signal[i] += exp( b*bVal*dot*dot );
            }
            signal[i] /= m_NumSticks;
        }
        else
            signal[i] = 1;
    }

    return signal;
}
