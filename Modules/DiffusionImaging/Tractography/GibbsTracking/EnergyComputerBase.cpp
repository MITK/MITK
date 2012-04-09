#ifndef _ENCOMPINTERFACE
#define _ENCOMPINTERFACE


#include "SphereInterpolator.cpp"
#include "ParticleGrid.cpp"
#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_matrix_fixed.h>
#include <mitkLog.h>

inline float myATAN2(float y,float x)
{
  float phi = acos(x);
  //   float phi = ((x>=1.0) ? ((0.0000*x+-0.0000)) : ((x>=1.0) ? ((-10.0167*x+10.0167)) : ((x>=0.9) ? ((-3.1336*x+3.2713)) : ((x>=0.8) ? ((-1.9247*x+2.1833)) : ((x>=0.5) ? ((-1.3457*x+1.7200)) : ((x>=0.0) ? ((-1.0472*x+1.5708)) : ((x>=-0.5) ? ((-1.0472*x+1.5708)) : ((x>=-0.8) ? ((-1.3457*x+1.4216)) : ((x>=-0.9) ? ((-1.9247*x+0.9583)) : ((x>=-1.0) ? ((-3.1336*x+-0.1297)) : ((x>=-1.0) ? ((-10.0167*x+-6.8751)) : 1 )))))))))));
  if (y < 0) phi = 2*PI - phi;
  if (phi<0) phi =  phi + PI;
  return phi;

}


class EnergyComputerBase
{

public:

  float *m_QBallImageData;
  const int *m_QBallImageSize;
  SphereInterpolator *m_SphereInterpolator;
  ParticleGrid<Particle> *m_ParticleGrid;

  int w,h,d;
  float voxsize_w;
  float voxsize_h;
  float voxsize_d;

  int w_sp,h_sp,d_sp;
  float voxsize_sp_w;
  float voxsize_sp_h;
  float voxsize_sp_d;


  int nip; // number of data vertices on sphere


  float *m_MaskImageData;
  float *cumulspatprob;
  int *spatidx;
  int scnt;



  float eigen_energy;
  vnl_matrix_fixed<double, 3, 3> m_RotationMatrix;

  EnergyComputerBase(float *qBallImageData, const int *qBallImageSize, double *voxsize, SphereInterpolator *sp, ParticleGrid<Particle> *pcon, float *maskImageData, int spmult, vnl_matrix_fixed<double, 3, 3> rotMatrix)
  {
    m_RotationMatrix = rotMatrix;
    m_QBallImageData = qBallImageData;
    m_QBallImageSize = qBallImageSize;
    m_SphereInterpolator = sp;

    m_MaskImageData = maskImageData;

    nip = m_QBallImageSize[0];


    w = m_QBallImageSize[1];
    h = m_QBallImageSize[2];
    d = m_QBallImageSize[3];

    voxsize_w = voxsize[0];
    voxsize_h = voxsize[1];
    voxsize_d = voxsize[2];


    w_sp = m_QBallImageSize[1]*spmult;
    h_sp = m_QBallImageSize[2]*spmult;
    d_sp = m_QBallImageSize[3]*spmult;

    voxsize_sp_w = voxsize[0]/spmult;
    voxsize_sp_h = voxsize[1]/spmult;
    voxsize_sp_d = voxsize[2]/spmult;


    fprintf(stderr,"Data size (voxels) : %i x %i x %i\n",w,h,d);
    fprintf(stderr,"voxel size:  %f x %f x %f\n",voxsize_w,voxsize_h,voxsize_d);
    fprintf(stderr,"mask_oversamp_mult: %i\n",spmult);

    if (nip != sp->nverts)
    {
      fprintf(stderr,"EnergyComputer: error during init: data does not match with interpolation scheme\n");
    }

    m_ParticleGrid = pcon;


    int totsz = w_sp*h_sp*d_sp;
    cumulspatprob = (float*) malloc(sizeof(float) * totsz);
    spatidx = (int*) malloc(sizeof(int) * totsz);
    if (cumulspatprob == 0 || spatidx == 0)
    {
      fprintf(stderr,"EnergyCOmputerBase: out of memory!\n");
      return ;
    }


    scnt = 0;
    cumulspatprob[0] = 0;
    for (int x = 1; x < w_sp;x++)
      for (int y = 1; y < h_sp;y++)
        for (int z = 1; z < d_sp;z++)
        {
      int idx = x+(y+z*h_sp)*w_sp;
      if (m_MaskImageData[idx] > 0.5)
      {
        cumulspatprob[scnt+1] = cumulspatprob[scnt] + m_MaskImageData[idx];
        spatidx[scnt] = idx;
        scnt++;
      }
    }

    for (int k = 0; k < scnt; k++)
    {
      cumulspatprob[k] /= cumulspatprob[scnt];
    }

    fprintf(stderr,"#active voxels: %i (in mask units) \n",scnt);



  }

  ~EnergyComputerBase()
  {
    free(cumulspatprob);
    free(spatidx);
  }

  virtual void setParameters()
  {

  }



  void drawSpatPosition(pVector *R)
  {
    float r = mtrand.frand();
    int j;
    int rl = 1;
    int rh = scnt;
    while(rh != rl)
    {
      j = rl + (rh-rl)/2;
      if (r < cumulspatprob[j])
      {
        rh = j;
        continue;
      }
      if (r > cumulspatprob[j])
      {
        rl = j+1;
        continue;
      }
      break;
    }
    R->SetXYZ(voxsize_sp_w*((float)(spatidx[rh-1] % w_sp)  + mtrand.frand()),
              voxsize_sp_h*((float)((spatidx[rh-1]/w_sp) % h_sp)  + mtrand.frand()),
              voxsize_sp_d*((float)(spatidx[rh-1]/(w_sp*h_sp))    + mtrand.frand()));
  }

  float SpatProb(pVector R)
  {
    int rx = int(R.GetX()/voxsize_sp_w);
    int ry = int(R.GetY()/voxsize_sp_h);
    int rz = int(R.GetZ()/voxsize_sp_d);
    if (rx >= 0 && rx < w_sp && ry >= 0 && ry < h_sp && rz >= 0 && rz < d_sp){
      return m_MaskImageData[rx + w_sp* (ry + h_sp*rz)];
    }
    else
      return 0;
  }

  /*
  inline float evaluateODF(pVector &R, pVector &N, float &len)
  {
    const int CU = 10;
    pVector Rs;
    float Dn = 0;
    int xint,yint,zint,spatindex;

    sinterp->getInterpolation(N);
    for (int i=-CU; i < CU;i++)
    {
      Rs = R + (N * len) * ((float)i/CU);
      xint = int(Rs.x);
      yint = int(Rs.y);
      zint = int(Rs.z);
      if (xint > 0 && xint < w-1 && yint > 0 && yint < h-1 && zint > 0 && zint < d-1)
      {
        spatindex = (xint + w*(yint+h*zint)) *nip;
        Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2]);
      }
    }

    Dn /= (float)(2*CU+1);
    return Dn;
  }
*/


  inline float evaluateODF(pVector &R, pVector &N, float &len)
  {
    const int CU = 10;
    pVector Rs;
    float Dn = 0;
    int xint,yint,zint,spatindex;

    vnl_vector_fixed<double, 3> n;
    n[0] = N[0];
    n[1] = N[1];
    n[2] = N[2];
    n = m_RotationMatrix*n;
    m_SphereInterpolator->getInterpolation(n);

    for (int i=-CU; i <= CU;i++)
    {
      Rs = R + (N * len) * ((float)i/CU);

      float Rx = Rs[0]/voxsize_w-0.5;
      float Ry = Rs[1]/voxsize_h-0.5;
      float Rz = Rs[2]/voxsize_d-0.5;


      xint = int(floor(Rx));
      yint = int(floor(Ry));
      zint = int(floor(Rz));


      if (xint >= 0 && xint < w-1 && yint >= 0 && yint < h-1 && zint >= 0 && zint < d-1)
      {
        float xfrac = Rx-xint;
        float yfrac = Ry-yint;
        float zfrac = Rz-zint;

        float weight;

        weight = (1-xfrac)*(1-yfrac)*(1-zfrac);
        spatindex = (xint + w*(yint+h*zint)) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (xfrac)*(1-yfrac)*(1-zfrac);
        spatindex = (xint+1 + w*(yint+h*zint)) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (1-xfrac)*(yfrac)*(1-zfrac);
        spatindex = (xint + w*(yint+1+h*zint)) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (1-xfrac)*(1-yfrac)*(zfrac);
        spatindex = (xint + w*(yint+h*(zint+1))) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (xfrac)*(yfrac)*(1-zfrac);
        spatindex = (xint+1 + w*(yint+1+h*zint)) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (1-xfrac)*(yfrac)*(zfrac);
        spatindex = (xint + w*(yint+1+h*(zint+1))) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (xfrac)*(1-yfrac)*(zfrac);
        spatindex = (xint+1 + w*(yint+h*(zint+1))) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

        weight = (xfrac)*(yfrac)*(zfrac);
        spatindex = (xint+1 + w*(yint+1+h*(zint+1))) *nip;
        Dn += (m_QBallImageData[spatindex + m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] + m_QBallImageData[spatindex + m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;


      }


    }

    Dn *= 1.0/(2*CU+1);
    return Dn;
  }


  /*
  inline float evaluateODF(pVector &R, pVector &N, float &len)
  {

    R.storeXYZ();

    float Rx = pVector::store[0]/voxsize_w;
    float Ry = pVector::store[1]/voxsize_h;
    float Rz = pVector::store[2]/voxsize_d;


    int xint = int(Rx);
    int yint = int(Ry);
    int zint = int(Rz);

    if (xint >= 0 && xint < w-1 && yint >= 0 && yint < h-1 && zint >= 0 && zint < d-1)
    {
      float xfrac = Rx-xint;
      float yfrac = Ry-yint;
      float zfrac = Rz-zint;
      sinterp->getInterpolation(N);

      float weight;
      int spatindex;
      float Dn = 0;

      weight = (1-xfrac)*(1-yfrac)*(1-zfrac);
      spatindex = (xint + w*(yint+h*zint)) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (xfrac)*(1-yfrac)*(1-zfrac);
      spatindex = (xint+1 + w*(yint+h*zint)) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (1-xfrac)*(yfrac)*(1-zfrac);
      spatindex = (xint + w*(yint+1+h*zint)) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (1-xfrac)*(1-yfrac)*(zfrac);
      spatindex = (xint + w*(yint+h*(zint+1))) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (xfrac)*(yfrac)*(1-zfrac);
      spatindex = (xint+1 + w*(yint+1+h*zint)) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (1-xfrac)*(yfrac)*(zfrac);
      spatindex = (xint + w*(yint+1+h*(zint+1))) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (xfrac)*(1-yfrac)*(zfrac);
      spatindex = (xint+1 + w*(yint+h*(zint+1))) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      weight = (xfrac)*(yfrac)*(zfrac);
      spatindex = (xint+1 + w*(yint+1+h*(zint+1))) *nip;
      Dn += (dataimg[spatindex + sinterp->idx[0]]*sinterp->interpw[0] + dataimg[spatindex + sinterp->idx[1]]*sinterp->interpw[1] + dataimg[spatindex + sinterp->idx[2]]* sinterp->interpw[2])*weight;

      return Dn;

    }
    return 0;
  }

*/

  virtual inline float computeExternalEnergy(pVector &R, pVector &N, float &cap, float &len, Particle *dp) { return 0;}
  virtual inline float computeInternalEnergy(Particle *p1) {return 0;}
  virtual inline float computeInternalEnergyConnection(Particle *p1,int ep1) {return 0;}
  virtual inline float computeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2) {return 0;}



};

#endif


