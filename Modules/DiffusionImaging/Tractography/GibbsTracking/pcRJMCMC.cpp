#ifndef __pcRJMCMC_cpp__
#define __pcRJMCMC_cpp__

//#include "mex.h"
//#include "matrix.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include <map>
#include <string.h>
#include <time.h>

using namespace std;

#define float float
#define PI M_PI
//#define INFINITY 99999999999.0

//#define TIMING

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
    //mexPrintf("%s  #proposals: %8.2fk  acceptratio: %.2f \% \n",s,1.0*N/1000.0,100.0*accept/N);
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
    //mexPrintf("%s total: %10.2fms calls: %10.1fk t/call: %10.3fms \n",s,time/1000.0,1.0*ncalls/1000.0,1.0*time/ncalls);
  }

  void report_time(const char *s)
  {
    //mexPrintf("%s: %.2fms \n",s,time/1000.0);
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

Timing externalenergy_time;
Timing internalenergy_time;
Timing odfeval_time;
Timing total_time;

Timing shiftproposal_time;
Timing birthproposal_time;
Timing deathproposal_time;
Timing capproposal_time;
Timing lenproposal_time;
Timing connproposal_time;

PropStats deathstats;
PropStats birthstats;
PropStats connstats;
PropStats shiftstats;
PropStats capstats;
PropStats lenstats;


#endif



#include "MersenneTwister.h"
MTRand mtrand;
float *BESSEL_APPROXCOEFF;


#include "EnergyComputer_connec.cpp"
//#include "EnergyComputer_center.cpp"
//#include "RJMCMC_singlegradprop.cpp"
#include "RJMCMC_randshift.cpp"


//void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[] )
//{
 	
//    if(nrhs != 7 && nrhs != 8) {
	
//	printf(" wrong usage!!!.\n\n");
//    return;
//	} else if(nlhs>3) {
//	printf("Too many output arguments\n");
//    return;
//	}
	
//	int pcnt = 0;
//	const mxArray *Points;
//	Points = prhs[pcnt++];
//	int numPoints = mxGetN(Points);
//  float *points = (float*) mxGetData(Points);
	
//	const mxArray *DataImg;
//	DataImg = prhs[pcnt++];
//  float *dimg = (float*) mxGetData(DataImg);
//	const int *dsize = mxGetDimensions(DataImg);
    
//	const mxArray *Mask;
//	Mask = prhs[pcnt++];
//  float *mask = (float*) mxGetData(Mask);
//    const int *dmsize = mxGetDimensions(Mask);
//    int mask_oversamp_mult = dmsize[0]/dsize[1];
    
    
    
//	const mxArray *VoxSize;
//	VoxSize = prhs[pcnt++];
//	double *voxsize = (double*) mxGetPr(VoxSize);
            
    

//	const mxArray *Params;
//	Params = prhs[pcnt++];
//	double *params = (double*) mxGetPr(Params);

//  float Temp = (float) params[0];
//	int numit = (int) params[1];
//  float conprob = (float) params[2];
//  float particle_weight = (float) params[3];
//  float particle_width = (float) params[4];
//  float particle_len = (float) params[5];
//  float chempot_connection = (float) params[6];
//  float chempot_particle = (float) params[7];
//  float inex_balance = (float) params[8];
//  float chempot2 = (float) params[9];
//  float meanval_sq = (float) params[10];

//    const mxArray *BesselExpansion;
//	BesselExpansion = prhs[pcnt++];
//  BESSEL_APPROXCOEFF = (float*) mxGetData(BesselExpansion);
    
    
    
//	// read spherical-interpolator data

//	const mxArray *sinterpstruct = prhs[pcnt++];
//	mxArray *Indices = mxGetField(sinterpstruct,0,"indices");
//	mxArray *BaryCoords = mxGetField(sinterpstruct,0,"barycoords");
//	mxArray *Beta = mxGetField(sinterpstruct,0,"beta");
//	mxArray *NumInterpPoints = mxGetField(sinterpstruct,0,"numpoints");
	
//  float *indimg = (float*) mxGetData(Indices);
//	const int *isize = mxGetDimensions(Indices);
//	int totsz = isize[0]*isize[1]*isize[2]*isize[3];
//	int *indeximg = (int*) malloc(sizeof(int)*totsz);
//	for (int k =0;k < totsz;k++)
//		indeximg[k] = int(indimg[k])-1;
//  float *barycoords = (float*) mxGetData(BaryCoords);
//  float *beta = (float*) mxGetData(Beta);
//  int nip = int(*((float*)mxGetData(NumInterpPoints)));
	
//    SphereInterpolator *sinterp = new SphereInterpolator(barycoords,indeximg,nip,isize[2],beta[0]);
	
//	double breakhandle = 0;
//	const mxArray *BreakHandle;
//	if (nrhs == 8)
//	{
//		BreakHandle = prhs[pcnt++];
//		breakhandle = *mxGetPr(BreakHandle);
//	}

//	#ifdef TIMING
//	externalenergy_time.clear();
//	internalenergy_time.clear();
//	odfeval_time.clear();
//	total_time.clear();

//	shiftproposal_time.clear();
//	birthproposal_time.clear();
//	deathproposal_time.clear();
//	capproposal_time.clear();
//	lenproposal_time.clear();
//	connproposal_time.clear();

//	deathstats.clear();
//	birthstats.clear();
//	connstats.clear();
//	shiftstats.clear();
//	capstats.clear();
//	lenstats.clear();
//	#endif
	


//  float cellsize = 2*particle_len;
//  float curv_hardthres = 0.7;

//    fprintf(stderr,"setting up MH-sampler \n"); fflush(stderr);
//	RJMCMC sampler(points,numPoints, dimg, dsize, voxsize, cellsize);
//    fprintf(stderr,"setting up Energy-computer \n"); fflush(stderr);
//    EnergyComputer encomp(dimg,dsize,voxsize,sinterp,&(sampler.pcontainer),mask,mask_oversamp_mult);

//    fprintf(stderr,"setting up parameters\n"); fflush(stderr);
//	sampler.setParameters(Temp,numit,conprob,particle_len,curv_hardthres,chempot_particle);
//   	sampler.setEnergyComputer(&encomp);
//    encomp.setParameters(particle_weight,particle_width,chempot_connection*particle_len*particle_len,particle_len,curv_hardthres,inex_balance,chempot2,meanval_sq);
    
//    fprintf(stderr,"starting to iterate\n"); fflush(stderr);
//	sampler.iterate(breakhandle);
	
//	int cnt = sampler.pcontainer.pcnt;
//	#ifdef TIMING
//	mexPrintf("\nEnergy\n------------------------\n");
//	externalenergy_time.report("external  ");
//	odfeval_time.report("odfeval   ");
//	internalenergy_time.report("internal  ");
//	total_time.report_time("total energy comp.  ");
	
//	mexPrintf("overhead for proposals und stuff:%.1fms\n",
//	(total_time.time-(externalenergy_time.time+odfeval_time.time+internalenergy_time.time))/1000.0);

//	mexPrintf("\nProposals\n------------------------\n");
//	birthproposal_time.report("birth  ");
//	deathproposal_time.report("death  ");
//	shiftproposal_time.report("shift  ");
//	connproposal_time.report("conne  ");
////	capproposal_time.report("capch  ");
////	lenproposal_time.report("length ");
//	mexPrintf("\n");
//	birthstats.report("birth  ");
//	deathstats.report("death  ");
//	shiftstats.report("shift  ");
//	connstats.report("conne  ");
////	lenstats.report("length ");
////	capstats.report("capch  ");
//	mexPrintf("\n");



//	#endif

	
//	int dims[] = {sampler.attrcnt, sampler.pcontainer.pcnt};
//  plhs[0] = mxCreateNumericArray(2,dims,mxGetClassID(Points),mxfloat);
//  float *npoints = (float*) mxGetData(plhs[0]);
//	sampler.writeout(npoints);
	

//	delete sinterp;
//	free(indeximg);


//}

#endif

