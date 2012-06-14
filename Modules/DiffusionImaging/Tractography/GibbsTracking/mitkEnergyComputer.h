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
#include <itkOrientationDistributionFunction.h>
#include <mitkParticleGrid.h>
#include <mitkSphereInterpolator.h>
#include <MersenneTwister.h>

using namespace mitk;

class MitkDiffusionImaging_EXPORT EnergyComputer
{

public:

    typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
    typedef itk::Image<OdfVectorType, 3>        ItkQBallImgType;
    typedef itk::Image<float, 3>                ItkFloatImageType;

    vnl_matrix_fixed<float, 3, 3>   m_RotationMatrix;
    ItkQBallImgType*                m_ImageData;
    vnl_vector_fixed<int, 3>        m_Size;
    vnl_vector_fixed<float, 3>      m_Spacing;
    SphereInterpolator*             m_SphereInterpolator;
    ParticleGrid*                   m_ParticleGrid;

    std::vector< float >            cumulspatprob;
    std::vector< int >              spatidx;

    bool m_UseTrilinearInterpolation;

    float *m_MaskImageData;
    int m_NumActiveVoxels;

    int w,h,d;
    float voxsize_w;
    float voxsize_h;
    float voxsize_d;

    int w_sp,h_sp,d_sp;
    float voxsize_sp_w;
    float voxsize_sp_h;
    float voxsize_sp_d;
    MTRand* m_RandGen;

    float eigen_energy;
    int nip; // number of data vertices on sphere


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


    EnergyComputer(MTRand* rgen, ItkQBallImgType* qballImage, SphereInterpolator *sp, ParticleGrid *pcon, float *mask, vnl_matrix_fixed<float, 3, 3> rotMatrix);

    void setParameters(float pwei,float pwid,float chempot_connection, float length,float curv_hardthres, float inex_balance, float chempot2, float meanv);

    void drawSpatPosition(vnl_vector_fixed<float, 3>& R);

    float SpatProb(vnl_vector_fixed<float, 3> R);

    float evaluateODF(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, float &len);

    float computeExternalEnergy(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, float &len, Particle *dp);

    float computeInternalEnergy(Particle *dp);

    float computeInternalEnergyConnection(Particle *p1,int ep1);

    float computeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2);

    float mbesseli0(float x);

    float mexp(float x);
};

#endif
