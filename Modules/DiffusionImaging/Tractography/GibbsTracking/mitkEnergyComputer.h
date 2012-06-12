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

#ifndef _ENCOMP
#define _ENCOMP

#include <MitkDiffusionImagingExports.h>
#include "mitkParticleGrid.h"
#include "mitkSphereInterpolator.h"
#include "MersenneTwister.h"
#include <vnl/vnl_matrix_fixed.h>
#include <fstream>

using namespace mitk;

class MitkDiffusionImaging_EXPORT EnergyComputer
{

public:
    float eigen_energy;
    vnl_matrix_fixed<float, 3, 3> m_RotationMatrix;
    float *m_QBallImageData;
    const int *m_QBallImageSize;
    SphereInterpolator *m_SphereInterpolator;
    ParticleGrid *m_ParticleGrid;

    int w,h,d;
    float voxsize_w;
    float voxsize_h;
    float voxsize_d;

    int w_sp,h_sp,d_sp;
    float voxsize_sp_w;
    float voxsize_sp_h;
    float voxsize_sp_d;
    MTRand* mtrand;

    int nip; // number of data vertices on sphere

    float *m_MaskImageData;
    float *cumulspatprob;
    int *spatidx;
    int scnt;

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


    EnergyComputer(MTRand* rgen, float *data, const int *dsz,  double *cellsize, SphereInterpolator *sp, ParticleGrid *pcon, float *spimg, int spmult, vnl_matrix_fixed<float, 3, 3> rotMatrix);

    void setParameters(float pwei,float pwid,float chempot_connection, float length,float curv_hardthres, float inex_balance, float chempot2, float meanv);

    void drawSpatPosition(vnl_vector_fixed<float, 3>& R);

    float SpatProb(vnl_vector_fixed<float, 3> R);

    float evaluateODF(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, float &len);

    float computeExternalEnergy(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, float &cap, float &len, Particle *dp);

    float computeInternalEnergy(Particle *dp);

    float computeInternalEnergyConnection(Particle *p1,int ep1);

    float computeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2);

    float mbesseli0(float x);

    float mexp(float x);
};

#endif
