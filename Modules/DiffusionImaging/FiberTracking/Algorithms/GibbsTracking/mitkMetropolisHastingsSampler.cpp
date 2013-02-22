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

#include "mitkMetropolisHastingsSampler.h"

using namespace mitk;

MetropolisHastingsSampler::MetropolisHastingsSampler(ParticleGrid* grid, EnergyComputer* enComp, ItkRandGenType* randGen, float curvThres)
    : m_AcceptedProposals(0)
    , m_ExTemp(0.01)
    , m_BirthProb(0.25)
    , m_DeathProb(0.05)
    , m_ShiftProb(0.15)
    , m_OptShiftProb(0.1)
    , m_ConnectionProb(0.45)
    , m_TractProb(0.5)
    , m_DelProb(0.1)
    , m_ChempotParticle(0.0)
{
    m_RandGen = randGen;
    m_ParticleGrid = grid;
    m_EnergyComputer = enComp;

    m_ParticleLength = m_ParticleGrid->m_ParticleLength;
    m_DistanceThreshold = m_ParticleLength*m_ParticleLength;
    m_Sigma = m_ParticleLength/8.0;
    m_Gamma = 1/(m_Sigma*m_Sigma*2);
    m_Z = pow(2*M_PI*m_Sigma,3.0/2.0)*(M_PI*m_Sigma/m_ParticleLength);

    m_CurvatureThreshold = curvThres;
    m_StopProb = exp(-1/m_TractProb);
}

void MetropolisHastingsSampler::SetProbabilities(float birth, float death, float shift, float optShift, float connect)
{
    m_BirthProb = birth;
    m_DeathProb = death;
    m_ShiftProb = shift;
    m_OptShiftProb = optShift;
    m_ConnectionProb = connect;
    float sum = m_BirthProb+m_DeathProb+m_ShiftProb+m_OptShiftProb+m_ConnectionProb;
    if (sum!=1 && sum>mitk::eps)
    {
        m_BirthProb /= sum;
        m_DeathProb /= sum;
        m_ShiftProb /= sum;
        m_OptShiftProb /= sum;
        m_ConnectionProb /= sum;
    }
    std::cout << "Update proposal probabilities:" << std::endl;
    std::cout << "Birth: " << m_BirthProb << std::endl;
    std::cout << "Death: " << m_DeathProb << std::endl;
    std::cout << "Shift: " << m_ShiftProb << std::endl;
    std::cout << "Optimal shift: " << m_OptShiftProb << std::endl;
    std::cout << "Connection: " << m_ConnectionProb << std::endl;
}

// update temperature of simulated annealing process
void MetropolisHastingsSampler::SetTemperature(float val)
{
    m_InTemp = val;
    m_Density = exp(-m_ChempotParticle/m_InTemp);
}

// add small random number drawn from gaussian to each vector element
void MetropolisHastingsSampler::DistortVector(float sigma, vnl_vector_fixed<float, 3>& vec)
{
    vec[0] += m_RandGen->GetNormalVariate(0.0, sigma);
    vec[1] += m_RandGen->GetNormalVariate(0.0, sigma);
    vec[2] += m_RandGen->GetNormalVariate(0.0, sigma);
}

// generate normalized random vector
vnl_vector_fixed<float, 3> MetropolisHastingsSampler::GetRandomDirection()
{
    vnl_vector_fixed<float, 3> vec;
    vec[0] = m_RandGen->GetNormalVariate();
    vec[1] = m_RandGen->GetNormalVariate();
    vec[2] = m_RandGen->GetNormalVariate();
    vec.normalize();
    return vec;
}

// generate actual proposal (birth, death, shift and connection of particle)
void MetropolisHastingsSampler::MakeProposal()
{
    float randnum = m_RandGen->GetVariate();

    // Birth Proposal
    if (randnum < m_BirthProb)
    {
        vnl_vector_fixed<float, 3> R;
        m_EnergyComputer->DrawRandomPosition(R);
        vnl_vector_fixed<float, 3> N = GetRandomDirection();
        Particle prop;
        prop.pos = R;
        prop.dir = N;

        float prob =  m_Density * m_DeathProb /((m_BirthProb)*(m_ParticleGrid->m_NumParticles+1));

        float ex_energy = m_EnergyComputer->ComputeExternalEnergy(R,N,0);
        float in_energy = m_EnergyComputer->ComputeInternalEnergy(&prop);
        prob *= exp((in_energy/m_InTemp+ex_energy/m_ExTemp)) ;

        if (prob > 1 || m_RandGen->GetVariate() < prob)
        {
            Particle *p = m_ParticleGrid->NewParticle(R);
            if (p!=0)
            {
                p->pos = R;
                p->dir = N;
                m_AcceptedProposals++;
            }
        }
    }
    // Death Proposal
    else if (randnum < m_BirthProb+m_DeathProb)
    {
        if (m_ParticleGrid->m_NumParticles > 0)
        {
            int pnum = m_RandGen->GetIntegerVariate()%m_ParticleGrid->m_NumParticles;
            Particle *dp = m_ParticleGrid->GetParticle(pnum);
            if (dp->pID == -1 && dp->mID == -1)
            {
                float ex_energy = m_EnergyComputer->ComputeExternalEnergy(dp->pos,dp->dir,dp);
                float in_energy = m_EnergyComputer->ComputeInternalEnergy(dp);

                float prob = m_ParticleGrid->m_NumParticles * (m_BirthProb) /(m_Density*m_DeathProb); //*SpatProb(dp->R);
                prob *= exp(-(in_energy/m_InTemp+ex_energy/m_ExTemp)) ;
                if (prob > 1 || m_RandGen->GetVariate() < prob)
                {
                    m_ParticleGrid->RemoveParticle(pnum);
                    m_AcceptedProposals++;
                }
            }
        }

    }
    // Shift Proposal
    else  if (randnum < m_BirthProb+m_DeathProb+m_ShiftProb)
    {
        if (m_ParticleGrid->m_NumParticles > 0)
        {
            int pnum = m_RandGen->GetIntegerVariate()%m_ParticleGrid->m_NumParticles;
            Particle *p =  m_ParticleGrid->GetParticle(pnum);
            Particle prop_p = *p;

            DistortVector(m_Sigma, prop_p.pos);
            DistortVector(m_Sigma/(2*m_ParticleLength), prop_p.dir);
            prop_p.dir.normalize();


            float ex_energy = m_EnergyComputer->ComputeExternalEnergy(prop_p.pos,prop_p.dir,p)
                    - m_EnergyComputer->ComputeExternalEnergy(p->pos,p->dir,p);
            float in_energy = m_EnergyComputer->ComputeInternalEnergy(&prop_p) - m_EnergyComputer->ComputeInternalEnergy(p);

            float prob = exp(ex_energy/m_ExTemp+in_energy/m_InTemp);
            if (m_RandGen->GetVariate() < prob)
            {
                vnl_vector_fixed<float, 3> Rtmp = p->pos;
                vnl_vector_fixed<float, 3> Ntmp = p->dir;
                p->pos = prop_p.pos;
                p->dir = prop_p.dir;
                if (!m_ParticleGrid->TryUpdateGrid(pnum))
                {
                    p->pos = Rtmp;
                    p->dir = Ntmp;
                }
                m_AcceptedProposals++;
            }
        }
    }
    // Optimal Shift Proposal
    else  if (randnum < m_BirthProb+m_DeathProb+m_ShiftProb+m_OptShiftProb)
    {
        if (m_ParticleGrid->m_NumParticles > 0)
        {

            int pnum = m_RandGen->GetIntegerVariate()%m_ParticleGrid->m_NumParticles;
            Particle *p =  m_ParticleGrid->GetParticle(pnum);

            bool no_proposal = false;
            Particle prop_p = *p;
            if (p->pID != -1 && p->mID != -1)
            {
                Particle *plus = m_ParticleGrid->GetParticle(p->pID);
                int ep_plus = (plus->pID == p->ID)? 1 : -1;
                Particle *minus = m_ParticleGrid->GetParticle(p->mID);
                int ep_minus = (minus->pID == p->ID)? 1 : -1;
                prop_p.pos = (plus->pos + plus->dir * (m_ParticleLength * ep_plus)  + minus->pos + minus->dir * (m_ParticleLength * ep_minus));
                prop_p.pos *= 0.5;
                prop_p.dir = plus->pos - minus->pos;
                prop_p.dir.normalize();
            }
            else if (p->pID != -1)
            {
                Particle *plus = m_ParticleGrid->GetParticle(p->pID);
                int ep_plus = (plus->pID == p->ID)? 1 : -1;
                prop_p.pos = plus->pos + plus->dir * (m_ParticleLength * ep_plus * 2);
                prop_p.dir = plus->dir;
            }
            else if (p->mID != -1)
            {
                Particle *minus = m_ParticleGrid->GetParticle(p->mID);
                int ep_minus = (minus->pID == p->ID)? 1 : -1;
                prop_p.pos = minus->pos + minus->dir * (m_ParticleLength * ep_minus * 2);
                prop_p.dir = minus->dir;
            }
            else
                no_proposal = true;

            if (!no_proposal)
            {
                float cos = dot_product(prop_p.dir, p->dir);
                float p_rev = exp(-((prop_p.pos-p->pos).squared_magnitude() + (1-cos*cos))*m_Gamma)/m_Z;

                float ex_energy = m_EnergyComputer->ComputeExternalEnergy(prop_p.pos,prop_p.dir,p)
                        - m_EnergyComputer->ComputeExternalEnergy(p->pos,p->dir,p);
                float in_energy = m_EnergyComputer->ComputeInternalEnergy(&prop_p) - m_EnergyComputer->ComputeInternalEnergy(p);

                float prob = exp(ex_energy/m_ExTemp+in_energy/m_InTemp)*m_ShiftProb*p_rev/(m_OptShiftProb+m_ShiftProb*p_rev);

                if (m_RandGen->GetVariate() < prob)
                {
                    vnl_vector_fixed<float, 3> Rtmp = p->pos;
                    vnl_vector_fixed<float, 3> Ntmp = p->dir;
                    p->pos = prop_p.pos;
                    p->dir = prop_p.dir;
                    if (!m_ParticleGrid->TryUpdateGrid(pnum))
                    {
                        p->pos = Rtmp;
                        p->dir = Ntmp;
                    }
                    m_AcceptedProposals++;
                }
            }
        }
    }
    else
    {
        if (m_ParticleGrid->m_NumParticles > 0)
        {
            int pnum = m_RandGen->GetIntegerVariate()%m_ParticleGrid->m_NumParticles;
            Particle *p = m_ParticleGrid->GetParticle(pnum);

            EndPoint P;
            P.p = p;
            P.ep = (m_RandGen->GetVariate() > 0.5)? 1 : -1;

            RemoveAndSaveTrack(P);
            if (m_BackupTrack.m_Probability != 0)
            {
                MakeTrackProposal(P);

                float prob = (m_ProposalTrack.m_Energy-m_BackupTrack.m_Energy)/m_InTemp ;

                prob = exp(prob)*(m_BackupTrack.m_Probability * pow(m_DelProb,m_ProposalTrack.m_Length))
                        /(m_ProposalTrack.m_Probability * pow(m_DelProb,m_BackupTrack.m_Length));
                if (m_RandGen->GetVariate() < prob)
                {
                    ImplementTrack(m_ProposalTrack);
                    m_AcceptedProposals++;
                }
                else
                {
                    ImplementTrack(m_BackupTrack);
                }
            }
            else
                ImplementTrack(m_BackupTrack);
        }
    }
}

// establish connections between particles stored in input Track
void MetropolisHastingsSampler::ImplementTrack(Track &T)
{
    for (int k = 1; k < T.m_Length;k++)
        m_ParticleGrid->CreateConnection(T.track[k-1].p,T.track[k-1].ep,T.track[k].p,-T.track[k].ep);
}

// remove pending track from random particle, save it in m_BackupTrack and calculate its probability
void MetropolisHastingsSampler::RemoveAndSaveTrack(EndPoint P)
{
    EndPoint Current = P;
    int cnt = 0;
    float energy = 0;
    float AccumProb = 1.0;
    m_BackupTrack.track[cnt] = Current;
    EndPoint Next;

    for (;;)
    {
        Next.p = 0;
        if (Current.ep == 1)
        {
            if (Current.p->pID != -1)
            {
                Next.p = m_ParticleGrid->GetParticle(Current.p->pID);
                Current.p->pID = -1;
                m_ParticleGrid->m_NumConnections--;
            }
        }
        else if (Current.ep == -1)
        {
            if (Current.p->mID != -1)
            {
                Next.p = m_ParticleGrid->GetParticle(Current.p->mID);
                Current.p->mID = -1;
                m_ParticleGrid->m_NumConnections--;
            }
        }
        else
        { fprintf(stderr,"MetropolisHastingsSampler_randshift: Connection inconsistent 3\n"); break; }

        if (Next.p == 0) // no successor
        {
            Next.ep = 0; // mark as empty successor
            break;
        }
        else
        {
            if (Next.p->pID == Current.p->ID)
            {
                Next.p->pID = -1;
                Next.ep = 1;
            }
            else if (Next.p->mID == Current.p->ID)
            {
                Next.p->mID = -1;
                Next.ep = -1;
            }
            else
            { fprintf(stderr,"MetropolisHastingsSampler_randshift: Connection inconsistent 4\n"); break; }
        }

        ComputeEndPointProposalDistribution(Current);
        AccumProb *= (m_SimpSamp.probFor(Next));

        if (Next.p == 0) // no successor -> break
            break;

        energy += m_EnergyComputer->ComputeInternalEnergyConnection(Current.p,Current.ep,Next.p,Next.ep);

        Current = Next;
        Current.ep *= -1;
        cnt++;
        m_BackupTrack.track[cnt] = Current;

        if (m_RandGen->GetVariate() > m_DelProb)
            break;
    }
    m_BackupTrack.m_Energy = energy;
    m_BackupTrack.m_Probability = AccumProb;
    m_BackupTrack.m_Length = cnt+1;
}

// generate new track using kind of a local tracking starting from P in the given direction, store it in m_ProposalTrack and calculate its probability
void MetropolisHastingsSampler::MakeTrackProposal(EndPoint P)
{
    EndPoint Current = P;
    int cnt = 0;
    float energy = 0;
    float AccumProb = 1.0;
    m_ProposalTrack.track[cnt++] = Current;
    Current.p->label = 1;

    for (;;)
    {
        // next candidate is already connected
        if ((Current.ep == 1 && Current.p->pID != -1) || (Current.ep == -1 && Current.p->mID != -1))
            break;

        // track too long
//        if (cnt > 250)
//            break;

        ComputeEndPointProposalDistribution(Current);

        int k = m_SimpSamp.draw(m_RandGen->GetVariate());

        // stop tracking proposed
        if (k==0)
            break;

        EndPoint Next = m_SimpSamp.objs[k];
        float probability = m_SimpSamp.probFor(k);

        // accumulate energy and proposal distribution
        energy += m_EnergyComputer->ComputeInternalEnergyConnection(Current.p,Current.ep,Next.p,Next.ep);
        AccumProb *= probability;

        // track to next endpoint
        Current = Next;
        Current.ep *= -1;

        Current.p->label = 1;  // put label to avoid loops
        m_ProposalTrack.track[cnt++] = Current;
    }

    m_ProposalTrack.m_Energy = energy;
    m_ProposalTrack.m_Probability = AccumProb;
    m_ProposalTrack.m_Length = cnt;

    // clear labels
    for (int j = 0; j < m_ProposalTrack.m_Length;j++)
        m_ProposalTrack.track[j].p->label = 0;
}

// get neigbouring particles of P and calculate the according connection probabilities
void MetropolisHastingsSampler::ComputeEndPointProposalDistribution(EndPoint P)
{
    Particle *p = P.p;
    int ep = P.ep;

    float dist,dot;
    vnl_vector_fixed<float, 3> R = p->pos + (p->dir * (ep*m_ParticleLength) );
    m_ParticleGrid->ComputeNeighbors(R);
    m_SimpSamp.clear();

    m_SimpSamp.add(m_StopProb,EndPoint(0,0));

    for (;;)
    {
        Particle *p2 =  m_ParticleGrid->GetNextNeighbor();
        if (p2 == 0) break;
        if (p!=p2 && p2->label == 0)
        {
            if (p2->mID == -1)
            {
                dist = (p2->pos - p2->dir * m_ParticleLength - R).squared_magnitude();
                if (dist < m_DistanceThreshold)
                {
                    dot = dot_product(p2->dir,p->dir) * ep;
                    if (dot > m_CurvatureThreshold)
                    {
                        float en = m_EnergyComputer->ComputeInternalEnergyConnection(p,ep,p2,-1);
                        m_SimpSamp.add(exp(en/m_TractProb),EndPoint(p2,-1));
                    }
                }
            }
            if (p2->pID == -1)
            {
                dist = (p2->pos + p2->dir * m_ParticleLength - R).squared_magnitude();
                if (dist < m_DistanceThreshold)
                {
                    dot = dot_product(p2->dir,p->dir) * (-ep);
                    if (dot > m_CurvatureThreshold)
                    {
                        float en = m_EnergyComputer->ComputeInternalEnergyConnection(p,ep,p2,+1);
                        m_SimpSamp.add(exp(en/m_TractProb),EndPoint(p2,+1));
                    }
                }
            }
        }
    }
}

// return number of accepted proposals
int MetropolisHastingsSampler::GetNumAcceptedProposals()
{
    return m_AcceptedProposals;
}


