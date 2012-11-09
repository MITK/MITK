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

#ifndef _ENCOMP
#define _ENCOMP

#include <MitkDiffusionImagingExports.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkParticleGrid.h>
#include <mitkSphereInterpolator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

using namespace mitk;

/**
* \brief Calculates internal and external energy of the new particle configuration proposal.   */

class MitkDiffusionImaging_EXPORT EnergyComputer
{

public:

    //typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
    //typedef itk::Image<OdfVectorType, 3>        ItkQBallImgType;
    typedef itk::Image<float, 3>                ItkFloatImageType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator ItkRandGenType;

    EnergyComputer(ItkFloatImageType* mask, ParticleGrid* particleGrid, SphereInterpolator* interpolator, ItkRandGenType* randGen);
    void SetParameters(float particleWeight, float particleWidth, float connectionPotential, float curvThres, float inexBalance, float particlePotential);

    // get random position inside mask
    void DrawRandomPosition(vnl_vector_fixed<float, 3>& R);

    // external energy calculation
    virtual float ComputeExternalEnergy(vnl_vector_fixed<float, 3>& R, vnl_vector_fixed<float, 3>& N, Particle* dp) =0;

    // internal energy calculation
    virtual float ComputeInternalEnergyConnection(Particle *p1,int ep1) = 0;
    virtual float ComputeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2) = 0;
    virtual float ComputeInternalEnergy(Particle *dp) = 0;

    int GetNumActiveVoxels();

protected:

    vnl_matrix_fixed<float, 3, 3>   m_RotationMatrix;
    SphereInterpolator*             m_SphereInterpolator;
    ParticleGrid*                   m_ParticleGrid;
    ItkRandGenType*                 m_RandGen;
//    ItkQBallImgType*                m_Image;
    ItkFloatImageType*              m_Mask;
    vnl_vector_fixed<int, 3>        m_Size;
    vnl_vector_fixed<float, 3>      m_Spacing;
    std::vector< float >            m_CumulatedSpatialProbability;
    std::vector< int >              m_ActiveIndices;    // indices inside mask

    bool    m_UseTrilinearInterpolation;    // is deactivated if less than 3 image slices are available
    int     m_NumActiveVoxels;              // voxels inside mask
    float   m_ConnectionPotential;          // larger value results in larger energy value -> higher proposal acceptance probability
    float   m_ParticleChemicalPotential;    // larger value results in larger energy value -> higher proposal acceptance probability
    float   gamma_s;
    float   gamma_reg_s;
    float   m_ParticleWeight;               // defines how much one particle contributes to the artificial signal
    float   m_ExtStrength;                  // weighting factor for external energy
    float   m_IntStrength;                  // weighting factor for internal energy
    float   m_ParticleLength;               // particle length
    float   m_SquaredParticleLength;        // squared particle length
    float   m_CurvatureThreshold;           // maximum angle accepted between two connected particles

    float SpatProb(vnl_vector_fixed<float, 3> pos);
    float EvaluateOdf(vnl_vector_fixed<float, 3> &pos, vnl_vector_fixed<float, 3> dir);
    float mbesseli0(float x);
    float mexp(float x);
};

#endif
