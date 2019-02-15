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

#include "mitkEnergyComputer.h"
#include <vnl/vnl_copy.h>
#include <itkNumericTraits.h>

using namespace mitk;

EnergyComputer::EnergyComputer(ItkFloatImageType* mask, ParticleGrid* particleGrid, SphereInterpolator* interpolator, ItkRandGenType* randGen)
    : m_UseTrilinearInterpolation(true)
{
    m_ParticleGrid = particleGrid;
    m_RandGen = randGen;
    m_SphereInterpolator = interpolator;
    m_Mask = mask;

    m_ParticleLength = m_ParticleGrid->m_ParticleLength;
    m_SquaredParticleLength = m_ParticleLength*m_ParticleLength;

    m_Size[0] = mask->GetLargestPossibleRegion().GetSize()[0];
    m_Size[1] = mask->GetLargestPossibleRegion().GetSize()[1];
    m_Size[2] = mask->GetLargestPossibleRegion().GetSize()[2];

    if (m_Size[0]<3 || m_Size[1]<3 || m_Size[2]<3)
        m_UseTrilinearInterpolation = false;

    m_Spacing[0] = mask->GetSpacing()[0];
    m_Spacing[1] = mask->GetSpacing()[1];
    m_Spacing[2] = mask->GetSpacing()[2];

    // get rotation matrix
    vnl_matrix<double> temp = mask->GetDirection().GetVnlMatrix();
    vnl_matrix<float>  directionMatrix; directionMatrix.set_size(3,3);
    vnl_copy(temp, directionMatrix);
    vnl_matrix_fixed<float, 3, 3> I = directionMatrix*directionMatrix.transpose();
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            I[i][j] = fabs(I[i][j]);
    if(!I.is_identity(0.001))
    {
        fprintf(stderr,"EnergyComputer: image direction is not a rotation matrix. Tracking not possible!\n");
        std::cout << directionMatrix << std::endl;
    }
    m_RotationMatrix = directionMatrix;

    if (ODF_SAMPLING_SIZE != m_SphereInterpolator->nverts)
        fprintf(stderr,"EnergyComputer: error during init: data does not match with interpolation scheme\n");

    int totsz = m_Size[0]*m_Size[1]*m_Size[2];
    m_CumulatedSpatialProbability.resize(totsz + 1, 0.0);
    m_ActiveIndices.resize(totsz, 0);

    // calculate active voxels and cumulate probabilities
    m_NumActiveVoxels = 0;
    m_CumulatedSpatialProbability[0] = 0;
    for (int x = 0; x < m_Size[0];x++)
        for (int y = 0; y < m_Size[1];y++)
            for (int z = 0; z < m_Size[2];z++)
            {
                int idx = x+(y+z*m_Size[1])*m_Size[0];
                ItkFloatImageType::IndexType index;
                index[0] = x; index[1] = y; index[2] = z;
                if (m_Mask->GetPixel(index) > 0.5)
                {
                    m_CumulatedSpatialProbability[m_NumActiveVoxels+1] = m_CumulatedSpatialProbability[m_NumActiveVoxels] + m_Mask->GetPixel(index);
                    m_ActiveIndices[m_NumActiveVoxels] = idx;
                    m_NumActiveVoxels++;
                }
            }
    for (int k = 0; k < m_NumActiveVoxels; k++)
        m_CumulatedSpatialProbability[k] /= m_CumulatedSpatialProbability[m_NumActiveVoxels];

    std::cout << "EnergyComputer: " << m_NumActiveVoxels << " active voxels found" << std::endl;
}

void EnergyComputer::SetParameters(float particleWeight, float particleWidth, float connectionPotential, float curvThres, float inexBalance, float particlePotential)
{
    m_ParticleChemicalPotential = particlePotential;
    m_ConnectionPotential = connectionPotential;
    m_ParticleWeight = particleWeight;

    float bal = 1/(1+exp(-inexBalance));
    m_ExtStrength = 2*bal;
    m_IntStrength = 2*(1-bal)/m_SquaredParticleLength;

    m_CurvatureThreshold = curvThres;

    float sigma_s = particleWidth;
    m_SigmaInv = 1/(2*sigma_s*sigma_s);
}

// draw random position from active voxels
void EnergyComputer::DrawRandomPosition(vnl_vector_fixed<float, 3>& R)
{
    float r = static_cast<float>(m_RandGen->GetVariate());
    int j;
    int rl = 1;
    int rh = m_NumActiveVoxels;
    while(rh != rl)
    {
        j = rl + (rh-rl)/2;
        if (r < m_CumulatedSpatialProbability[j])
        {
            rh = j;
            continue;
        }
        if (r > m_CumulatedSpatialProbability[j])
        {
            rl = j+1;
            continue;
        }
        break;
    }
    R[0] = m_Spacing[0]*((float)(m_ActiveIndices[rh-1] % m_Size[0])  + m_RandGen->GetVariate());
    R[1] = m_Spacing[1]*((float)((m_ActiveIndices[rh-1]/m_Size[0]) % m_Size[1])  + m_RandGen->GetVariate());
    R[2] = m_Spacing[2]*((float)(m_ActiveIndices[rh-1]/(m_Size[0]*m_Size[1]))    + m_RandGen->GetVariate());
}

// return spatial probability of position
float EnergyComputer::SpatProb(vnl_vector_fixed<float, 3> pos)
{
    ItkFloatImageType::IndexType index;
    index[0] = floor(pos[0]/m_Spacing[0]);
    index[1] = floor(pos[1]/m_Spacing[1]);
    index[2] = floor(pos[2]/m_Spacing[2]);

    if (m_Mask->GetLargestPossibleRegion().IsInside(index)) // is inside image?
        return m_Mask->GetPixel(index);
    else
        return 0;
}

float EnergyComputer::CalcI0(float x)
{
    //    BESSEL_APPROXCOEFF[0] = -0.1714;
    //    BESSEL_APPROXCOEFF[1] = 0.5332;
    //    BESSEL_APPROXCOEFF[2] = -1.4889;
    //    BESSEL_APPROXCOEFF[3] = 2.0389;
    float y = x*x;
    float erg = -0.1714;
    erg += y*0.5332;
    erg += y*y*-1.4889;
    erg += y*y*y*2.0389;
    return erg;
}

float EnergyComputer::mexp(float x)
{
    return((x>=7.0) ? 0 : ((x>=5.0) ? (-0.0029*x+0.0213) : ((x>=3.0) ? (-0.0215*x+0.1144) : ((x>=2.0) ? (-0.0855*x+0.3064) : ((x>=1.0) ? (-0.2325*x+0.6004) : ((x>=0.5) ? (-0.4773*x+0.8452) : ((x>=0.0) ? (-0.7869*x+1.0000) : 1 )))))));
    //  return exp(-x);
}

int EnergyComputer::GetNumActiveVoxels()
{
    return m_NumActiveVoxels;
}
