


/// bessel for wid = 1

//#define mbesseli0(x) ((x>=1.0) ? ((-0.2578*x+0.7236)*exp(x)) : ((x>=0.9) ? ((-0.2740*x+0.7398)*exp(x)) : ((x>=0.8) ? ((-0.3099*x+0.7720)*exp(x)) : ((x>=0.7) ? ((-0.3634*x+0.8149)*exp(x)) : ((x>=0.5) ? ((-0.4425*x+0.8663)*exp(x)) : ((x>=0.3) ? ((-0.5627*x+0.9264)*exp(x)) : ((x>=0.2) ? ((-0.6936*x+0.9657)*exp(x)) : ((x>=0.1) ? ((-0.8016*x+0.9873)*exp(x)) : ((x>=0.0) ? ((-0.9290*x+1.0000)*exp(x)) : 1 )))))))))
//#define mbesseli0(x) ((x>=1.0) ? ((0.5652*x+0.7009)) : ((x>=0.8) ? ((0.4978*x+0.7683)) : ((x>=0.6) ? ((0.3723*x+0.8686)) : ((x>=0.4) ? ((0.2582*x+0.9371)) : ((x>=0.2) ? ((0.1519*x+0.9796)) : ((x>=0.0) ? ((0.0501*x+1.0000)) : 1 ))))))


inline float mbesseli0(float x)
{
    float y = x*x;
    float erg = BESSEL_APPROXCOEFF[0];
    erg += y*BESSEL_APPROXCOEFF[1];
    erg += y*y*BESSEL_APPROXCOEFF[2];
    erg += y*y*y*BESSEL_APPROXCOEFF[3];
    return erg;
}

//
//
// inline REAL mbesseli0(REAL x)
// {
//     REAL y = x*x;
//     REAL erg = BESSEL_APPROXCOEFF[0];
//     erg += y*BESSEL_APPROXCOEFF[1];
//     erg += y*x*BESSEL_APPROXCOEFF[2];
//     erg += x*x*BESSEL_APPROXCOEFF[3];
//     return erg;
// }

inline float mexp(float x)
{

  return((x>=7.0) ? 0 : ((x>=5.0) ? (-0.0029*x+0.0213) : ((x>=3.0) ? (-0.0215*x+0.1144) : ((x>=2.0) ? (-0.0855*x+0.3064) : ((x>=1.0) ? (-0.2325*x+0.6004) : ((x>=0.5) ? (-0.4773*x+0.8452) : ((x>=0.0) ? (-0.7869*x+1.0000) : 1 )))))));
  //  return exp(-x);

}


#include "ParticleGrid.cpp"

#include "EnergyComputerBase.cpp"
#include <fstream>


class EnergyComputer : public EnergyComputerBase
{

public:


  float eigencon_energy;

  float chempot2;
  float meanval_sq;

  float gamma_s;
  float gamma_reg_s;

  float particle_weight;
  float ex_strength;
  float in_strength;

  float particle_length_sq;
  float curv_hard;


  EnergyComputer(float *data, const int *dsz,  double *cellsize, SphereInterpolator *sp, ParticleGrid<Particle> *pcon, float *spimg, int spmult, vnl_matrix_fixed<double, 3, 3> rotMatrix) : EnergyComputerBase(data,dsz,cellsize,sp,pcon,spimg,spmult,rotMatrix)
  {

  }

  void setParameters(float pwei,float pwid,float chempot_connection, float length,float curv_hardthres, float inex_balance, float chempot2, float meanv)
  {
    this->chempot2 = chempot2;
    meanval_sq = meanv;

    eigencon_energy = chempot_connection;
    eigen_energy = 0;
    particle_weight = pwei;

    float bal = 1/(1+exp(-inex_balance));
    ex_strength = 2*bal;                         // cleanup (todo)
    in_strength = 2*(1-bal)/length/length;         // cleanup (todo)
    //    in_strength = 0.64/length/length;         // cleanup (todo)

    particle_length_sq = length*length;
    curv_hard = curv_hardthres;

    float sigma_s = pwid;
    gamma_s = 1/(sigma_s*sigma_s);
    gamma_reg_s =1/(length*length/4);
  }



  ////////////////////////////////////////////////////////////////////////////
  ////// External Energy
  ////////////////////////////////////////////////////////////////////////////
  inline float computeExternalEnergy(pVector &R, pVector &N, float &cap, float &len, Particle *dp)
  {
    float m = SpatProb(R);
    if (m == 0)
    {
      return -INFINITY;
    }

    float Dn = evaluateODF(R,N,len);

    float Sn = 0;
    float Pn = 0;

    m_ParticleGrid->computeNeighbors(R);
    for (;;)
    {
      Particle *p =  m_ParticleGrid->getNextNeighbor();
      if (p == 0) break;
      if (dp != p)
      {
        float dot = fabs(N*p->N);
        float bw = mbesseli0(dot);
        float dpos = (p->R-R).norm_square();
        float w = mexp(dpos*gamma_s);
        Sn += w*(bw+chempot2)*p->cap ;
        //Sn += w*(bw-meanval_sq)*p->cap ;
        w = mexp(dpos*gamma_reg_s);
        Pn += w*bw;
      }
    }

    float energy = 0;
    energy += (2*(Dn/particle_weight-Sn) - (mbesseli0(1.0)+chempot2)*cap)*cap;
    //energy += (2*(Dn/particle_weight-Sn) - (mbesseli0(1.0)-meanval_sq)*cap)*cap;

    return energy*ex_strength;
  }


  ////////////////////////////////////////////////////////////////////////////
  ////// Internal Energy
  ////////////////////////////////////////////////////////////////////////////

  inline float computeInternalEnergy(Particle *dp)
  {
    float energy = eigen_energy;

    if (dp->pID != -1)
      energy += computeInternalEnergyConnection(dp,+1);

    if (dp->mID != -1)
      energy += computeInternalEnergyConnection(dp,-1);

    //ie_file << energy << "\n";

    return energy;
  }

  inline float computeInternalEnergyConnection(Particle *p1,int ep1)
  {
    Particle *p2 = 0;
    int ep2;
    if (ep1 == 1)
      p2 = m_ParticleGrid->ID_2_address[p1->pID];
    else
      p2 = m_ParticleGrid->ID_2_address[p1->mID];
    if (p2->mID == p1->ID)
      ep2 = -1;
    else if (p2->pID == p1->ID)
      ep2 = 1;
    else
      fprintf(stderr,"EnergyComputer_connec: Connections are inconsistent!\n");

    if (p2 == 0)
      fprintf(stderr,"bug2");

    return computeInternalEnergyConnection(p1,ep1,p2,ep2);
  }

  inline float computeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2)
  {
#ifdef TIMING
    tic(&internalenergy_time);
#endif

    if ((p1->N*p2->N)*ep1*ep2 > -curv_hard)
      return -INFINITY;

    pVector R1 = p1->R + (p1->N * (p1->len * ep1));
    pVector R2 = p2->R + (p2->N * (p2->len * ep2));

    if ((R1-R2).norm_square() > particle_length_sq)
      return -INFINITY;

    pVector R = (p2->R + p1->R)*0.5;

    if (SpatProb(R) == 0)
      return -INFINITY;

    float norm1 = (R1-R).norm_square();
    float norm2 = (R2-R).norm_square();


    float energy = (eigencon_energy-norm1-norm2)*in_strength;

#ifdef TIMING
    toc(&internalenergy_time);
#endif

    return energy;
  }









};

