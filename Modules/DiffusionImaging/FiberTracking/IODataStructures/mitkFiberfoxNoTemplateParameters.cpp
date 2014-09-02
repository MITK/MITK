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

#include "mitkFiberfoxNoTemplateParameters.h"

void mitk::SignalGenerationParameters::GenerateGradientHalfShell()
{
    int NPoints = 2*m_NumGradients;
    m_GradientDirections.clear();

    m_NumBaseline = NPoints/20;
    if (m_NumBaseline==0)
        m_NumBaseline=1;

    GradientType g;
    g.Fill(0.0);
    for (unsigned int i=0; i<m_NumBaseline; i++)
        m_GradientDirections.push_back(g);

    if (NPoints==0)
        return;

    vnl_vector<double> theta; theta.set_size(NPoints);
    vnl_vector<double> phi; phi.set_size(NPoints);
    double C = sqrt(4*M_PI);
    phi(0) = 0.0;
    phi(NPoints-1) = 0.0;
    for(int i=0; i<NPoints; i++)
    {
        theta(i) = acos(-1.0+2.0*i/(NPoints-1.0)) - M_PI / 2.0;
        if( i>0 && i<NPoints-1)
        {
            phi(i) = (phi(i-1) + C /
                      sqrt(NPoints*(1-(-1.0+2.0*i/(NPoints-1.0))*(-1.0+2.0*i/(NPoints-1.0)))));
        }
    }

    for(int i=0; i<NPoints; i++)
    {
        g[2] = sin(theta(i));
        if (g[2]<0)
            continue;
        g[0] = cos(theta(i)) * cos(phi(i));
        g[1] = cos(theta(i)) * sin(phi(i));
        m_GradientDirections.push_back(g);
    }
}

std::vector< int > mitk::SignalGenerationParameters::GetBaselineIndices()
{
    std::vector< int > result;
    for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
        if (m_GradientDirections.at(i).GetNorm()<0.0001)
            result.push_back(i);
    return result;
}

unsigned int mitk::SignalGenerationParameters::GetFirstBaselineIndex()
{
    for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
        if (m_GradientDirections.at(i).GetNorm()<0.0001)
            return i;
    return -1;
}

bool mitk::SignalGenerationParameters::IsBaselineIndex(unsigned int idx)
{
    if (m_GradientDirections.size()>idx && m_GradientDirections.at(idx).GetNorm()<0.0001)
        return true;
    return false;
}

unsigned int mitk::SignalGenerationParameters::GetNumWeightedVolumes()
{
    return m_NumGradients;
}

unsigned int mitk::SignalGenerationParameters::GetNumBaselineVolumes()
{
    return m_NumBaseline;
}

unsigned int mitk::SignalGenerationParameters::GetNumVolumes()
{
    return m_GradientDirections.size();
}

mitk::SignalGenerationParameters::GradientListType mitk::SignalGenerationParameters::GetGradientDirections()
{
    return m_GradientDirections;
}

mitk::SignalGenerationParameters::GradientType mitk::SignalGenerationParameters::GetGradientDirection(unsigned int i)
{
    return m_GradientDirections.at(i);
}

void mitk::SignalGenerationParameters::SetNumWeightedVolumes(int numGradients)
{
    m_NumGradients = numGradients;
    GenerateGradientHalfShell();
}

void mitk::SignalGenerationParameters::SetGradienDirections(GradientListType gradientList)
{
    m_GradientDirections = gradientList;
    m_NumGradients = 0;
    m_NumBaseline = 0;
    for( unsigned int i=0; i<this->m_GradientDirections.size(); i++)
    {
        if (m_GradientDirections.at(i).GetNorm()>0.0001)
            m_NumGradients++;
        else
            m_NumBaseline++;
    }
}

void mitk::SignalGenerationParameters::SetGradienDirections(mitk::DiffusionImage<short>::GradientDirectionContainerType::Pointer gradientList)
{
    m_NumGradients = 0;
    m_NumBaseline = 0;
    m_GradientDirections.clear();

    for( unsigned int i=0; i<gradientList->Size(); i++)
    {
        GradientType g;
        g[0] = gradientList->at(i)[0];
        g[1] = gradientList->at(i)[1];
        g[2] = gradientList->at(i)[2];
        m_GradientDirections.push_back(g);

        if (m_GradientDirections.at(i).GetNorm()>0.0001)
            m_NumGradients++;
        else
            m_NumBaseline++;
    }
}
