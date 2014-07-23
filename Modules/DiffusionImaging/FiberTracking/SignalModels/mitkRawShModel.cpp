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
#include <mitkRawShModel.h>
#include <boost/math/special_functions.hpp>
#include <itkOrientationDistributionFunction.h>
#include <mitkFiberfoxParameters.h>

using namespace mitk;
using namespace boost::math;

template< class ScalarType >
RawShModel< ScalarType >::RawShModel()
    : m_ShOrder(0)
{

}

template< class ScalarType >
RawShModel< ScalarType >::~RawShModel()
{

}

// convert cartesian to spherical coordinates
template< class ScalarType >
void RawShModel< ScalarType >::Cart2Sph()
{
    GradientListType tempDirs = m_RotatedGradientList;
    m_SphCoords.set_size(tempDirs.size(), 2);
    m_SphCoords.fill(0.0);

    for (unsigned int i=0; i<tempDirs.size(); i++)
    {
        GradientType g = tempDirs[i];
        if( g.GetNorm() > 0.0001 )
        {
            tempDirs[i].Normalize();
            m_SphCoords(i,0) = acos(tempDirs[i][2]); // theta
            m_SphCoords(i,1) = atan2(tempDirs[i][1], tempDirs[i][0]); // phi
        }
    }
}

template< class ScalarType >
void RawShModel< ScalarType >::SetFiberDirection(GradientType fiberDirection)
{
    this->m_FiberDirection = fiberDirection;
    this->m_FiberDirection.Normalize();

    GradientType unit; unit.Fill(0.0); unit[0] = 1;
    GradientType axis = itk::CrossProduct(this->m_FiberDirection, m_PrototypeMaxDirection);
    axis.Normalize();

    vnl_quaternion<double> rotation(axis.GetVnlVector(), acos(dot_product(this->m_FiberDirection.GetVnlVector(), m_PrototypeMaxDirection.GetVnlVector())));
    rotation.normalize();

    m_RotatedGradientList.clear();
    for (unsigned int i=0; i<this->m_GradientList.size(); i++)
    {
        GradientType dir = this->m_GradientList.at(i);
        dir.Normalize();
        vnl_vector_fixed< double, 3 > vnlDir = rotation.rotate(dir.GetVnlVector());
        dir[0] = vnlDir[0]; dir[1] = vnlDir[1]; dir[2] = vnlDir[2];
        dir.Normalize();
        m_RotatedGradientList.push_back(dir);
    }

    Cart2Sph();
}

template< class ScalarType >
bool RawShModel< ScalarType >::SetShCoefficients(vnl_vector< double > shCoefficients)
{
    m_ShCoefficients = shCoefficients;
    m_ShOrder = 2;
    while ( (m_ShOrder*m_ShOrder + m_ShOrder + 2)/2 + m_ShOrder <= m_ShCoefficients.size() )
        m_ShOrder += 2;
    m_ShOrder -= 2;

    itk::OrientationDistributionFunction<double, 10000> odf;
    m_RotatedGradientList.clear();
    for (unsigned int i=0; i<odf.GetNumberOfComponents(); i++)
    {
        GradientType dir; dir[0]=odf.GetDirection(i)[0]; dir[1]=odf.GetDirection(i)[1]; dir[2]=odf.GetDirection(i)[2];
        dir.Normalize();
        m_RotatedGradientList.push_back(dir);
    }
    Cart2Sph();
    PixelType signal = SimulateMeasurement();

    int minDirIdx = 0;
    double min = itk::NumericTraits<double>::max();
    for (unsigned int i=0; i<signal.GetSize(); i++)
    {
        if (signal[i]>m_B0Signal || signal[i]<0)
        {
            MITK_INFO << "Corrupted signal value detected. Kernel rejected.";
            return false;
        }
        if (signal[i]<min)
        {
            min = signal[i];
            minDirIdx = i;
        }
    }
    m_PrototypeMaxDirection = m_RotatedGradientList.at(minDirIdx);
    m_PrototypeMaxDirection.Normalize();
    return true;
}

template< class ScalarType >
ScalarType RawShModel< ScalarType >::SimulateMeasurement(unsigned int dir)
{
    ScalarType signal = 0;

    if (dir>=m_RotatedGradientList.size())
        return signal;

    int j, m; double mag, plm;
    vnl_vector< double > shBasis;
    shBasis.set_size(m_ShCoefficients.size());
    shBasis.fill(0.0);

    if (m_RotatedGradientList[dir].GetNorm()>0.001)
    {
        j=0;
        for (int l=0; l<=m_ShOrder; l=l+2)
            for (m=-l; m<=l; m++)
            {
                plm = legendre_p<double>(l,abs(m),cos(m_SphCoords(dir,0)));
                mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;

                if (m<0)
                    shBasis[j] = sqrt(2.0)*mag*cos(fabs((double)m)*m_SphCoords(dir,1));
                else if (m==0)
                    shBasis[j] = mag;
                else
                    shBasis[j] = pow(-1.0, m)*sqrt(2.0)*mag*sin(m*m_SphCoords(dir,1));

                j++;
            }
    }

    for (unsigned int i=0; i<m_ShCoefficients.size(); i++)
        signal += m_ShCoefficients[i]*shBasis[i];

    return signal;
}

template< class ScalarType >
typename RawShModel< ScalarType >::PixelType RawShModel< ScalarType >::SimulateMeasurement()
{
    PixelType signal;
    signal.SetSize(m_RotatedGradientList.size());

    int M = m_RotatedGradientList.size();
    int j, m; double mag, plm;

    vnl_matrix< double > shBasis;
    shBasis.set_size(M, m_ShCoefficients.size());
    shBasis.fill(0.0);

    for (int p=0; p<M; p++)
    {
        if (m_RotatedGradientList[p].GetNorm()>0.001)
        {
            j=0;
            for (int l=0; l<=m_ShOrder; l=l+2)
                for (m=-l; m<=l; m++)
                {
                    plm = legendre_p<double>(l,abs(m),cos(m_SphCoords(p,0)));
                    mag = sqrt((double)(2*l+1)/(4.0*M_PI)*factorial<double>(l-abs(m))/factorial<double>(l+abs(m)))*plm;

                    if (m<0)
                        shBasis(p,j) = sqrt(2.0)*mag*cos(fabs((double)m)*m_SphCoords(p,1));
                    else if (m==0)
                        shBasis(p,j) = mag;
                    else
                        shBasis(p,j) = pow(-1.0, m)*sqrt(2.0)*mag*sin(m*m_SphCoords(p,1));

                    j++;
                }


            double val = 0;
            for (unsigned int cidx=0; cidx<m_ShCoefficients.size(); cidx++)
                val += m_ShCoefficients[cidx]*shBasis(p,cidx);
            signal[p] = val;
//            if (val>m_B0Signal || val<0)
//            {
//                MITK_INFO << "SIGNALGEN ERROR: " << val;
//                val = 0;
//            }
        }
        else
            signal[p] = m_B0Signal;
    }

    return signal;
}
