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
    : m_BValue(1000)
    , m_Diffusivity(0.001)
    , m_NumSticks(42)
    , m_RandomizeSticks(false)
{
    this->m_RandGen = itk::Statistics::MersenneTwisterRandomVariateGenerator::New();
    this->m_RandGen->SetSeed();

    vnl_matrix_fixed<double,3,42>* sticks = itk::PointShell<42, vnl_matrix_fixed<double, 3, 42> >::DistributePointShell();
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
ScalarType AstroStickModel< ScalarType >::SimulateMeasurement(unsigned int dir)
{
    ScalarType signal = 0;

    if (dir>=this->m_GradientList.size())
        return signal;

    ScalarType b = -m_BValue*m_Diffusivity;

    if (m_RandomizeSticks)  // random number of sticks
        m_NumSticks = 30 + this->m_RandGen->GetIntegerVariate()%31;

    GradientType g = this->m_GradientList[dir];
    ScalarType bVal = g.GetNorm(); bVal *= bVal;

    if (bVal>0.0001)    // is weighted direction
    {
        for (unsigned int j=0; j<m_NumSticks; j++)
        {
            ScalarType dot = 0;
            if(m_RandomizeSticks)
                dot = GetRandomDirection()*g;
            else
                dot = m_Sticks[j]*g;
            signal += std::exp( (double)(b*bVal*dot*dot) );
        }
        signal /= m_NumSticks;
    }
    else    // is baseline direction
        signal = 1;

    return signal;
}

template< class ScalarType >
typename AstroStickModel< ScalarType >::GradientType AstroStickModel< ScalarType >::GetRandomDirection()
{
    GradientType vec;
    vec[0] = this->m_RandGen->GetNormalVariate();
    vec[1] = this->m_RandGen->GetNormalVariate();
    vec[2] = this->m_RandGen->GetNormalVariate();
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
        m_NumSticks = 30 + this->m_RandGen->GetIntegerVariate()%31;

    for( unsigned int i=0; i<this->m_GradientList.size(); i++)
    {
        GradientType g = this->m_GradientList[i];
        ScalarType bVal = g.GetNorm(); bVal *= bVal;

        if (bVal>0.0001)
        {
            for (unsigned int j=0; j<m_NumSticks; j++)
            {
                ScalarType dot = 0;
                if(m_RandomizeSticks)
                    dot = GetRandomDirection()*g;
                else
                    dot = m_Sticks[j]*g;
                signal[i] += std::exp( (double)(b*bVal*dot*dot) );
            }
            signal[i] /= m_NumSticks;
        }
        else
            signal[i] = 1;
    }

    return signal;
}
