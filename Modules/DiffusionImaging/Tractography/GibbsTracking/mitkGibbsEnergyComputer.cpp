#include <mitkGibbsEnergyComputer.h>

#include <vnl/vnl_vector_fixed.h>
#include <vnl/vnl_copy.h>
#include <itkNumericTraits.h>
#include <MitkDiffusionImagingExports.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

using namespace mitk;

GibbsEnergyComputer::GibbsEnergyComputer(ItkQBallImgType* qballImage, ItkFloatImageType* mask, ParticleGrid* particleGrid, SphereInterpolator* interpolator, ItkRandGenType* randGen)
:EnergyComputer(mask, particleGrid, interpolator, randGen)

{
  m_Image = qballImage;
}

float GibbsEnergyComputer::EvaluateOdf(vnl_vector_fixed<float, 3>& pos, vnl_vector_fixed<float, 3> dir)
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

float GibbsEnergyComputer::ComputeExternalEnergy(vnl_vector_fixed<float, 3> &R, vnl_vector_fixed<float, 3> &N, Particle *dp)
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

float GibbsEnergyComputer::ComputeInternalEnergy(Particle *dp)
{
    float energy = 0;

    if (dp->pID != -1)  // has predecessor
        energy += ComputeInternalEnergyConnection(dp,+1);

    if (dp->mID != -1)  // has successor
        energy += ComputeInternalEnergyConnection(dp,-1);

    return energy;
}

float GibbsEnergyComputer::ComputeInternalEnergyConnection(Particle *p1,int ep1)
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

float GibbsEnergyComputer::ComputeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2)
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