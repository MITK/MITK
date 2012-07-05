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
#include <vnl/vnl_copy.h>
#include <itkNumericTraits.h>

using namespace mitk;

EnergyComputer::EnergyComputer(ItkQBallImgType* qballImage, ItkFloatImageType* mask, ParticleGrid* particleGrid, SphereInterpolator* interpolator, ItkRandGenType* randGen)
    : m_UseTrilinearInterpolation(true)
{
    m_ParticleGrid = particleGrid;
    m_RandGen = randGen;
    m_Image = qballImage;
    m_SphereInterpolator = interpolator;
    m_Mask = mask;

    m_ParticleLength = m_ParticleGrid->m_ParticleLength;
    m_SquaredParticleLength = m_ParticleLength*m_ParticleLength;

    m_Size[0] = m_Image->GetLargestPossibleRegion().GetSize()[0];
    m_Size[1] = m_Image->GetLargestPossibleRegion().GetSize()[1];
    m_Size[2] = m_Image->GetLargestPossibleRegion().GetSize()[2];

    if (m_Size[0]<3 || m_Size[1]<3 || m_Size[2]<3)
        m_UseTrilinearInterpolation = false;

    m_Spacing[0] = m_Image->GetSpacing()[0];
    m_Spacing[1] = m_Image->GetSpacing()[1];
    m_Spacing[2] = m_Image->GetSpacing()[2];

    // calculate rotation matrix
    vnl_matrix<double> temp = m_Image->GetDirection().GetVnlMatrix();
    vnl_matrix<float>  directionMatrix; directionMatrix.set_size(3,3);
    vnl_copy(temp, directionMatrix);
    vnl_vector_fixed<float, 3> d0 = directionMatrix.get_column(0); d0.normalize();
    vnl_vector_fixed<float, 3> d1 = directionMatrix.get_column(1); d1.normalize();
    vnl_vector_fixed<float, 3> d2 = directionMatrix.get_column(2); d2.normalize();
    directionMatrix.set_column(0, d0);
    directionMatrix.set_column(1, d1);
    directionMatrix.set_column(2, d2);
    vnl_matrix_fixed<float, 3, 3> I = directionMatrix*directionMatrix.transpose();
    if(!I.is_identity(mitk::eps))
        fprintf(stderr,"itkGibbsTrackingFilter: image direction is not a rotation matrix. Tracking not possible!\n");
    m_RotationMatrix = directionMatrix;

    if (QBALL_ODFSIZE != m_SphereInterpolator->nverts)
        fprintf(stderr,"EnergyComputer: error during init: data does not match with interpolation scheme\n");

    int totsz = m_Size[0]*m_Size[1]*m_Size[2];
    m_CumulatedSpatialProbability.resize(totsz, 0.0); // +1?
    m_ActiveIndices.resize(totsz, 0);

    // calculate active voxels and cumulate probabilities
    m_NumActiveVoxels = 0;
    m_CumulatedSpatialProbability[0] = 0;
    for (int x = 0; x < m_Size[0];x++)
        for (int y = 0; y < m_Size[1];y++)
            for (int z = 0; z < m_Size[2];z++)
            {
                int idx = x+(y+z*m_Size[1])*m_Size[0];
                ItkFloatImageType::IndexType index;
                index[0] = x; index[1] = y; index[2] = z;
                if (m_Mask->GetPixel(index) > 0.5)
                {
                    m_CumulatedSpatialProbability[m_NumActiveVoxels+1] = m_CumulatedSpatialProbability[m_NumActiveVoxels] + m_Mask->GetPixel(index);
                    m_ActiveIndices[m_NumActiveVoxels] = idx;
                    m_NumActiveVoxels++;
                }
            }
    for (int k = 0; k < m_NumActiveVoxels; k++)
        m_CumulatedSpatialProbability[k] /= m_CumulatedSpatialProbability[m_NumActiveVoxels];

    std::cout << "EnergyComputer: " << m_NumActiveVoxels << " active voxels found" << std::endl;
}

void EnergyComputer::SetParameters(float particleWeight, float particleWidth, float connectionPotential, float curvThres, float inexBalance, float particlePotential)
{
    m_ParticleChemicalPotential = particlePotential;
    m_ConnectionPotential = connectionPotential;
    m_ParticleWeight = particleWeight;

    float bal = 1/(1+exp(-inexBalance));
    m_ExtStrength = 2*bal;
    m_IntStrength = 2*(1-bal)/m_SquaredParticleLength;

    m_CurvatureThreshold = curvThres;

    float sigma_s = particleWidth;
    gamma_s = 1/(sigma_s*sigma_s);
    gamma_reg_s =1/(m_SquaredParticleLength/4);
}

// draw random position from active voxels
void EnergyComputer::DrawRandomPosition(vnl_vector_fixed<float, 3>& R)
{
    float r = m_RandGen->GetVariate();//m_RandGen->frand();
    int j;
    int rl = 1;
    int rh = m_NumActiveVoxels;
    while(rh != rl)
    {
        j = rl + (rh-rl)/2;
        if (r < m_CumulatedSpatialProbability[j])
        {
            rh = j;
            continue;
        }
        if (r > m_CumulatedSpatialProbability[j])
        {
            rl = j+1;
            continue;
        }
        break;
    }
    R[0] = m_Spacing[0]*((float)(m_ActiveIndices[rh-1] % m_Size[0])  + m_RandGen->GetVariate());
    R[1] = m_Spacing[1]*((float)((m_ActiveIndices[rh-1]/m_Size[0]) % m_Size[1])  + m_RandGen->GetVariate());
    R[2] = m_Spacing[2]*((float)(m_ActiveIndices[rh-1]/(m_Size[0]*m_Size[1]))    + m_RandGen->GetVariate());
}

// return spatial probability of position
float EnergyComputer::SpatProb(vnl_vector_fixed<float, 3> pos)
{
    ItkFloatImageType::IndexType index;
    index[0] = floor(pos[0]/m_Spacing[0]);
    index[1] = floor(pos[1]/m_Spacing[1]);
    index[2] = floor(pos[2]/m_Spacing[2]);

    if (m_Mask->GetLargestPossibleRegion().IsInside(index)) // is inside image?
        return m_Mask->GetPixel(index);
    else
        return 0;
}

float EnergyComputer::EvaluateOdf(vnl_vector_fixed<float, 3>& pos, vnl_vector_fixed<float, 3> dir)
{
    const int sampleSteps = 10;             // evaluate ODF at 2*sampleSteps+1 positions along dir
    vnl_vector_fixed<float, 3> samplePos;   // current position to evaluate
    float result = 0;                       // average of sampled ODF values
    int xint, yint, zint;                   // voxel containing samplePos

    // rotate particle direction according to image rotation
    dir = m_RotationMatrix*dir;

    // get interpolation for rotated direction
    m_SphereInterpolator->getInterpolation(dir);

    // sample ODF values along particle direction
    for (int i=-sampleSteps; i <= sampleSteps;i++)
    {
        samplePos = pos + (dir * m_ParticleLength) * ((float)i/sampleSteps);

        if (!m_UseTrilinearInterpolation)   // image has not enough slices to use trilinear interpolation
        {
            ItkQBallImgType::IndexType index;
            index[0] = floor(pos[0]/m_Spacing[0]);
            index[1] = floor(pos[1]/m_Spacing[1]);
            index[2] = floor(pos[2]/m_Spacing[2]);
            if (m_Image->GetLargestPossibleRegion().IsInside(index))
            {
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2]);
            }
        }
        else    // use trilinear interpolation
        {
            float Rx = samplePos[0]/m_Spacing[0]-0.5;
            float Ry = samplePos[1]/m_Spacing[1]-0.5;
            float Rz = samplePos[2]/m_Spacing[2]-0.5;

            xint = floor(Rx);
            yint = floor(Ry);
            zint = floor(Rz);

            if (xint >= 0 && xint < m_Size[0]-1 && yint >= 0 && yint < m_Size[1]-1 && zint >= 0 && zint < m_Size[2]-1)
            {
                float xfrac = Rx-xint;
                float yfrac = Ry-yint;
                float zfrac = Rz-zint;

                ItkQBallImgType::IndexType index;
                float weight;

                weight = (1-xfrac)*(1-yfrac)*(1-zfrac);
                index[0] = xint; index[1] = yint; index[2] = zint;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (xfrac)*(1-yfrac)*(1-zfrac);
                index[0] = xint+1; index[1] = yint; index[2] = zint;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (1-xfrac)*(yfrac)*(1-zfrac);
                index[0] = xint; index[1] = yint+1; index[2] = zint;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (1-xfrac)*(1-yfrac)*(zfrac);
                index[0] = xint; index[1] = yint; index[2] = zint+1;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (xfrac)*(yfrac)*(1-zfrac);
                index[0] = xint+1; index[1] = yint+1; index[2] = zint;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (1-xfrac)*(yfrac)*(zfrac);
                index[0] = xint; index[1] = yint+1; index[2] = zint+1;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (xfrac)*(1-yfrac)*(zfrac);
                index[0] = xint+1; index[1] = yint; index[2] = zint+1;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;

                weight = (xfrac)*(yfrac)*(zfrac);
                index[0] = xint+1; index[1] = yint+1; index[2] = zint+1;
                result += (m_Image->GetPixel(index)[m_SphereInterpolator->idx[0]-1]*m_SphereInterpolator->interpw[0] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[1]-1]*m_SphereInterpolator->interpw[1] +
                       m_Image->GetPixel(index)[m_SphereInterpolator->idx[2]-1]* m_SphereInterpolator->interpw[2])*weight;
            }
        }
    }
    result /= (2*sampleSteps+1);    // average result over taken samples
    return result;
}

float EnergyComputer::ComputeExternalEnergy(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, Particle *dp)
{
    if (SpatProb(R) == 0)   // check if position is inside mask
        return itk::NumericTraits<float>::NonpositiveMin();

    float odfVal = EvaluateOdf(R, N);   // evaluate ODF in given direction

    float modelVal = 0;
    m_ParticleGrid->ComputeNeighbors(R);    // retrieve neighbouring particles from particle grid
    Particle* neighbour =  m_ParticleGrid->GetNextNeighbor();
    while (neighbour!=NULL)                         // iterate over nieghbouring particles
    {
        if (dp != neighbour)                        // don't evaluate against itself
        {
            // see Reisert et al. "Global Reconstruction of Neuronal Fibers", MICCAI 2009
            float dot = fabs(dot_product(N,neighbour->dir));
            float bw = mbesseli0(dot);
            float dpos = (neighbour->pos-R).squared_magnitude();
            float w = mexp(dpos*gamma_s);
            modelVal += w*(bw+m_ParticleChemicalPotential);
            w = mexp(dpos*gamma_reg_s);
        }
        neighbour =  m_ParticleGrid->GetNextNeighbor();
    }

    float energy = 2*(odfVal/m_ParticleWeight-modelVal) - (mbesseli0(1.0)+m_ParticleChemicalPotential);
    return energy*m_ExtStrength;
}

float EnergyComputer::ComputeInternalEnergy(Particle *dp)
{
    float energy = 0;

    if (dp->pID != -1)  // has predecessor
        energy += ComputeInternalEnergyConnection(dp,+1);

    if (dp->mID != -1)  // has successor
        energy += ComputeInternalEnergyConnection(dp,-1);

    return energy;
}

float EnergyComputer::ComputeInternalEnergyConnection(Particle *p1,int ep1)
{
    Particle *p2 = 0;
    int ep2;

    if (ep1 == 1)
        p2 = m_ParticleGrid->GetParticle(p1->pID);  // get predecessor
    else
        p2 = m_ParticleGrid->GetParticle(p1->mID);  // get successor

    // check in which direction the connected particle is pointing
    if (p2->mID == p1->ID)
        ep2 = -1;
    else if (p2->pID == p1->ID)
        ep2 = 1;
    else
       std::cout << "EnergyComputer: Connections are inconsistent!" << std::endl;

    return ComputeInternalEnergyConnection(p1,ep1,p2,ep2);
}

float EnergyComputer::ComputeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2)
{
    // see Reisert et al. "Global Reconstruction of Neuronal Fibers", MICCAI 2009
    if ((dot_product(p1->dir,p2->dir))*ep1*ep2 > -m_CurvatureThreshold)     // angle between particles is too sharp
        return itk::NumericTraits<float>::NonpositiveMin();

    // calculate the endpoints of the two particles
    vnl_vector_fixed<float, 3> endPoint1 = p1->pos + (p1->dir * (m_ParticleLength * ep1));
    vnl_vector_fixed<float, 3> endPoint2 = p2->pos + (p2->dir * (m_ParticleLength * ep2));

    // check if endpoints are too far apart to connect
    if ((endPoint1-endPoint2).squared_magnitude() > m_SquaredParticleLength)
        return itk::NumericTraits<float>::NonpositiveMin();

    // calculate center point of the two particles
    vnl_vector_fixed<float, 3> R = (p2->pos + p1->pos); R *= 0.5;

    // they are not allowed to connect if the mask image does not allow it
    if (SpatProb(R) == 0)
        return itk::NumericTraits<float>::NonpositiveMin();

    // get distances of endpoints to center point
    float norm1 = (endPoint1-R).squared_magnitude();
    float norm2 = (endPoint2-R).squared_magnitude();

    // calculate actual internal energy
    float energy = (m_ConnectionPotential-norm1-norm2)*m_IntStrength;
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

int EnergyComputer::GetNumActiveVoxels()
{
    return m_NumActiveVoxels;
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
