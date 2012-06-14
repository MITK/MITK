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

    ParticleGrid*   m_ParticleGrid;
    const int*      datasz;
    EnergyComputer* enc;
    int             m_Iterations;
    float           width;
    float           height;
    float           depth;
    int             m_AcceptedProposals;

    float m_InTemp;
    float m_ExTemp;
    float m_Density;

    float m_BirthProb;
    float m_DeathProb;
    float m_ShiftProb;
    float m_OptShiftProb;
    float m_ConnectionProb;

    float sigma_g;
    float gamma_g;
    float Z_g;

    float dthres;
    float nthres;
    float T_prop;
    float stopprobability;
    float del_prob;

    float len_def;
    float len_sig;

    float cap_def;
    float cap_sig;

    float externalEnergy;
    float internalEnergy;
    float m_ChempotParticle;

    MTRand* mtrand;
    Track TrackProposal, TrackBackup;
    SimpSamp simpsamp;

    MetropolisHastingsSampler(ParticleGrid* grid, MTRand* randGen);

    void SetEnergyComputer(EnergyComputer *e);
    void SetParameters(float Temp, int numit, float plen, float curv_hardthres, float chempot_particle);
    void SetTemperature(float temp);

    void Iterate(float* acceptance, unsigned long* numCon, unsigned long* numPart, bool *abort);
    void IterateOneStep();

    void ImplementTrack(Track& T);
    void RemoveAndSaveTrack(EndPoint P);
    void MakeTrackProposal(EndPoint P);
    void ComputeEndPointProposalDistribution(EndPoint P);

    vnl_vector_fixed<float, 3> distortn(float sigma, vnl_vector_fixed<float, 3>& vec);
    vnl_vector_fixed<float, 3> rand_sphere();
//    vnl_vector_fixed<float, 3> rand(float w, float h, float d);
};

}

#endif


