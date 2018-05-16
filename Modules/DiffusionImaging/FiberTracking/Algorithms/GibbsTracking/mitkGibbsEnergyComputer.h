#ifndef GIBBSENERGYCOMPUTER_H
#define GIBBSENERGYCOMPUTER_H

#include <MitkFiberTrackingExports.h>
#include <itkOrientationDistributionFunction.h>
#include <mitkParticleGrid.h>
#include <mitkSphereInterpolator.h>
#include <itkMersenneTwisterRandomVariateGenerator.h>

#include "mitkEnergyComputer.h"


using namespace mitk;

/**
* \brief ODF lookuptable based energy computer.   */

class MITKFIBERTRACKING_EXPORT GibbsEnergyComputer : public EnergyComputer
{
  public:

    typedef itk::Vector<float, ODF_SAMPLING_SIZE>   OdfVectorType;
    typedef itk::Image<OdfVectorType, 3>        ItkOdfImgType;
    typedef itk::Image<float, 3>                ItkFloatImageType;
    typedef itk::Statistics::MersenneTwisterRandomVariateGenerator ItkRandGenType;

    GibbsEnergyComputer(ItkOdfImgType* OdfImage, ItkFloatImageType* mask, ParticleGrid* particleGrid, SphereInterpolator* interpolator, ItkRandGenType* randGen);

    virtual ~GibbsEnergyComputer();

    // external energy calculation
    float ComputeExternalEnergy(vnl_vector_fixed<float, 3>& R, vnl_vector_fixed<float, 3>& N, Particle* dp) override;

    // internal energy calculation
    float ComputeInternalEnergyConnection(Particle *p1,int ep1) override;
    float ComputeInternalEnergyConnection(Particle *p1,int ep1, Particle *p2, int ep2) override;
    float ComputeInternalEnergy(Particle *dp) override;

    float EvaluateOdf(vnl_vector_fixed<float, 3>& pos, vnl_vector_fixed<float, 3> dir);
  protected:

    ItkOdfImgType*                m_Image;

};

#endif
