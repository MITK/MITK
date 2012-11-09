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
#include <mitkSimpSamp.h>

// ITK
#include <itkImage.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

// MISC
#include <fstream>

namespace mitk
{

/**
* \brief Generates ne proposals of particle configurations.   */

class MitkDiffusionImaging_EXPORT MetropolisHastingsSampler
{
public:

    typedef itk::Image< float, 3 >  ItkFloatImageType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator ItkRandGenType;

    MetropolisHastingsSampler(ParticleGrid* grid, EnergyComputer* enComp, ItkRandGenType* randGen, float curvThres);
    void SetTemperature(float val);

    void MakeProposal();
    int GetNumAcceptedProposals();
    void SetProbabilities(float birth, float death, float shift, float optShift, float connect);

protected:

    // connection proposal related methods
    void ImplementTrack(Track& T);
    void RemoveAndSaveTrack(EndPoint P);
    void MakeTrackProposal(EndPoint P);
    void ComputeEndPointProposalDistribution(EndPoint P);

    // generate random vectors
    void DistortVector(float sigma, vnl_vector_fixed<float, 3>& vec);
    vnl_vector_fixed<float, 3> GetRandomDirection();

    ItkRandGenType* m_RandGen;      // random generator
    Track       m_ProposalTrack;    // stores proposal track
    Track       m_BackupTrack;      // stores track removed for new proposal traCK
    SimpSamp    m_SimpSamp;         // neighbouring particles and their probabilities for the local tracking

    float m_InTemp;     // simulated annealing temperature
    float m_ExTemp;     // simulated annealing temperature
    float m_Density;

    float m_BirthProb;          // probability for particle birth
    float m_DeathProb;          // probability for particle death
    float m_ShiftProb;          // probability for particle shift
    float m_OptShiftProb;       // probability for optimal particle shift
    float m_ConnectionProb;     // probability for particle connection proposal

    float m_Sigma;
    float m_Gamma;
    float m_Z;

    float m_DistanceThreshold;  // threshold for maximum distance between connected particles
    float m_CurvatureThreshold; // threshold for maximum angle between connected particles
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


