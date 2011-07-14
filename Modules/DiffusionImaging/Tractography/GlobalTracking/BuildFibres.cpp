#ifndef _BUILDFIBRES
#define _BUILDFIBRES

//#include "matrix.h"
#include <math.h>
#include <map>
#include <vector>
#include <string.h>

using namespace std;

#define PI 3.1415926536



#ifdef TIMING

static struct timeval timeS;

class PropStats
{
  int N;
  int accept;
public:
  void clear() { N = 0; accept = 0;}
  void propose() {N++;}
  void accepted() {accept++;}

  void report(const char *s)
  {
    mexPrintf("%s  #proposals: %8.2fk  acceptratio: %.2f \% \n",s,1.0*N/1000.0,100.0*accept/N);
  }
};


class Timing
{
public:
  Timing() { time = 0; ncalls = 0;}
  void clear() {time = 0; ncalls=0;}


  long time;
  int ncalls;

  void report(const char *s)
  {
    mexPrintf("%s total: %10.2fms calls: %10.1fk t/call: %10.3fms \n",s,time/1000.0,1.0*ncalls/1000.0,1.0*time/ncalls);
  }

  void report_time(const char *s)
  {
    mexPrintf("%s: %.2fms \n",s,time/1000.0);
  }

};

inline void tic(Timing *t)
{
  gettimeofday( &timeS, NULL);
  t->time -= (timeS.tv_sec*1000000 + timeS.tv_usec);
  t->ncalls++;
}
inline void toc(Timing *t)
{
  gettimeofday( &timeS, NULL);
  t->time += (timeS.tv_sec*1000000 + timeS.tv_usec);
}


#endif

#include "ParticleGrid.cpp"

class CCAnalysis
{
public:
  Particle *particles;
  int pcnt;
  int attrcnt;
  typedef vector< Particle* > ParticleContainerType;
  typedef vector< ParticleContainerType* > FiberContainerType;

  FiberContainerType* m_FiberContainer;

  CCAnalysis(float *points, int numPoints, double spacing[])
  {
    m_FiberContainer = new FiberContainerType();
    particles = (Particle*) malloc(sizeof(Particle)*numPoints);
    pcnt = numPoints;
    attrcnt = 10;
    for (int k = 0; k < numPoints; k++)
    {
      Particle *p = &(particles[k]);
      p->R = pVector(points[attrcnt*k]/spacing[0], points[attrcnt*k+1]/spacing[1],points[attrcnt*k+2]/spacing[2]);
      p->N = pVector(points[attrcnt*k+3],points[attrcnt*k+4],points[attrcnt*k+5]);
      p->cap =  points[attrcnt*k+6];
      p->len =  points[attrcnt*k+7];
      p->mID = (int) points[attrcnt*k+8];
      p->pID = (int) points[attrcnt*k+9];
      p->ID = k;
      p->label = 0;
    }
  }

  ~CCAnalysis()
  {
    for (int i=0; i<m_FiberContainer->size(); i++)
      delete(m_FiberContainer->at(i));
    delete(m_FiberContainer);
    free(particles);
  }

  int iterate(int minSize)
  {
    int cur_label = 1;
    int numFibers = 0;
    for (int k = 0; k < pcnt;k++)
    {
      Particle *dp =  &(particles[k]);
      if (dp->label == 0)
      {
        ParticleContainerType* container = new ParticleContainerType();
        dp->label = cur_label;
        dp->numerator = 0;
        labelPredecessors(dp, container);
        labelSuccessors(dp, container);
        //labelrecursivly(dp, 0);
        cur_label++;
        if(container->size()>minSize){
          m_FiberContainer->push_back(container);
          numFibers++;
        }
      }
    }
    return numFibers;
  }

  void labelPredecessors(Particle *dp, ParticleContainerType* container)
  {
    if (dp->mID != -1 && dp->mID!=dp->ID)
    {
      if (dp->ID!=particles[dp->mID].pID)
      {
        if (dp->ID==particles[dp->mID].mID)
        {
          int tmp = particles[dp->mID].pID;
          particles[dp->mID].pID = particles[dp->mID].mID;
          particles[dp->mID].mID = tmp;
        }
      }
      if (particles[dp->mID].label == 0)
      {
        particles[dp->mID].label = dp->label;
        particles[dp->mID].numerator = dp->numerator-1;
        labelPredecessors(&(particles[dp->mID]), container);
      }
    }

    container->push_back(dp);
  }
  void labelSuccessors(Particle *dp, ParticleContainerType* container)
  {
    if(container->back()->ID!=dp->ID)
      container->push_back(dp);

    if (dp->pID != -1 && dp->pID!=dp->ID)
    {
      if (dp->ID!=particles[dp->pID].mID)
      {
        if (dp->ID==particles[dp->pID].pID)
        {
          int tmp = particles[dp->pID].pID;
          particles[dp->pID].pID = particles[dp->pID].mID;
          particles[dp->pID].mID = tmp;
        }
      }
      if (particles[dp->pID].label == 0)
      {
        particles[dp->pID].label = dp->label;
        particles[dp->pID].numerator = dp->numerator+1;
        labelSuccessors(&(particles[dp->pID]), container);
      }
    }
  }

  void labelrecursivly(Particle *dp, int depth)
  {
    int label = dp->label;

    if (dp->mID != -1)
    {
      if (particles[dp->mID].label == 0)
      {
        particles[dp->mID].label = label;
        labelrecursivly(&(particles[dp->mID]),depth+1);
      }
    }
    if (dp->pID != -1)
    {
      if (particles[dp->pID].label == 0)
      {
        particles[dp->pID].label = label;
        labelrecursivly(&(particles[dp->pID]),depth+1);
      }
    }
  }
};





//klaus static int cmpfloat2(const void *p1,const void *p2)
//klaus {
//klaus   if (((REAL*)p1)[1] > ((REAL*)p2)[1])
//klaus         return 1;
//klaus     else
//klaus         return -1;
//klaus }



typedef std::vector<int> vecint;



//void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
//{
//
//    if(nrhs != 2) {
//  printf("\nUsage: Df = STderivative(f)\n\n",nrhs);
//  printf(" Computes xxx\n");
//  printf(" Parameters:\n");
//  printf("   f - 2D input image of type REAL \n");
//  printf(" Return value Df contains xxx.\n\n");
//    return;
//  } else if(nlhs>2) {
//  printf("Too many output arguments\n");
//    return;
//  }
//
//  fprintf(stderr,"building fibers "); fflush(stderr);
//
//  int pcnt = 0;
//  const mxArray *Points;
//  Points = prhs[pcnt++];
//  int numPoints = mxGetN(Points);
//  REAL *points = (REAL*) mxGetData(Points);
//
//
//  const mxArray *Params;
//  Params = prhs[pcnt++];
//  double *params = (double*) mxGetPr(Params);
//
//
//  int minnumelements = (int) params[0];
//
//
//  CCAnalysis ccana(points,numPoints);
//
//  #ifdef TIMING
//
//  #endif
//
//  int numc = ccana.iterate();
//
//  fprintf(stderr,"."); fflush(stderr);
//
//  vector<vecint> components(numc);
//
//    int i;
//
//  for (i = 0;i < ccana.pcnt;i++)
//  {
//    components[ccana.particles[i].label-1].push_back(i);
//  }
//
//  fprintf(stderr,"."); fflush(stderr);
//
//  for (i = 0; i < numc; i++)
//  {
//    Particle *last = &(ccana.particles[components[i][0]]);
//    last->numerator = 0;
//    Particle *next = (last->pID == -1)? 0 : &(ccana.particles[last->pID]);
//    for (;;)
//    {
//      if (next == 0)
//        break;
//      next->numerator = last->numerator+1;
//      int nextID = -1;
//      if (next->pID != last->ID)
//        nextID = next->pID;
//      if (next->mID != last->ID)
//        nextID = next->mID;
//      last = next;
//      next = (nextID == -1)? 0: &(ccana.particles[nextID]);
//      if (last->numerator > components[i].size()) // circular
//        break;
//    }
//
//    last = &(ccana.particles[components[i][0]]);
//    next = (last->mID == -1)? 0 : &(ccana.particles[last->mID]);
//    for (;;)
//    {
//      if (next == 0)
//        break;
//      next->numerator = last->numerator-1;
//      int nextID = -1;
//      if (next->pID != last->ID)
//        nextID = next->pID;
//      if (next->mID != last->ID)
//        nextID = next->mID;
//      last = next;
//      next = (nextID == -1)? 0: &(ccana.particles[nextID]);
//      if (last->numerator < -components[i].size()) // circular
//        break;
//    }
//
//  }
//
//  fprintf(stderr,"."); fflush(stderr);
//
//
//  #ifdef TIMING
//
//  #endif
//
//  int index = 0;
//  for (i = 0; i < numc; i++)
//  {
//    if (components[i].size() >= minnumelements)
//    {
//      index++;
//    }
//  }
//
//  int cdims[] = {index};
//  plhs[0] = mxCreateCellArray(1,cdims);
//
//  index = 0;
//  for (i = 0; i < numc; i++)
//  {
//    mxArray *ll = 0;
//    if (components[i].size() >= minnumelements)
//    {
//      ll = mxCreateNumericMatrix(2,components[i].size(),mxGetClassID(Points),mxREAL);
//      REAL *dat = (REAL*) mxGetData(ll);
//      for (int k = 0; k < components[i].size(); k++)
//      {
//        dat[2*k] = components[i][k];
//        dat[2*k+1] = ccana.particles[components[i][k]].numerator;
//      }
//      qsort(dat,components[i].size(),sizeof(REAL)*2,cmpfloat2);
//      mxSetCell(plhs[0],index++,ll);
//    }
//  }
//
//  fprintf(stderr,".\n"); fflush(stderr);
//
//}
//

#endif
