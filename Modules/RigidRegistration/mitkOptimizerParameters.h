/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 11215 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef MITKOPTIMIZERPARAMETERS_H
#define MITKOPTIMIZERPARAMETERS_H

#include <itkArray.h>
#include "MitkRigidRegistrationExports.h"
#include <itkObjectFactory.h>
#include "mitkCommon.h"

namespace mitk {

  /*!
  \brief This class is used to hold all optimizer parameters needed for a rigid registration process.  

  To use the rigid registration framework you have to create an instance of this class and fill it with the parameters
  belonging to the selected optimizer. To let the rigid registration work properly, this instance has to be given 
  to mitkImageRegistrationMethod before calling the update() method in mitkImageRegistrationMethod.

  Also instances of the classes mitkTransformParameters and mitkMetricParameters have to be set in mitkImageRegistrationMethod 
  before calling the update() method.

  \ingroup RigidRegistration

  \author Daniel Stein
  */
  class MITK_RIGIDREGISTRATION_EXPORT OptimizerParameters : public ::itk::Object
  {
  public:

    mitkClassMacro(OptimizerParameters,::itk::Object);
    itkNewMacro(Self);

    /**
    \brief Unique integer value for every optimizer.
    */
    enum OptimizerType {
      EXHAUSTIVEOPTIMIZER = 0,
      GRADIENTDESCENTOPTIMIZER = 1,
      QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER = 2,
      LBFGSBOPTIMIZER = 3,
      ONEPLUSONEEVOLUTIONARYOPTIMIZER = 4,
      POWELLOPTIMIZER = 5,
      FRPROPTIMIZER = 6,
      REGULARSTEPGRADIENTDESCENTOPTIMIZER = 7,
      VERSORTRANSFORMOPTIMIZER = 8,
      AMOEBAOPTIMIZER = 9,
      CONJUGATEGRADIENTOPTIMIZER = 10,
      LBFGSOPTIMIZER = 11,
      SPSAOPTIMIZER = 12,
      VERSORRIGID3DTRANSFORMOPTIMIZER = 13
    };

    /**
    \brief Sets the optimizer used for registration by its unique integer value.
    */
    itkSetMacro( Optimizer, int );
    /**
    \brief Returns the optimizer used for registration by its unique integer value.
    */
    itkGetMacro( Optimizer, int );

    /**
    \brief Sets the images dimension which have to be registered.
    */
    itkSetMacro( Dimension, unsigned int );
    
    /**
    \brief Returns the images dimension which have to be registered.
    */
    itkGetMacro( Dimension, unsigned int );

    /**
    \brief Sets whether the optimizer should maximize or minimize the metrics value.
    */
    itkSetMacro( Maximize, bool );
    
    /**
    \brief Returns whether the optimizer should maximize or minimize the metrics value.
    */
    itkGetMacro( Maximize, bool );

    /**
    \brief for itk::ExhaustiveOptimizer
    */
    itkSetMacro( StepLengthExhaustive, float );
    
    /**
    \brief for itk::ExhaustiveOptimizer
    */
    itkGetMacro( StepLengthExhaustive, float );

    /**
    \brief for itk::ExhaustiveOptimizer
    */
    itkSetMacro( NumberOfStepsExhaustive, int );
    
    /**
    \brief for itk::ExhaustiveOptimizer
    */
    itkGetMacro( NumberOfStepsExhaustive, int );

    /**
    \brief for itk::GradientDescentOptimizer
    */
    itkSetMacro( LearningRateGradientDescent, float );
    
    /**
    \brief for itk::GradientDescentOptimizer
    */
    itkGetMacro( LearningRateGradientDescent, float );

    /**
    \brief for itk::GradientDescentOptimizer
    */
    itkSetMacro( NumberOfIterationsGradientDescent, int );
    
    /**
    \brief for itk::GradientDescentOptimizer
    */
    itkGetMacro( NumberOfIterationsGradientDescent, int );

    /**
    \brief for itk::QuaternionRigidTransformGradientDescentOptimizer
    */
    itkSetMacro( LearningRateQuaternionRigidTransformGradientDescent, float );
    
    /**
    \brief for itk::QuaternionRigidTransformGradientDescentOptimizer
    */
    itkGetMacro( LearningRateQuaternionRigidTransformGradientDescent, float );

    /**
    \brief for itk::QuaternionRigidTransformGradientDescentOptimizer
    */
    itkSetMacro( NumberOfIterationsQuaternionRigidTransformGradientDescent, int );
    
    /**
    \brief for itk::QuaternionRigidTransformGradientDescentOptimizer
    */
    itkGetMacro( NumberOfIterationsQuaternionRigidTransformGradientDescent, int );

    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkSetMacro( GrowthFactorOnePlusOneEvolutionary, float );
    
    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkGetMacro( GrowthFactorOnePlusOneEvolutionary, float );

    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkSetMacro( ShrinkFactorOnePlusOneEvolutionary, float );
    
    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkGetMacro( ShrinkFactorOnePlusOneEvolutionary, float );

    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkSetMacro( EpsilonOnePlusOneEvolutionary, float );
    
    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkGetMacro( EpsilonOnePlusOneEvolutionary, float );

    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkSetMacro( InitialRadiusOnePlusOneEvolutionary, float );
    
    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkGetMacro( InitialRadiusOnePlusOneEvolutionary, float );

    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkSetMacro( NumberOfIterationsOnePlusOneEvolutionary, int );
    
    /**
    \brief for itk::OnePlusOneEvolutionaryOptimizer
    */
    itkGetMacro( NumberOfIterationsOnePlusOneEvolutionary, int );

    /**
    \brief for itk::PowellOptimizer
    */
    itkSetMacro( StepLengthPowell, float );
    
    /**
    \brief for itk::PowellOptimizer
    */
    itkGetMacro( StepLengthPowell, float );

    /**
    \brief for itk::PowellOptimizer
    */
    itkSetMacro( StepTolerancePowell, float );
    
    /**
    \brief for itk::PowellOptimizer
    */
    itkGetMacro( StepTolerancePowell, float );

    /**
    \brief for itk::PowellOptimizer
    */
    itkSetMacro( ValueTolerancePowell, float );
    
    /**
    \brief for itk::PowellOptimizer
    */
    itkGetMacro( ValueTolerancePowell, float );

    /**
    \brief for itk::PowellOptimizer
    */
    itkSetMacro( NumberOfIterationsPowell, int );
    
    /**
    \brief for itk::PowellOptimizer
    */
    itkGetMacro( NumberOfIterationsPowell, int );

    /**
    \brief for itk::FRPROptimizer
    */
    itkSetMacro( StepLengthFRPR, float );
    
    /**
    \brief for itk::FRPROptimizer
    */
    itkGetMacro( StepLengthFRPR, float );

    /**
    \brief for itk::FRPROptimizer
    */
    itkSetMacro( FletchReevesFRPR, bool );
    
    /**
    \brief for itk::FRPROptimizer
    */
    itkGetMacro( FletchReevesFRPR, bool );

    /**
    \brief for itk::FRPROptimizer
    */
    itkSetMacro( PolakRibiereFRPR, bool );
    
    /**
    \brief for itk::FRPROptimizer
    */
    itkGetMacro( PolakRibiereFRPR, bool );

    /**
    \brief for itk::FRPROptimizer
    */
    itkSetMacro( NumberOfIterationsFRPR, int );
    
    /**
    \brief for itk::FRPROptimizer
    */
    itkGetMacro( NumberOfIterationsFRPR, int );

    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkSetMacro( GradientMagnitudeToleranceRegularStepGradientDescent, float );
    
    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkGetMacro( GradientMagnitudeToleranceRegularStepGradientDescent, float );

    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkSetMacro( MinimumStepLengthRegularStepGradientDescent, float );
    
    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkGetMacro( MinimumStepLengthRegularStepGradientDescent, float );

    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkSetMacro( MaximumStepLengthRegularStepGradientDescent, float );
   
    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkGetMacro( MaximumStepLengthRegularStepGradientDescent, float );

    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkSetMacro( NumberOfIterationsRegularStepGradientDescent, int );
    
    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkGetMacro( NumberOfIterationsRegularStepGradientDescent, int );

    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkSetMacro( RelaxationFactorRegularStepGradientDescent, double );
    
    /**
    \brief for itk::RegularStepGradientDescentOptimizer
    */
    itkGetMacro( RelaxationFactorRegularStepGradientDescent, double );

    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkSetMacro( GradientMagnitudeToleranceVersorRigid3DTransform, float );
    
    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkGetMacro( GradientMagnitudeToleranceVersorRigid3DTransform, float );

    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkSetMacro( MinimumStepLengthVersorRigid3DTransform, float );
    
    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkGetMacro( MinimumStepLengthVersorRigid3DTransform, float );

    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkSetMacro( MaximumStepLengthVersorRigid3DTransform, float );
    
    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkGetMacro( MaximumStepLengthVersorRigid3DTransform, float );

    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkSetMacro( NumberOfIterationsVersorRigid3DTransform, int );
    
    /**
    \brief for itk::VersorRigid3DTransformOptimizer
    */
    itkGetMacro( NumberOfIterationsVersorRigid3DTransform, int );

    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkSetMacro( GradientMagnitudeToleranceVersorTransform, float );
    
    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkGetMacro( GradientMagnitudeToleranceVersorTransform, float );

    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkSetMacro( MinimumStepLengthVersorTransform, float );
    
    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkGetMacro( MinimumStepLengthVersorTransform, float );

    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkSetMacro( MaximumStepLengthVersorTransform, float );
    
    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkGetMacro( MaximumStepLengthVersorTransform, float );

    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkSetMacro( NumberOfIterationsVersorTransform, int );
    
    /**
    \brief for itk::VersorTransformOptimizer
    */
    itkGetMacro( NumberOfIterationsVersorTransform, int );

    /**
    \brief for itk::AmoebaOptimizer
    */
    void SetSimplexDeltaAmoeba(itk::Array<double> simplexDelta);
    
    /**
    \brief for itk::AmoebaOptimizer
    */
    itk::Array<double> GetSimplexDeltaAmoeba();

    /**
    \brief for itk::AmoebaOptimizer
    */
    itkSetMacro( ParametersConvergenceToleranceAmoeba, float );
    
    /**
    \brief for itk::AmoebaOptimizer
    */
    itkGetMacro( ParametersConvergenceToleranceAmoeba, float );

    /**
    \brief for itk::AmoebaOptimizer
    */
    itkSetMacro( FunctionConvergenceToleranceAmoeba, float );
    
    /**
    \brief for itk::AmoebaOptimizer
    */
    itkGetMacro( FunctionConvergenceToleranceAmoeba, float );

    /**
    \brief for itk::AmoebaOptimizer
    */
    itkSetMacro( NumberOfIterationsAmoeba, int );

    /**
    \brief for itk::AmoebaOptimizer
    */
    itkGetMacro( NumberOfIterationsAmoeba, int );

    // nothing for itk::ConjugateGradientOptimizer

    /**
    \brief for itk::LBFGSOptimizer
    */
    itkSetMacro( GradientConvergenceToleranceLBFGS, float );
    
    /**
    \brief for itk::LBFGSOptimizer
    */
    itkGetMacro( GradientConvergenceToleranceLBFGS, float );

    /**
    \brief for itk::LBFGSOptimizer
    */
    itkSetMacro( LineSearchAccuracyLBFGS, float );
    
    /**
    \brief for itk::LBFGSOptimizer
    */
    itkGetMacro( LineSearchAccuracyLBFGS, float );

    /**
    \brief for itk::LBFGSOptimizer
    */
    itkSetMacro( DefaultStepLengthLBFGS, float );
    
    /**
    \brief for itk::LBFGSOptimizer
    */
    itkGetMacro( DefaultStepLengthLBFGS, float );

    /**
    \brief for itk::LBFGSOptimizer
    */
    itkSetMacro( TraceOnLBFGS, bool );
    
    /**
    \brief for itk::LBFGSOptimizer
    */
    itkGetMacro( TraceOnLBFGS, bool );

    /**
    \brief for itk::LBFGSOptimizer
    */
    itkSetMacro( NumberOfIterationsLBFGS, int );
    
    /**
    \brief for itk::LBFGSOptimizer
    */
    itkGetMacro( NumberOfIterationsLBFGS, int );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( aSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( aSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( ASPSA, float );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( ASPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( AlphaSPSA, float );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( AlphaSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( cSPSA, float );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( cSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( GammaSPSA, float );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( GammaSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( ToleranceSPSA, float );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( ToleranceSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( StateOfConvergenceDecayRateSPSA, float );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( StateOfConvergenceDecayRateSPSA, float );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( MinimumNumberOfIterationsSPSA, int );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( MinimumNumberOfIterationsSPSA, int );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( NumberOfPerturbationsSPSA, int );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( NumberOfPerturbationsSPSA, int );

    /**
    \brief for itk::SPSAOptimizer
    */
    itkSetMacro( NumberOfIterationsSPSA, int );
    
    /**
    \brief for itk::SPSAOptimizer
    */
    itkGetMacro( NumberOfIterationsSPSA, int );

  protected:
    OptimizerParameters(); 
    ~OptimizerParameters(){};

    // for all Optimizer
    int m_Optimizer;
    unsigned int m_Dimension;
    bool m_Maximize;
    
    // for itk::ExhaustiveOptimizer
    float m_StepLengthExhaustive;
    int m_NumberOfStepsExhaustive;

    // for itk::GradientDescentOptimizer
    float m_LearningRateGradientDescent;
    int m_NumberOfIterationsGradientDescent;

    // for itk::QuaternionRigidTransformGradientDescentOptimizer
    float m_LearningRateQuaternionRigidTransformGradientDescent;
    int m_NumberOfIterationsQuaternionRigidTransformGradientDescent;

    // for itk::LBFGSBOptimizer

    // for itk::OnePlusOneEvolutionaryOptimizer
    float m_GrowthFactorOnePlusOneEvolutionary;
    float m_ShrinkFactorOnePlusOneEvolutionary;
    float m_EpsilonOnePlusOneEvolutionary;
    float m_InitialRadiusOnePlusOneEvolutionary;
    int m_NumberOfIterationsOnePlusOneEvolutionary;

    // for itk::PowellOptimizer
    float m_StepLengthPowell;
    float m_StepTolerancePowell;
    float m_ValueTolerancePowell;
    int m_NumberOfIterationsPowell;

    // for itk::FRPROptimizer
    float m_StepLengthFRPR;
    bool m_FletchReevesFRPR;
    bool m_PolakRibiereFRPR;
    int m_NumberOfIterationsFRPR;

    // for itk::RegularStepGradientDescentOptimizer
    float m_GradientMagnitudeToleranceRegularStepGradientDescent;
    float m_MinimumStepLengthRegularStepGradientDescent;
    float m_MaximumStepLengthRegularStepGradientDescent;
    int m_NumberOfIterationsRegularStepGradientDescent;
    double m_RelaxationFactorRegularStepGradientDescent;

    // for itk::VersorRigid3DTransformOptimizer
    float m_GradientMagnitudeToleranceVersorRigid3DTransform;
    float m_MinimumStepLengthVersorRigid3DTransform;
    float m_MaximumStepLengthVersorRigid3DTransform;
    int m_NumberOfIterationsVersorRigid3DTransform;
    
    // for itk::VersorTransformOptimizer
    float m_GradientMagnitudeToleranceVersorTransform;
    float m_MinimumStepLengthVersorTransform;
    float m_MaximumStepLengthVersorTransform;
    int m_NumberOfIterationsVersorTransform;

    // for itk::AmoebaOptimizer
    itk::Array<double> m_SimplexDeltaAmoeba;
    float m_ParametersConvergenceToleranceAmoeba;
    float m_FunctionConvergenceToleranceAmoeba;
    int m_NumberOfIterationsAmoeba;

    // for itk::ConjugateGradientOptimizer

    // for itk::LBFGSOptimizer
    float m_GradientConvergenceToleranceLBFGS;
    float m_LineSearchAccuracyLBFGS;
    float m_DefaultStepLengthLBFGS;
    bool m_TraceOnLBFGS;
    int m_NumberOfIterationsLBFGS;

    // for itk::SPSAOptimizer
    float m_aSPSA;
    float m_ASPSA;
    float m_AlphaSPSA;
    float m_cSPSA;
    float m_GammaSPSA;
    float m_ToleranceSPSA;
    float m_StateOfConvergenceDecayRateSPSA;
    int m_MinimumNumberOfIterationsSPSA;
    int m_NumberOfPerturbationsSPSA;
    int m_NumberOfIterationsSPSA;

  };

} // namespace mitk

#endif // MITKOPTIMIZERPARAMETERS_H
