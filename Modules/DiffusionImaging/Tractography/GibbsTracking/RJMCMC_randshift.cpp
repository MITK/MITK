
#include "ParticleGrid.cpp"
#include "RJMCMCBase.cpp"
#include <fstream>

class RJMCMC : public RJMCMCBase
{
public:

  float T_in ;
  float T_ex ;
  float dens;


  float p_birth;
  float p_death;
  float p_shift;
  float p_shiftopt;
  float p_cap;
  float p_con;



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


  Track TrackProposal, TrackBackup;


  SimpSamp<EndPoint> simpsamp;


  RJMCMC(float *points,int numPoints,  float *dimg, const int *dsz, double *voxsz, double cellsz) : RJMCMCBase(points,numPoints,dimg,dsz,voxsz,cellsz)
  {
    externalEnergy = 0;
    internalEnergy = 0;
  }

  void SetParameters(float Temp, int numit, float plen, float curv_hardthres, float chempot_particle)
  {
    m_Iterations = numit;

    p_birth = 0.25;
    p_death = 0.05;
    p_shift = 0.15;
    p_shiftopt = 0.1;
    p_con = 0.45;
    p_cap = 0.0;

    m_ChempotParticle = chempot_particle;

    float sum = p_birth+p_death+p_shift+p_shiftopt+p_con;
    p_birth /= sum; p_death /= sum; p_shift /= sum; p_shiftopt /= sum;

    T_in = Temp;
    T_ex = 0.01;
    dens = exp(-chempot_particle/T_in);

    len_def = plen;
    len_sig = 0.0;
    cap_def = 1.0;
    cap_sig = 0.0;

    // shift proposal
    sigma_g = len_def/8.0;
    gamma_g = 1/(sigma_g*sigma_g*2);
    Z_g = pow(2*PI*sigma_g,3.0/2.0)*(PI*sigma_g/len_def);

    // conn proposal
    dthres = len_def;
    nthres = curv_hardthres;
    T_prop = 0.5;
    dthres *= dthres;
    stopprobability = exp(-1/T_prop);
    del_prob = 0.1;
  }

  void SetTemperature(float temp)
  {
    T_in = temp;
    dens = exp(-m_ChempotParticle/T_in);
  }

  void IterateOneStep()
  {
    float randnum = mtrand.frand();
    //randnum = 0;

    ///////////////////////////////////////////////////////////////
    //////// Birth Proposal
    ///////////////////////////////////////////////////////////////
    if (randnum < p_birth)
    {

#ifdef TIMING
      tic(&birthproposal_time);
      birthstats.propose();
#endif

      pVector R;
      enc->drawSpatPosition(&R);

      //fprintf(stderr,"drawn: %f, %f, %f\n",R[0],R[1],R[2]);
      //R.setXYZ(20.5*3, 35.5*3, 1.5*3);

      pVector N; N.rand_sphere();
      //N.setXYZ(1,0,0);
      float cap =  cap_def - cap_sig*mtrand.frand();
      float len =  len_def;// + len_sig*(mtrand.frand()-0.5);
      Particle prop;
      prop.R = R;
      prop.N = N;
      prop.cap = cap;
      prop.len = len;


      float prob =  dens * p_death /((p_birth)*(m_ParticleGrid.pcnt+1));

      float ex_energy = enc->computeExternalEnergy(R,N,cap,len,0);
      float in_energy = enc->computeInternalEnergy(&prop);

      prob *= exp((in_energy/T_in+ex_energy/T_ex)) ;

      if (prob > 1 || mtrand.frand() < prob)
      {
        Particle *p = m_ParticleGrid.newParticle(R);
        if (p!=0)
        {
          p->R = R;
          p->N = N;
          p->cap = cap;
          p->len = len;
#ifdef TIMING
          birthstats.accepted();
#endif
          m_AcceptedProposals++;
        }
      }

#ifdef TIMING
      toc(&birthproposal_time);
#endif
    }
    ///////////////////////////////////////////////////////////////
    //////// Death Proposal
    ///////////////////////////////////////////////////////////////
    else if (randnum < p_birth+p_death)
    {
      if (m_ParticleGrid.pcnt > 0)
      {
#ifdef TIMING
        tic(&deathproposal_time);
        deathstats.propose();
#endif

        int pnum = rand()%m_ParticleGrid.pcnt;
        Particle *dp = &(m_ParticleGrid.particles[pnum]);
        if (dp->pID == -1 && dp->mID == -1)
        {

          float ex_energy = enc->computeExternalEnergy(dp->R,dp->N,dp->cap,dp->len,dp);
          float in_energy = enc->computeInternalEnergy(dp);

          float prob = m_ParticleGrid.pcnt * (p_birth) /(dens*p_death); //*SpatProb(dp->R);
          prob *= exp(-(in_energy/T_in+ex_energy/T_ex)) ;
          if (prob > 1 || mtrand.frand() < prob)
          {
            m_ParticleGrid.remove(pnum);
#ifdef TIMING
            deathstats.accepted();
#endif
            m_AcceptedProposals++;
          }
        }
#ifdef TIMING
        toc(&deathproposal_time);
#endif
      }

    }
    ///////////////////////////////////////////////////////////////
    //////// Cap change Proposal
    ///////////////////////////////////////////////////////////////
    else  if (randnum < p_birth+p_death+p_cap)
    {
      if (m_ParticleGrid.pcnt > 0)
      {

        int pnum = rand()%m_ParticleGrid.pcnt;
        Particle *p =  &(m_ParticleGrid.particles[pnum]);
        Particle prop_p = *p;

        prop_p.cap = cap_def - cap_sig*mtrand.frand();

        float ex_energy = enc->computeExternalEnergy(prop_p.R,prop_p.N,prop_p.cap,p->len,p)
                          - enc->computeExternalEnergy(p->R,p->N,p->cap,p->len,p);
        //float in_energy = enc->computeExternalEnergy(prop_p.R,prop_p.N,p->cap,p->len,p)
        //    - enc->computeExternalEnergy(p->R,p->N,p->cap,p->len,p);
        float prob = exp(ex_energy/T_ex);
        // * SpatProb(p->R) / SpatProb(prop_p.R);
        if (mtrand.frand() < prob)
        {
          p->cap = prop_p.cap;
          m_AcceptedProposals++;
        }

      }

    }

    ///////////////////////////////////////////////////////////////
    //////// Shift Proposal
    ///////////////////////////////////////////////////////////////
    else  if (randnum < p_birth+p_death+p_shift+p_cap)
    {
      float energy = 0;
      if (m_ParticleGrid.pcnt > 0)
      {
#ifdef TIMING
        tic(&shiftproposal_time);
        shiftstats.propose();
#endif

        int pnum = rand()%m_ParticleGrid.pcnt;
        Particle *p =  &(m_ParticleGrid.particles[pnum]);
        Particle prop_p = *p;

        prop_p.R.distortn(sigma_g);
        prop_p.N.distortn(sigma_g/(2*p->len));
        prop_p.N.normalize();


        float ex_energy = enc->computeExternalEnergy(prop_p.R,prop_p.N,p->cap,p->len,p)
                          - enc->computeExternalEnergy(p->R,p->N,p->cap,p->len,p);
        float in_energy = enc->computeInternalEnergy(&prop_p) - enc->computeInternalEnergy(p);

        float prob = exp(ex_energy/T_ex+in_energy/T_in);
        // * SpatProb(p->R) / SpatProb(prop_p.R);
        if (mtrand.frand() < prob)
        {
          pVector Rtmp = p->R;
          pVector Ntmp = p->N;
          p->R = prop_p.R;
          p->N = prop_p.N;
          if (!m_ParticleGrid.tryUpdateGrid(pnum))
          {
            p->R = Rtmp;
            p->N = Ntmp;
          }
#ifdef TIMING
          shiftstats.accepted();
#endif
          m_AcceptedProposals++;
        }

#ifdef TIMING
        toc(&shiftproposal_time);
#endif

      }

    }
    else  if (randnum < p_birth+p_death+p_shift+p_shiftopt+p_cap)
    {
      float energy = 0;
      if (m_ParticleGrid.pcnt > 0)
      {

        int pnum = rand()%m_ParticleGrid.pcnt;
        Particle *p =  &(m_ParticleGrid.particles[pnum]);

        bool no_proposal = false;
        Particle prop_p = *p;
        if (p->pID != -1 && p->mID != -1)
        {
          Particle *plus = m_ParticleGrid.ID_2_address[p->pID];
          int ep_plus = (plus->pID == p->ID)? 1 : -1;
          Particle *minus = m_ParticleGrid.ID_2_address[p->mID];
          int ep_minus = (minus->pID == p->ID)? 1 : -1;
          prop_p.R = (plus->R + plus->N * (plus->len * ep_plus)  + minus->R + minus->N * (minus->len * ep_minus))*0.5;
          prop_p.N = plus->R - minus->R;
          prop_p.N.normalize();
        }
        else if (p->pID != -1)
        {
          Particle *plus = m_ParticleGrid.ID_2_address[p->pID];
          int ep_plus = (plus->pID == p->ID)? 1 : -1;
          prop_p.R = plus->R + plus->N * (plus->len * ep_plus * 2);
          prop_p.N = plus->N;
        }
        else if (p->mID != -1)
        {
          Particle *minus = m_ParticleGrid.ID_2_address[p->mID];
          int ep_minus = (minus->pID == p->ID)? 1 : -1;
          prop_p.R = minus->R + minus->N * (minus->len * ep_minus * 2);
          prop_p.N = minus->N;
        }
        else
          no_proposal = true;

        if (!no_proposal)
        {
          float cos = prop_p.N*p->N;
          float p_rev = exp(-((prop_p.R-p->R).norm_square() + (1-cos*cos))*gamma_g)/Z_g;

          float ex_energy = enc->computeExternalEnergy(prop_p.R,prop_p.N,p->cap,p->len,p)
                            - enc->computeExternalEnergy(p->R,p->N,p->cap,p->len,p);
          float in_energy = enc->computeInternalEnergy(&prop_p) - enc->computeInternalEnergy(p);

          float prob = exp(ex_energy/T_ex+in_energy/T_in)*p_shift*p_rev/(p_shiftopt+p_shift*p_rev);
          //* SpatProb(p->R) / SpatProb(prop_p.R);

          if (mtrand.frand() < prob)
          {
            pVector Rtmp = p->R;
            pVector Ntmp = p->N;
            p->R = prop_p.R;
            p->N = prop_p.N;
            if (!m_ParticleGrid.tryUpdateGrid(pnum))
            {
              p->R = Rtmp;
              p->N = Ntmp;
            }
            m_AcceptedProposals++;
          }
        }
      }

    }
    else
    {


      if (m_ParticleGrid.pcnt > 0)
      {

#ifdef TIMING
        tic(&connproposal_time);
        connstats.propose();
#endif

        int pnum = rand()%m_ParticleGrid.pcnt;
        Particle *p = &(m_ParticleGrid.particles[pnum]);

        EndPoint P;
        P.p = p;
        P.ep = (mtrand.frand() > 0.5)? 1 : -1;

        RemoveAndSaveTrack(P);
        if (TrackBackup.proposal_probability != 0)
        {
          MakeTrackProposal(P);

          float prob = (TrackProposal.energy-TrackBackup.energy)/T_in ;

          //            prob = exp(prob)*(TrackBackup.proposal_probability)
          //                                        /(TrackProposal.proposal_probability);
          prob = exp(prob)*(TrackBackup.proposal_probability * pow(del_prob,TrackProposal.length))
                 /(TrackProposal.proposal_probability * pow(del_prob,TrackBackup.length));
          if (mtrand.frand() < prob)
          {
            ImplementTrack(TrackProposal);
#ifdef TIMING
            connstats.accepted();
#endif
            m_AcceptedProposals++;
          }
          else
          {
            ImplementTrack(TrackBackup);
          }
        }
        else
          ImplementTrack(TrackBackup);

#ifdef TIMING
        toc(&connproposal_time);
#endif
      }
    }
  }


  void ImplementTrack(Track &T)
  {
    for (int k = 1; k < T.length;k++)
    {
      m_ParticleGrid.createConnection(T.track[k-1].p,T.track[k-1].ep,T.track[k].p,-T.track[k].ep);
    }
  }



  void RemoveAndSaveTrack(EndPoint P)
  {
    EndPoint Current = P;

    int cnt = 0;
    float energy = 0;
    float AccumProb = 1.0;
    TrackBackup.track[cnt] = Current;

    EndPoint Next;



    for (;;)
    {
      Next.p = 0;
      if (Current.ep == 1)
      {
        if (Current.p->pID != -1)
        {
          Next.p = m_ParticleGrid.ID_2_address[Current.p->pID];
          Current.p->pID = -1;
          m_ParticleGrid.concnt--;
        }
      }
      else if (Current.ep == -1)
      {
        if (Current.p->mID != -1)
        {
          Next.p = m_ParticleGrid.ID_2_address[Current.p->mID];
          Current.p->mID = -1;
          m_ParticleGrid.concnt--;
        }
      }
      else
      { fprintf(stderr,"RJMCMC_randshift: Connection inconsistent 3\n"); break; }

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
        { fprintf(stderr,"RJMCMC_randshift: Connection inconsistent 4\n"); break; }
      }


      ComputeEndPointProposalDistribution(Current);

      AccumProb *= (simpsamp.probFor(Next));

      if (Next.p == 0) // no successor -> break
        break;

      energy += enc->computeInternalEnergyConnection(Current.p,Current.ep,Next.p,Next.ep);

      Current = Next;
      Current.ep *= -1;
      cnt++;
      TrackBackup.track[cnt] = Current;


      if (mtrand.rand() > del_prob)
      {
        break;
      }

    }
    TrackBackup.energy = energy;
    TrackBackup.proposal_probability = AccumProb;
    TrackBackup.length = cnt+1;

  }



  void MakeTrackProposal(EndPoint P)
  {
    EndPoint Current = P;
    int cnt = 0;
    float energy = 0;
    float AccumProb = 1.0;
    TrackProposal.track[cnt++] = Current;
    Current.p->label = 1;

    for (;;)
    {

      // next candidate is already connected
      if ((Current.ep == 1 && Current.p->pID != -1) || (Current.ep == -1 && Current.p->mID != -1))
        break;

      // track too long
      if (cnt > 250)
        break;

      ComputeEndPointProposalDistribution(Current);

      //       // no candidates anymore
      //       if (simpsamp.isempty())
      //         break;

      int k = simpsamp.draw();

      // stop tracking proposed
      if (k==0)
        break;

      EndPoint Next = simpsamp.objs[k];
      float probability = simpsamp.probFor(k);

      // accumulate energy and proposal distribution
      energy += enc->computeInternalEnergyConnection(Current.p,Current.ep,Next.p,Next.ep);
      AccumProb *= probability;

      // track to next endpoint
      Current = Next;
      Current.ep *= -1;

      Current.p->label = 1;  // put label to avoid loops
      TrackProposal.track[cnt++] = Current;



    }

    TrackProposal.energy = energy;
    TrackProposal.proposal_probability = AccumProb;
    TrackProposal.length = cnt;

    // clear labels
    for (int j = 0; j < TrackProposal.length;j++)
    {
      TrackProposal.track[j].p->label = 0;
    }

  }




  void ComputeEndPointProposalDistribution(EndPoint P)
  {
    Particle *p = P.p;
    int ep = P.ep;

    float dist,dot;
    pVector R = p->R + (p->N * ep*p->len);
    m_ParticleGrid.computeNeighbors(R);
    simpsamp.clear();

    simpsamp.add(stopprobability,EndPoint(0,0));

    for (;;)
    {
      Particle *p2 =  m_ParticleGrid.getNextNeighbor();
      if (p2 == 0) break;
      if (p!=p2 && p2->label == 0)
      {
        if (p2->mID == -1)
        {
          dist = (p2->R - p2->N * p2->len - R).norm_square();
          if (dist < dthres)
          {
            dot = p2->N*p->N * ep;
            if (dot > nthres)
            {
              float en = enc->computeInternalEnergyConnection(p,ep,p2,-1);
              simpsamp.add(exp(en/T_prop),EndPoint(p2,-1));
            }
          }
        }
        if (p2->pID == -1)
        {
          dist = (p2->R + p2->N * p2->len - R).norm_square();
          if (dist < dthres)
          {
            dot = p2->N*p->N * (-ep);
            if (dot > nthres)
            {
              float en = enc->computeInternalEnergyConnection(p,ep,p2,+1);
              simpsamp.add(exp(en/T_prop),EndPoint(p2,+1));
            }
          }
        }
      }
    }
  }


};



