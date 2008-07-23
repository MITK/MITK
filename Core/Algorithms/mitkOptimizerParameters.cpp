/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: 9502 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkOptimizerParameters.h"


namespace mitk {

  OptimizerParameters::OptimizerParameters() : 
    m_Optimizer(EXHAUSTIVEOPTIMIZER),
    // for all Optimizer
    m_Dimension(2),
    m_Maximize(false),
    
    // for itk::ExhaustiveOptimizer
    m_StepLengthExhaustive(1.0),
    m_NumberOfStepsExhaustive(10),
    
    // for itk::GradientDescentOptimizer
    m_LearningRateGradientDescent(0.01),
    m_NumberOfIterationsGradientDescent(200),

    // for itk::QuaternionRigidTransformGradientDescentOptimizer
    m_LearningRateQuaternionRigidTransformGradientDescent(0.01),
    m_NumberOfIterationsQuaternionRigidTransformGradientDescent(200),

    // for itk::LBFGSBOptimizer

    // for itk::OnePlusOneEvolutionaryOptimizer
    m_GrowthFactorOnePlusOneEvolutionary(0.01),
    m_ShrinkFactorOnePlusOneEvolutionary(0.001),
    m_EpsilonOnePlusOneEvolutionary(2),
    m_InitialRadiusOnePlusOneEvolutionary(1),
    m_NumberOfIterationsOnePlusOneEvolutionary(200),

    // for itk::PowellOptimizer
    m_StepLengthPowell(0.01),
    m_StepTolerancePowell(0.001),
    m_ValueTolerancePowell(0.001),
    m_NumberOfIterationsPowell(200),

    // for itk::FRPROptimizer
    m_StepLengthFRPR(0.01),
    m_FletchReevesFRPR(true),
    m_PolakRibiereFRPR(false),
    m_NumberOfIterationsFRPR(200),

    // for itk::RegularStepGradientDescentOptimizer
    m_GradientMagnitudeToleranceRegularStepGradientDescent(0.0001),
    m_MinimumStepLengthRegularStepGradientDescent(0.001),
    m_MaximumStepLengthRegularStepGradientDescent(1.0),
    m_NumberOfIterationsRegularStepGradientDescent(100),
    m_RelaxationFactorRegularStepGradientDescent(0.5),

    // for itk::VersorRigid3DTransformOptimizer
    m_GradientMagnitudeToleranceVersorRigid3DTransform(0.0001),
    m_MinimumStepLengthVersorRigid3DTransform(0.0001),
    m_MaximumStepLengthVersorRigid3DTransform(4.0),
    m_NumberOfIterationsVersorRigid3DTransform(200),

    // for itk::VersorTransformOptimizer
    m_GradientMagnitudeToleranceVersorTransform(0.0001),
    m_MinimumStepLengthVersorTransform(0.0001),
    m_MaximumStepLengthVersorTransform(4.0),
    m_NumberOfIterationsVersorTransform(200),

    // for itk::AmoebaOptimizer
    m_ParametersConvergenceToleranceAmoeba(0.01),
    m_FunctionConvergenceToleranceAmoeba(0.001),
    m_NumberOfIterationsAmoeba(200),

    // for itk::ConjugateGradientOptimizer

    // for itk::LBFGSOptimizer
    m_GradientConvergenceToleranceLBFGS(0.01),
    m_LineSearchAccuracyLBFGS(0.01),
    m_DefaultStepLengthLBFGS(0.01),
    m_TraceOnLBFGS(true),
    m_NumberOfIterationsLBFGS(200),

    // for itk::SPSAOptimizer
    m_aSPSA(1.0),
    m_ASPSA(10.0),
    m_AlphaSPSA(0.602),
    m_cSPSA(0.0001),
    m_GammaSPSA(0.101),
    m_ToleranceSPSA(1e-5),
    m_StateOfConvergenceDecayRateSPSA(0.5),
    m_MinimumNumberOfIterationsSPSA(10),
    m_NumberOfPerturbationsSPSA(1),
    m_NumberOfIterationsSPSA(200)
  {
    // for itk::AmoebaOptimizer
    m_SimplexDeltaAmoeba.SetSize(16);
    m_SimplexDeltaAmoeba.Fill(5.0);
  }

  /********************************** for itk::AmoebaOptimizer ****************************************/
  
  void OptimizerParameters::SetSimplexDeltaAmoeba(itk::Array<double> simplexDelta)
  {
    m_SimplexDeltaAmoeba = simplexDelta;
  }

  itk::Array<double> OptimizerParameters::GetSimplexDeltaAmoeba()
  {
    return m_SimplexDeltaAmoeba;
  }

} // namespace mitk
