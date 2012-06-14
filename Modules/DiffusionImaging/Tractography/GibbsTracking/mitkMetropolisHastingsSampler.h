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

#ifndef _SAMPLER
#define _SAMPLER

// MITK
#include <MitkDiffusionImagingExports.h>
#include <mitkParticleGrid.h>
#include <mitkEnergyComputer.h>
#include <MersenneTwister.h>
#include <mitkSimpSamp.h>

// ITK
#include <itkImage.h>

// MISC
#include <fstream>

namespace mitk
{

class MitkDiffusionImaging_EXPORT MetropolisHastingsSampler
{
public:

    typedef itk::Image< float, 3 >  ItkFloatImageType;

    MetropolisHastingsSampler(ParticleGrid* grid, EnergyComputer* enComp, MTRand* randGen, float curvThres);
    void SetTemperature(float val);

    void MakeProposal();
    int GetNumAcceptedProposals();

protected:

    // connection proposal related methods
    void ImplementTrack(Track& T);
    void RemoveAndSaveTrack(EndPoint P);
    void MakeTrackProposal(EndPoint P);
    void ComputeEndPointProposalDistribution(EndPoint P);

    // generate random vectors
    vnl_vector_fixed<float, 3> DistortVector(float sigma, vnl_vector_fixed<float, 3>& vec);
    vnl_vector_fixed<float, 3> GetRandomDirection();

    MTRand*     m_RandGen;
    Track       m_ProposalTrack;
    Track       m_BackupTrack;
    SimpSamp    m_SimpSamp;

    float m_InTemp;
    float m_ExTemp;
    float m_Density;

    float m_BirthProb;
    float m_DeathProb;
    float m_ShiftProb;
    float m_OptShiftProb;
    float m_ConnectionProb;

    float m_Sigma;
    float m_Gamma;
    float m_Z;

    float m_DistanceThreshold;
    float m_CurvatureThreshold;
    float m_TractProb;
    float m_StopProb;
    float m_DelProb;
    float m_ParticleLength;
    float m_ChempotParticle;

    ParticleGrid*   m_ParticleGrid;
    EnergyComputer* m_EnergyComputer;
    int             m_AcceptedProposals;
};

}

#endif


