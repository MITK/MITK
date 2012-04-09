
#include "ParticleGrid.cpp"
#include <vector>
#include "MersenneTwister.h"

class RJMCMCBase
{
public:

  ParticleGrid<Particle> m_ParticleGrid;
  float *m_QBallImageData;
  const int *datasz;
  EnergyComputerBase *enc;
  int m_Iterations;
  float width;
  float height;
  float depth;
  double *voxsize;
  int m_NumAttributes;
  int m_AcceptedProposals;

  RJMCMCBase(float *points,int numPoints, float *dimg, const int *dsz, double *voxsize, double cellsize)
    : m_QBallImageData(dimg)
    , datasz(dsz)
    , enc(0)
    , width(dsz[1]*voxsize[0])
    , height(dsz[2]*voxsize[1])
    , depth(dsz[3]*voxsize[2])
    , voxsize(voxsize)
    , m_NumAttributes(0)
    , m_AcceptedProposals(0)
  {
    fprintf(stderr,"Data dimensions (mm) :  %f x %f x %f\n",width,height,depth);
    fprintf(stderr,"Data dimensions (voxel) :  %i x %i x %i\n",datasz[1],datasz[2],datasz[3]);
    fprintf(stderr,"voxel size (mm) :  %lf x %lf x %lf\n",voxsize[0],voxsize[1],voxsize[2]);

    float cellcnt_x = (int)((float)width/cellsize) +1;
    float cellcnt_y = (int)((float)height/cellsize) +1;
    float cellcnt_z = (int)((float)depth/cellsize) +1;
    //int cell_capacity = 2048;
    //int cell_capacity = 64;
    int cell_capacity = 512;

    fprintf(stderr,"grid dimensions :  %f x %f x %f\n",cellcnt_x,cellcnt_y,cellcnt_z);
    fprintf(stderr,"grid cell size (mm) :  %f^3\n",cellsize);
    fprintf(stderr,"cell capacity :  %i\n",cell_capacity);
    fprintf(stderr,"#cells*cellcap :  %.1f K\n",cell_capacity*cellcnt_x*cellcnt_y*cellcnt_z/1000);

    int minsize = 1000000;
    int err = m_ParticleGrid.allocate(((numPoints>minsize)? (numPoints+100000) : minsize), cellcnt_x, cellcnt_y, cellcnt_z, cellsize, cell_capacity);

    if (err == -1)
    {
      fprintf(stderr,"RJMCMCBase: out of Memory!\n");
      return;
    }

    m_NumAttributes = 10;
    for (int k = 0; k < numPoints; k++)
    {
      Particle *p = m_ParticleGrid.newParticle(pVector(points[m_NumAttributes*k], points[m_NumAttributes*k+1],points[m_NumAttributes*k+2]));
      if (p!=0)
      {
        p->N = pVector(points[m_NumAttributes*k+3],points[m_NumAttributes*k+4],points[m_NumAttributes*k+5]);
        p->cap =  points[m_NumAttributes*k+6];
        p->len =  points[m_NumAttributes*k+7];
        p->mID = (int) points[m_NumAttributes*k+8];
        p->pID = (int) points[m_NumAttributes*k+9];
        if (p->mID != -1)
          m_ParticleGrid.concnt++;
        if (p->pID != -1)
          m_ParticleGrid.concnt++;
        p->label = 0;
      }
      else
      {
        fprintf(stderr,"error: cannot allocate particle,  con. indices will be wrong! \n");
      }
    }
    m_ParticleGrid.concnt /= 2;

    m_Iterations = 0;
    m_AcceptedProposals = 0;
  }

  ~RJMCMCBase()
  {

  }

  void WriteOutParticles(float *npoints)
  {
    for (int k = 0; k < m_ParticleGrid.pcnt; k++)
    {
      Particle *p = &(m_ParticleGrid.particles[k]);
      npoints[m_NumAttributes*k] = p->R.GetX();
      npoints[m_NumAttributes*k+1] = p->R.GetY();
      npoints[m_NumAttributes*k+2] = p->R.GetZ();
      npoints[m_NumAttributes*k+3] = p->N.GetX();
      npoints[m_NumAttributes*k+4] = p->N.GetY();
      npoints[m_NumAttributes*k+5] = p->N.GetZ();
      npoints[m_NumAttributes*k+6] = p->cap;
      npoints[m_NumAttributes*k+7] = p->len;
      npoints[m_NumAttributes*k+8] = m_ParticleGrid.ID_2_index(p->mID);
      npoints[m_NumAttributes*k+9] = m_ParticleGrid.ID_2_index(p->pID);
    }
  }

  void SetEnergyComputer(EnergyComputerBase *e)
  {
    enc = e;
  }

  void Iterate(float* acceptance, unsigned long* numCon, unsigned long* numPart, bool *abort)
  {
    m_AcceptedProposals = 0;
    for (int it = 0; it < m_Iterations;it++)
    {
      if (*abort)
        break;

      IterateOneStep();

      *numCon = m_ParticleGrid.concnt;
      *numPart = m_ParticleGrid.pcnt;
    }
    *acceptance = (float)m_AcceptedProposals/m_Iterations;
  }

  virtual void IterateOneStep()
  {

  }
};



