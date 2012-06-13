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

#include "mitkEnergyComputer.h"

using namespace mitk;

EnergyComputer::EnergyComputer(MTRand* rgen, ItkQBallImgType* data, const int *dsz,  double *cellsize, SphereInterpolator *sp, ParticleGrid *pcon, float *spimg, int spmult, vnl_matrix_fixed<float, 3, 3> rotMatrix)
{
    mtrand = rgen;
    m_RotationMatrix = rotMatrix;
    m_QBallImageData = data;
    m_QBallImageSize = dsz;
    m_SphereInterpolator = sp;

    m_MaskImageData = spimg;

    nip = m_QBallImageSize[0];


    w = m_QBallImageSize[1];
    h = m_QBallImageSize[2];
    d = m_QBallImageSize[3];

    voxsize_w = cellsize[0];
    voxsize_h = cellsize[1];
    voxsize_d = cellsize[2];


    w_sp = m_QBallImageSize[1]*spmult;
    h_sp = m_QBallImageSize[2]*spmult;
    d_sp = m_QBallImageSize[3]*spmult;

    voxsize_sp_w = cellsize[0]/spmult;
    voxsize_sp_h = cellsize[1]/spmult;
    voxsize_sp_d = cellsize[2]/spmult;


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

void EnergyComputer::setParameters(float pwei,float pwid,float chempot_connection, float length,float curv_hardthres, float inex_balance, float chempot2, float meanv)
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

void EnergyComputer::drawSpatPosition(vnl_vector_fixed<float, 3>& R)
{
    float r = mtrand->frand();
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
    R[0] = voxsize_sp_w*((float)(spatidx[rh-1] % w_sp)  + mtrand->frand());
    R[1] = voxsize_sp_h*((float)((spatidx[rh-1]/w_sp) % h_sp)  + mtrand->frand());
    R[2] = voxsize_sp_d*((float)(spatidx[rh-1]/(w_sp*h_sp))    + mtrand->frand());
}

float EnergyComputer::SpatProb(vnl_vector_fixed<float, 3> R)
{
    int rx = int(R[0]/voxsize_sp_w);
    int ry = int(R[1]/voxsize_sp_h);
    int rz = int(R[2]/voxsize_sp_d);
    if (rx >= 0 && rx < w_sp && ry >= 0 && ry < h_sp && rz >= 0 && rz < d_sp){
        return m_MaskImageData[rx + w_sp* (ry + h_sp*rz)];
    }
    else
        return 0;
}

float EnergyComputer::evaluateODF(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, float &len)
{
    const int CU = 10;
    vnl_vector_fixed<float, 3> Rs;
    float Dn = 0;
    int xint,yint,zint,spatindex;

    vnl_vector_fixed<float, 3> n;
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

            ItkQBallImgType::IndexType index;
            float weight;

            weight = (1-xfrac)*(1-yfrac)*(1-zfrac);
            index[0] = xint; index[1] = yint; index[2] = zint;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (xfrac)*(1-yfrac)*(1-zfrac);
            index[0] = xint+1; index[1] = yint; index[2] = zint;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (1-xfrac)*(yfrac)*(1-zfrac);
            index[0] = xint; index[1] = yint+1; index[2] = zint;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (1-xfrac)*(1-yfrac)*(zfrac);
            index[0] = xint; index[1] = yint; index[2] = zint+1;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (xfrac)*(yfrac)*(1-zfrac);
            index[0] = xint+1; index[1] = yint+1; index[2] = zint;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (1-xfrac)*(yfrac)*(zfrac);
            index[0] = xint; index[1] = yint+1; index[2] = zint+1;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (xfrac)*(1-yfrac)*(zfrac);
            index[0] = xint+1; index[1] = yint; index[2] = zint+1;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

            weight = (xfrac)*(yfrac)*(zfrac);
            index[0] = xint+1; index[1] = yint+1; index[2] = zint+1;
            Dn += (m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                   m_QBallImageData->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;
        }
    }
    Dn *= 1.0/(2*CU+1);
    return Dn;
}

float EnergyComputer::computeExternalEnergy(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, float &cap, float &len, Particle *dp)
{
    float m = SpatProb(R);
    if (m == 0)
        return -INFINITY;

    float Dn = evaluateODF(R,N,len);

    float Sn = 0;
    float Pn = 0;

    m_ParticleGrid->ComputeNeighbors(R);
    for (;;)
    {
        Particle *p =  m_ParticleGrid->GetNextNeighbor();
        if (p == 0) break;
        if (dp != p)
        {
            float dot = fabs(dot_product(N,p->N));
            float bw = mbesseli0(dot);
            float dpos = (p->R-R).squared_magnitude();
            float w = mexp(dpos*gamma_s);
            Sn += w*(bw+chempot2)*p->cap ;
            w = mexp(dpos*gamma_reg_s);
            Pn += w*bw;
        }
    }

    float energy = 0;
    energy += (2*(Dn/particle_weight-Sn) - (mbesseli0(1.0)+chempot2)*cap)*cap;

    return energy*ex_strength;
}

float EnergyComputer::computeInternalEnergy(Particle *dp)
{
    float energy = eigen_energy;

    if (dp->pID != -1)
        energy += computeInternalEnergyConnection(dp,+1);

    if (dp->mID != -1)
        energy += computeInternalEnergyConnection(dp,-1);

    return energy;
}

float EnergyComputer::computeInternalEnergyConnection(Particle *p1,int ep1)
{
    Particle *p2 = 0;
    int ep2;
    if (ep1 == 1)
        p2 = m_ParticleGrid->GetParticle(p1->pID);
    else
        p2 = m_ParticleGrid->GetParticle(p1->mID);
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

float EnergyComputer::computeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2)
{
    if ((dot_product(p1->N,p2->N))*ep1*ep2 > -curv_hard)
        return -INFINITY;

    vnl_vector_fixed<float, 3> R1 = p1->R + (p1->N * (p1->len * ep1));
    vnl_vector_fixed<float, 3> R2 = p2->R + (p2->N * (p2->len * ep2));

    if ((R1-R2).squared_magnitude() > particle_length_sq)
        return -INFINITY;

    vnl_vector_fixed<float, 3> R = (p2->R + p1->R); R *= 0.5;

    if (SpatProb(R) == 0)
        return -INFINITY;

    float norm1 = (R1-R).squared_magnitude();
    float norm2 = (R2-R).squared_magnitude();


    float energy = (eigencon_energy-norm1-norm2)*in_strength;

    return energy;
}

float EnergyComputer::mbesseli0(float x)
{
    //    BESSEL_APPROXCOEFF[0] = -0.1714;
    //    BESSEL_APPROXCOEFF[1] = 0.5332;
    //    BESSEL_APPROXCOEFF[2] = -1.4889;
    //    BESSEL_APPROXCOEFF[3] = 2.0389;
    float y = x*x;
    float erg = -0.1714;
    erg += y*0.5332;
    erg += y*y*-1.4889;
    erg += y*y*y*2.0389;
    return erg;
}

float EnergyComputer::mexp(float x)
{
    return((x>=7.0) ? 0 : ((x>=5.0) ? (-0.0029*x+0.0213) : ((x>=3.0) ? (-0.0215*x+0.1144) : ((x>=2.0) ? (-0.0855*x+0.3064) : ((x>=1.0) ? (-0.2325*x+0.6004) : ((x>=0.5) ? (-0.4773*x+0.8452) : ((x>=0.0) ? (-0.7869*x+1.0000) : 1 )))))));
    //  return exp(-x);
}

//ComputeFiberCorrelation()
//{
//    float bD = 15;

//    vnl_matrix_fixed<double, 3, QBALL_ODFSIZE> bDir =
//        *itk::PointShell<QBALL_ODFSIZE, vnl_matrix_fixed<double, 3, QBALL_ODFSIZE> >::DistributePointShell();

//    const int N = QBALL_ODFSIZE;

//    vnl_matrix_fixed<double, QBALL_ODFSIZE, 3> temp = bDir.transpose();
//    vnl_matrix_fixed<double, N, N> C = temp*bDir;
//    vnl_matrix_fixed<double, N, N> Q = C;
//    vnl_vector_fixed<double, N> mean;
//    for(int i=0; i<N; i++)
//    {
//      double tempMean = 0;
//      for(int j=0; j<N; j++)
//      {
//        C(i,j) = abs(C(i,j));
//        Q(i,j) = exp(-bD * C(i,j) * C(i,j));
//        tempMean += Q(i,j);
//      }
//      mean[i] = tempMean/N;
//    }

//    vnl_matrix_fixed<double, N, N> repMean;
//    for (int i=0; i<N; i++)
//      repMean.set_row(i, mean);
//    Q -= repMean;

//    vnl_matrix_fixed<double, N, N> P = Q*Q;

//    std::vector<const double *> pointer;
//    pointer.reserve(N*N);
//    double * start = C.data_block();
//    double * end =  start + N*N;
//    for (double * iter = start; iter != end; ++iter)
//    {
//      pointer.push_back(iter);
//    }
//    std::sort(pointer.begin(), pointer.end(), LessDereference());

//    vnl_vector_fixed<double,N*N> alpha;
//    vnl_vector_fixed<double,N*N> beta;
//    for (int i=0; i<N*N; i++) {
//      alpha(i) = *pointer[i];
//      beta(i)  = *(P.data_block()+(pointer[i]-start));
//    }

//    double nfac = sqrt(beta(N*N-1));
//    beta = beta / (nfac*nfac);
//    Q = Q / nfac;

//    double sum = 0;
//    for(int i=0; i<N; i++)
//    {
//      sum += Q(0,i);
//    }
//    // if left to default 0
//    // then mean is not substracted in order to correct odf integral
//    // this->m_Meanval_sq = (sum*sum)/N;

//    vnl_vector_fixed<double,N*N> alpha_0;
//    vnl_vector_fixed<double,N*N> alpha_2;
//    vnl_vector_fixed<double,N*N> alpha_4;
//    vnl_vector_fixed<double,N*N> alpha_6;
//    for(int i=0; i<N*N; i++)
//    {
//      alpha_0(i) = 1;
//      alpha_2(i) = alpha(i)*alpha(i);
//      alpha_4(i) = alpha_2(i)*alpha_2(i);
//      alpha_6(i) = alpha_4(i)*alpha_2(i);
//    }

//    vnl_matrix_fixed<double, N*N, 4> T;
//    T.set_column(0,alpha_0);
//    T.set_column(1,alpha_2);
//    T.set_column(2,alpha_4);
//    T.set_column(3,alpha_6);

//    vnl_vector_fixed<double,4> coeff = vnl_matrix_inverse<double>(T).pinverse()*beta;

//    MITK_INFO << "itkGibbsTrackingFilter: Bessel oefficients: " << coeff;

//    BESSEL_APPROXCOEFF = new float[4];

//    BESSEL_APPROXCOEFF[0] = coeff(0);
//    BESSEL_APPROXCOEFF[1] = coeff(1);
//    BESSEL_APPROXCOEFF[2] = coeff(2);
//    BESSEL_APPROXCOEFF[3] = coeff(3);
//    BESSEL_APPROXCOEFF[0] = -0.1714;
//    BESSEL_APPROXCOEFF[1] = 0.5332;
//    BESSEL_APPROXCOEFF[2] = -1.4889;
//    BESSEL_APPROXCOEFF[3] = 2.0389;
//}
