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
#include <itkObjectFactory.h>
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT OptimizerParameters : public ::itk::Object
  {
  public:

    mitkClassMacro(OptimizerParameters,::itk::Object);
    itkNewMacro(Self);

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

    // for all Optimizer
    itkSetMacro( Optimizer, int );
    itkGetMacro( Optimizer, int );

    itkSetMacro( Dimension, int );
    itkGetMacro( Dimension, int );

    itkSetMacro( Maximize, bool );
    itkGetMacro( Maximize, bool );

    // for itk::ExhaustiveOptimizer
    itkSetMacro( StepLengthExhaustive, float );
    itkGetMacro( StepLengthExhaustive, float );

    itkSetMacro( NumberOfStepsExhaustive, int );
    itkGetMacro( NumberOfStepsExhaustive, int );

    // for itk::GradientDescentOptimizer
    itkSetMacro( LearningRateGradientDescent, float );
    itkGetMacro( LearningRateGradientDescent, float );

    itkSetMacro( NumberOfIterationsGradientDescent, int );
    itkGetMacro( NumberOfIterationsGradientDescent, int );

    // for itk::QuaternionRigidTransformGradientDescentOptimizer
    itkSetMacro( LearningRateQuaternionRigidTransformGradientDescent, float );
    itkGetMacro( LearningRateQuaternionRigidTransformGradientDescent, float );

    itkSetMacro( NumberOfIterationsQuaternionRigidTransformGradientDescent, int );
    itkGetMacro( NumberOfIterationsQuaternionRigidTransformGradientDescent, int );

    // for itk::OnePlusOneEvolutionaryOptimizer
    itkSetMacro( GrowthFactorOnePlusOneEvolutionary, float );
    itkGetMacro( GrowthFactorOnePlusOneEvolutionary, float );

    itkSetMacro( ShrinkFactorOnePlusOneEvolutionary, float );
    itkGetMacro( ShrinkFactorOnePlusOneEvolutionary, float );

    itkSetMacro( EpsilonOnePlusOneEvolutionary, float );
    itkGetMacro( EpsilonOnePlusOneEvolutionary, float );

    itkSetMacro( InitialRadiusOnePlusOneEvolutionary, float );
    itkGetMacro( InitialRadiusOnePlusOneEvolutionary, float );

    itkSetMacro( NumberOfIterationsOnePlusOneEvolutionary, int );
    itkGetMacro( NumberOfIterationsOnePlusOneEvolutionary, int );

    // for itk::PowellOptimizer
    itkSetMacro( StepLengthPowell, float );
    itkGetMacro( StepLengthPowell, float );

    itkSetMacro( StepTolerancePowell, float );
    itkGetMacro( StepTolerancePowell, float );

    itkSetMacro( ValueTolerancePowell, float );
    itkGetMacro( ValueTolerancePowell, float );

    itkSetMacro( NumberOfIterationsPowell, int );
    itkGetMacro( NumberOfIterationsPowell, int );

    // for itk::FRPROptimizer
    itkSetMacro( StepLengthFRPR, float );
    itkGetMacro( StepLengthFRPR, float );

    itkSetMacro( FletchReevesFRPR, bool );
    itkGetMacro( FletchReevesFRPR, bool );

    itkSetMacro( PolakRibiereFRPR, bool );
    itkGetMacro( PolakRibiereFRPR, bool );

    itkSetMacro( NumberOfIterationsFRPR, int );
    itkGetMacro( NumberOfIterationsFRPR, int );

    // for itk::RegularStepGradientDescentOptimizer
    itkSetMacro( GradientMagnitudeToleranceRegularStepGradientDescent, float );
    itkGetMacro( GradientMagnitudeToleranceRegularStepGradientDescent, float );

    itkSetMacro( MinimumStepLengthRegularStepGradientDescent, float );
    itkGetMacro( MinimumStepLengthRegularStepGradientDescent, float );

    itkSetMacro( MaximumStepLengthRegularStepGradientDescent, float );
    itkGetMacro( MaximumStepLengthRegularStepGradientDescent, float );

    itkSetMacro( NumberOfIterationsRegularStepGradientDescent, int );
    itkGetMacro( NumberOfIterationsRegularStepGradientDescent, int );

    itkSetMacro( RelaxationFactorRegularStepGradientDescent, double );
    itkGetMacro( RelaxationFactorRegularStepGradientDescent, double );

    // for itk::VersorRigid3DTransformOptimizer
    itkSetMacro( GradientMagnitudeToleranceVersorRigid3DTransform, float );
    itkGetMacro( GradientMagnitudeToleranceVersorRigid3DTransform, float );

    itkSetMacro( MinimumStepLengthVersorRigid3DTransform, float );
    itkGetMacro( MinimumStepLengthVersorRigid3DTransform, float );

    itkSetMacro( MaximumStepLengthVersorRigid3DTransform, float );
    itkGetMacro( MaximumStepLengthVersorRigid3DTransform, float );

    itkSetMacro( NumberOfIterationsVersorRigid3DTransform, int );
    itkGetMacro( NumberOfIterationsVersorRigid3DTransform, int );

    // for itk::VersorTransformOptimizer
    itkSetMacro( GradientMagnitudeToleranceVersorTransform, float );
    itkGetMacro( GradientMagnitudeToleranceVersorTransform, float );

    itkSetMacro( MinimumStepLengthVersorTransform, float );
    itkGetMacro( MinimumStepLengthVersorTransform, float );

    itkSetMacro( MaximumStepLengthVersorTransform, float );
    itkGetMacro( MaximumStepLengthVersorTransform, float );

    itkSetMacro( NumberOfIterationsVersorTransform, int );
    itkGetMacro( NumberOfIterationsVersorTransform, int );

    // for itk::AmoebaOptimizer
    void SetSimplexDeltaAmoeba(itk::Array<double> simplexDelta);
    itk::Array<double> GetSimplexDeltaAmoeba();

    itkSetMacro( ParametersConvergenceToleranceAmoeba, float );
    itkGetMacro( ParametersConvergenceToleranceAmoeba, float );

    itkSetMacro( FunctionConvergenceToleranceAmoeba, float );
    itkGetMacro( FunctionConvergenceToleranceAmoeba, float );

    itkSetMacro( NumberOfIterationsAmoeba, int );
    itkGetMacro( NumberOfIterationsAmoeba, int );

    // for itk::ConjugateGradientOptimizer

    // for itk::LBFGSOptimizer
    itkSetMacro( GradientConvergenceToleranceLBFGS, float );
    itkGetMacro( GradientConvergenceToleranceLBFGS, float );

    itkSetMacro( LineSearchAccuracyLBFGS, float );
    itkGetMacro( LineSearchAccuracyLBFGS, float );

    itkSetMacro( DefaultStepLengthLBFGS, float );
    itkGetMacro( DefaultStepLengthLBFGS, float );

    itkSetMacro( TraceOnLBFGS, bool );
    itkGetMacro( TraceOnLBFGS, bool );

    itkSetMacro( NumberOfIterationsLBFGS, int );
    itkGetMacro( NumberOfIterationsLBFGS, int );

    // for itk::SPSAOptimizer
    itkSetMacro( aSPSA, float );
    itkGetMacro( aSPSA, float );

    itkSetMacro( ASPSA, float );
    itkGetMacro( ASPSA, float );

    itkSetMacro( AlphaSPSA, float );
    itkGetMacro( AlphaSPSA, float );

    itkSetMacro( cSPSA, float );
    itkGetMacro( cSPSA, float );

    itkSetMacro( GammaSPSA, float );
    itkGetMacro( GammaSPSA, float );

    itkSetMacro( ToleranceSPSA, float );
    itkGetMacro( ToleranceSPSA, float );

    itkSetMacro( StateOfConvergenceDecayRateSPSA, float );
    itkGetMacro( StateOfConvergenceDecayRateSPSA, float );

    itkSetMacro( MinimumNumberOfIterationsSPSA, int );
    itkGetMacro( MinimumNumberOfIterationsSPSA, int );

    itkSetMacro( NumberOfPerturbationsSPSA, int );
    itkGetMacro( NumberOfPerturbationsSPSA, int );

    itkSetMacro( NumberOfIterationsSPSA, int );
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
