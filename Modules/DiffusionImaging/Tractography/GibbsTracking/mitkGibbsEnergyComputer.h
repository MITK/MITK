#ifndef GIBBSENERGYCOMPUTER_H
#define GIBBSENERGYCOMPUTER_H

#include <MitkDiffusionImagingExports.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkParticleGrid.h>
#include <mitkSphereInterpolator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

#include "mitkEnergyComputer.h"


using namespace mitk;

/**
* \brief ODF lookuptable based energy computer.   */

class MitkDiffusionImaging_EXPORT GibbsEnergyComputer : public EnergyComputer
{
  public:

    typedef itk::Vector<float, QBALL_ODFSIZE>   OdfVectorType;
    typedef itk::Image<OdfVectorType, 3>        ItkQBallImgType;
    typedef itk::Image<float, 3>                ItkFloatImageType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator ItkRandGenType;

    GibbsEnergyComputer(ItkQBallImgType* qballImage, ItkFloatImageType* mask, ParticleGrid* particleGrid, SphereInterpolator* interpolator, ItkRandGenType* randGen);

    // external energy calculation
    float ComputeExternalEnergy(vnl_vector_fixed<float, 3>& R, vnl_vector_fixed<float, 3>& N, Particle* dp);

    // internal energy calculation
    float ComputeInternalEnergyConnection(Particle *p1,int ep1);
    float ComputeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2);
    float ComputeInternalEnergy(Particle *dp);

    float EvaluateOdf(vnl_vector_fixed<float, 3>& pos, vnl_vector_fixed<float, 3> dir);
  protected:

    ItkQBallImgType*                m_Image;

};

#endif
