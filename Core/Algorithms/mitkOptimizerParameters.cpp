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

  OptimizerParameters* OptimizerParameters::m_Instance = NULL;

  /**
   * Get the instance of this OptimizerParameters
   */
  OptimizerParameters* OptimizerParameters::GetInstance()
  {
    if (m_Instance == NULL)
    {
      m_Instance = new OptimizerParameters();
    }
    return m_Instance;
  }

  OptimizerParameters::OptimizerParameters()
  {
    // for all Optimizer
    m_Optimizer = EXHAUSTIVEOPTIMIZER;
    m_Dimension = 2;
    m_Maximize = false;
    
    // for itk::ExhaustiveOptimizer
    m_StepLengthExhaustive = 1.0;
    m_NumberOfStepsExhaustive = 10;
    
    // for itk::GradientDescentOptimizer
    m_LearningRateGradientDescent = 0.01;
    m_IterationsGradientDescent = 200;

    // for itk::QuaternionRigidTransformGradientDescentOptimizer
    m_LearningRateQuaternionRigidTransformGradientDescent = 0.01;
    m_IterationsQuaternionRigidTransformGradientDescent = 200;

    // for itk::LBFGSBOptimizer

    // for itk::OnePlusOneEvolutionaryOptimizer
    m_GrowthFactorOnePlusOneEvolutionary = 0.01;
    m_ShrinkFactorOnePlusOneEvolutionary = 0.001;
    m_EpsilonOnePlusOneEvolutionary = 2;
    m_InitialRadiusOnePlusOneEvolutionary = 1;
    m_IterationsOnePlusOneEvolutionary = 200;

    // for itk::PowellOptimizer
    m_StepLengthPowell = 0.01;
    m_StepTolerancePowell = 0.001;
    m_ValueTolerancePowell = 0.001;
    m_IterationsPowell = 200;

    // for itk::FRPROptimizer
    m_StepLengthFRPR = 0.01;
    m_FletchReevesFRPR = true;
    m_PolakRibiereFRPR = false;
    m_IterationsFRPR = 200;

    // for itk::RegularStepGradientDescentOptimizer
    m_GradientMagnitudeToleranceRegularStepGradientDescent = 0.0001;
    m_MinimumStepLengthRegularStepGradientDescent = 0.001;
    m_MaximumStepLengthRegularStepGradientDescent = 1.0;
    m_IterationsRegularStepGradientDescent = 100;
    m_RelaxationFactorRegularStepGradientDescent = 0.5;

    // for itk::VersorRigid3DTransformOptimizer
    m_GradientMagnitudeToleranceVersorRigid3DTransform = 0.0001;
    m_MinimumStepLengthVersorRigid3DTransform = 0.0001;
    m_MaximumStepLengthVersorRigid3DTransform = 4.0;
    m_IterationsVersorRigid3DTransform = 200;

    // for itk::VersorTransformOptimizer
    m_GradientMagnitudeToleranceVersorTransform = 0.0001;
    m_MinimumStepLengthVersorTransform = 0.0001;
    m_MaximumStepLengthVersorTransform = 4.0;
    m_IterationsVersorTransform = 200;

    // for itk::AmoebaOptimizer
    m_SimplexDeltaAmoeba.SetSize(16);
    m_SimplexDeltaAmoeba.Fill(5.0);
    m_ParametersConvergenceToleranceAmoeba = 0.01;
    m_FunctionConvergenceToleranceAmoeba = 0.001;
    m_IterationsAmoeba = 200;

    // for itk::ConjugateGradientOptimizer

    // for itk::LBFGSOptimizer
    m_GradientConvergenceToleranceLBFGS = 0.01;
    m_LineSearchAccuracyLBFGS = 0.01;
    m_DefaultStepLengthLBFGS = 0.01;
    m_TraceOnLBFGS = true;
    m_IterationsLBFGS = 200;

    // for itk::SPSAOptimizer
    m_aSPSA = 1.0;
    m_ASPSA = 10.0;
    m_AlphaSPSA = 0.602;
    m_cSPSA = 0.0001;
    m_GammaSPSA = 0.101;
    m_ToleranceSPSA = 1e-5;
    m_StateOfConvergenceDecayRateSPSA = 0.5;
    m_MinimumNumberOfIterationsSPSA = 10;
    m_NumberOfPerturbationsSPSA = 1;
    m_IterationsSPSA = 200;
  }

  OptimizerParameters::~OptimizerParameters()
  {
  }

  /********************************** for all Optimizer ****************************************/

  void OptimizerParameters::SetOptimizer(int optimizer)
  {
	  m_Optimizer = optimizer;
  }

  int OptimizerParameters::GetOptimizer()
  {
	  return m_Optimizer;
  }

  void OptimizerParameters::SetDimension(unsigned int dimension)
  {
	  m_Dimension = dimension;
  }

  unsigned int OptimizerParameters::GetDimension()
  {
	  return m_Dimension;
  }
  
  void OptimizerParameters::SetMaximize(bool maximize)
  {
	  m_Maximize = maximize;
  }

  bool OptimizerParameters::GetMaximize()
  {
	  return m_Maximize;
  }

  /********************************** for itk::ExhaustiveOptimizer ****************************************/

  void OptimizerParameters::SetStepLengthExhaustive(float stepLength)
  {
    m_StepLengthExhaustive = stepLength;
  }

  float OptimizerParameters::GetStepLengthExhaustive()
  {
    return m_StepLengthExhaustive;
  }

  void OptimizerParameters::SetNumberOfStepsExhaustive(int numberOfSteps)
  {
    m_NumberOfStepsExhaustive = numberOfSteps;
  }

  int OptimizerParameters::GetNumberOfStepsExhaustive()
  {
    return m_NumberOfStepsExhaustive;
  }

  /********************************** for itk::GradientDescentOptimizer ****************************************/

  void OptimizerParameters::SetLearningRateGradientDescent(float learningRate)
  {
	  m_LearningRateGradientDescent = learningRate;
  }

  float OptimizerParameters::GetLearningRateGradientDescent()
  {
	  return m_LearningRateGradientDescent;
  }

  void OptimizerParameters::SetNumberOfIterationsGradientDescent(int iterations)
  {
	  m_IterationsGradientDescent = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsGradientDescent()
  {
	  return m_IterationsGradientDescent;
  }

  /********************************** for itk::QuaternionRigidTransformGradientDescentOptimizer ****************/

  void OptimizerParameters::SetLearningRateQuaternionRigidTransformGradientDescent(float learningRate)
  {
	  m_LearningRateQuaternionRigidTransformGradientDescent = learningRate;
  }

  float OptimizerParameters::GetLearningRateQuaternionRigidTransformGradientDescent()
  {
	  return m_LearningRateQuaternionRigidTransformGradientDescent;
  }

  void OptimizerParameters::SetNumberOfIterationsQuaternionRigidTransformGradientDescent(int iterations)
  {
	  m_IterationsQuaternionRigidTransformGradientDescent = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsQuaternionRigidTransformGradientDescent()
  {
	  return m_IterationsQuaternionRigidTransformGradientDescent;
  }

  /********************************** for itk::LBFGSBOptimizer *************************************************/


  /********************************** for itk::OnePlusOneEvolutionaryOptimizer *********************************/
  
  void OptimizerParameters::SetGrowthFactorOnePlusOneEvolutionary(float growthFactor)
  {
	  m_GrowthFactorOnePlusOneEvolutionary = growthFactor;
  }

  float OptimizerParameters::GetGrowthFactorOnePlusOneEvolutionary()
  {
	  return m_GrowthFactorOnePlusOneEvolutionary;
  }

  void OptimizerParameters::SetShrinkFactorOnePlusOneEvolutionary(float shrinkFactor)
  {
	  m_ShrinkFactorOnePlusOneEvolutionary = shrinkFactor;
  }

  float OptimizerParameters::GetShrinkFactorOnePlusOneEvolutionary()
  {
	  return m_ShrinkFactorOnePlusOneEvolutionary;
  }

  void OptimizerParameters::SetEpsilonOnePlusOneEvolutionary(float epsilon)
  {
	  m_EpsilonOnePlusOneEvolutionary = epsilon;
  }

  float OptimizerParameters::GetEpsilonOnePlusOneEvolutionary()
  {
	  return m_EpsilonOnePlusOneEvolutionary;
  }

  void OptimizerParameters::SetInitialRadiusOnePlusOneEvolutionary(float initialRadius)
  {
	  m_InitialRadiusOnePlusOneEvolutionary = initialRadius;
  }

  float OptimizerParameters::GetInitialRadiusOnePlusOneEvolutionary()
  {
	  return m_InitialRadiusOnePlusOneEvolutionary;
  }

  void OptimizerParameters::SetNumberOfIterationsOnePlusOneEvolutionary(int iterations)
  {
	  m_IterationsOnePlusOneEvolutionary = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsOnePlusOneEvolutionary()
  {
	  return m_IterationsOnePlusOneEvolutionary;
  }

  /********************************** for itk::PowellOptimizer *********************************/
  
  void OptimizerParameters::SetStepLengthPowell(float stepLength)
  {
	  m_StepLengthPowell = stepLength;
  }

  float OptimizerParameters::GetStepLengthPowell()
  {
	  return m_StepLengthPowell;
  }

  void OptimizerParameters::SetStepTolerancePowell(float stepTolerance)
  {
	  m_StepTolerancePowell = stepTolerance;
  }

  float OptimizerParameters::GetStepTolerancePowell()
  {
	  return m_StepTolerancePowell;
  }

  void OptimizerParameters::SetValueTolerancePowell(float valueTolerance)
  {
	  m_ValueTolerancePowell = valueTolerance;
  }

  float OptimizerParameters::GetValueTolerancePowell()
  {
	  return m_ValueTolerancePowell;
  }

  void OptimizerParameters::SetNumberOfIterationsPowell(int iterations)
  {
	  m_IterationsPowell = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsPowell()
  {
	  return m_IterationsPowell;
  }
  
  /********************************** for itk::FRPROptimizer *********************************/

  void OptimizerParameters::SetStepLengthFRPR(float stepLength)
  {
	  m_StepLengthFRPR = stepLength;
  }

  float OptimizerParameters::GetStepLengthFRPR()
  {
	  return m_StepLengthFRPR;
  }

  void OptimizerParameters::SetToFletchReevesFRPR(bool fletchReeves)
  {
	  m_FletchReevesFRPR = fletchReeves;
    m_PolakRibiereFRPR = !fletchReeves;
  }

  bool OptimizerParameters::GetFletchReevesFRPR()
  {
	  return m_FletchReevesFRPR;
  }

  void OptimizerParameters::SetToPolakRibiereFRPR(bool polakRibiere)
  {
	  m_PolakRibiereFRPR = polakRibiere;
    m_FletchReevesFRPR = !polakRibiere;
  }

  bool OptimizerParameters::GetPolakRibiereFRPR()
  {
	  return m_PolakRibiereFRPR;
  }

  void OptimizerParameters::SetNumberOfIterationsFRPR(int iterations)
  {
	  m_IterationsFRPR = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsFRPR()
  {
	  return m_IterationsFRPR;
  }

  /********************************** for itk::RegularStepGradientDescentOptimizer *********************************/

  void OptimizerParameters::SetGradientMagnitudeToleranceRegularStepGradientDescent(float gradientMagnitudeTolerance)
  {
	  m_GradientMagnitudeToleranceRegularStepGradientDescent = gradientMagnitudeTolerance;
  }

  float OptimizerParameters::GetGradientMagnitudeToleranceRegularStepGradientDescent()
  {
	  return m_GradientMagnitudeToleranceRegularStepGradientDescent;
  }

  void OptimizerParameters::SetMinimumStepLengthRegularStepGradientDescent(float minimumStepLength)
  {
	  m_MinimumStepLengthRegularStepGradientDescent = minimumStepLength;
  }

  float OptimizerParameters::GetMinimumStepLengthRegularStepGradientDescent()
  {
	  return m_MinimumStepLengthRegularStepGradientDescent;
  }

  void OptimizerParameters::SetMaximumStepLengthRegularStepGradientDescent(float maximumStepLength)
  {
	  m_MaximumStepLengthRegularStepGradientDescent = maximumStepLength;
  }

  float OptimizerParameters::GetMaximumStepLengthRegularStepGradientDescent()
  {
	  return m_MaximumStepLengthRegularStepGradientDescent;
  }

  void OptimizerParameters::SetNumberOfIterationsRegularStepGradientDescent(int iterations)
  {
	  m_IterationsRegularStepGradientDescent = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsRegularStepGradientDescent()
  {
	  return m_IterationsRegularStepGradientDescent;
  }

  void OptimizerParameters::SetRelaxationFactorRegularStepGradientDescent(double relaxationFactor)
  {
	  m_RelaxationFactorRegularStepGradientDescent = relaxationFactor;
  }

  double OptimizerParameters::GetRelaxationFactorRegularStepGradientDescent()
  {
	  return m_RelaxationFactorRegularStepGradientDescent;
  }

  /********************************** for itk::VersorTransformOptimizer *********************************/
  
  void OptimizerParameters::SetGradientMagnitudeToleranceVersorRigid3DTransform(float gradientMagnitudeTolerance)
  {
	  m_GradientMagnitudeToleranceVersorRigid3DTransform = gradientMagnitudeTolerance;
  }

  float OptimizerParameters::GetGradientMagnitudeToleranceVersorRigid3DTransform()
  {
	  return m_GradientMagnitudeToleranceVersorRigid3DTransform;
  }

  void OptimizerParameters::SetMinimumStepLengthVersorRigid3DTransform(float minimumStepLength)
  {
	  m_MinimumStepLengthVersorRigid3DTransform = minimumStepLength;
  }

  float OptimizerParameters::GetMinimumStepLengthVersorRigid3DTransform()
  {
	  return m_MinimumStepLengthVersorRigid3DTransform;
  }

  void OptimizerParameters::SetMaximumStepLengthVersorRigid3DTransform(float maximumStepLength)
  {
	  m_MaximumStepLengthVersorRigid3DTransform = maximumStepLength;
  }

  float OptimizerParameters::GetMaximumStepLengthVersorRigid3DTransform()
  {
	  return m_MaximumStepLengthVersorRigid3DTransform;
  }

  void OptimizerParameters::SetNumberOfIterationsVersorRigid3DTransform(int iterations)
  {
	  m_IterationsVersorRigid3DTransform = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsVersorRigid3DTransform()
  {
	  return m_IterationsVersorRigid3DTransform;
  }

  /********************************** for itk::VersorTransformOptimizer *********************************/
  
  void OptimizerParameters::SetGradientMagnitudeToleranceVersorTransform(float gradientMagnitudeTolerance)
  {
	  m_GradientMagnitudeToleranceVersorTransform = gradientMagnitudeTolerance;
  }

  float OptimizerParameters::GetGradientMagnitudeToleranceVersorTransform()
  {
	  return m_GradientMagnitudeToleranceVersorTransform;
  }

  void OptimizerParameters::SetMinimumStepLengthVersorTransform(float minimumStepLength)
  {
	  m_MinimumStepLengthVersorTransform = minimumStepLength;
  }

  float OptimizerParameters::GetMinimumStepLengthVersorTransform()
  {
	  return m_MinimumStepLengthVersorTransform;
  }

  void OptimizerParameters::SetMaximumStepLengthVersorTransform(float maximumStepLength)
  {
	  m_MaximumStepLengthVersorTransform = maximumStepLength;
  }

  float OptimizerParameters::GetMaximumStepLengthVersorTransform()
  {
	  return m_MaximumStepLengthVersorTransform;
  }

  void OptimizerParameters::SetNumberOfIterationsVersorTransform(int iterations)
  {
	  m_IterationsVersorTransform = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsVersorTransform()
  {
	  return m_IterationsVersorTransform;
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

  void OptimizerParameters::SetParametersConvergenceToleranceAmoeba(float parametersConvergenceTolerance)
  {
	  m_ParametersConvergenceToleranceAmoeba = parametersConvergenceTolerance;
  }

  float OptimizerParameters::GetParametersConvergenceToleranceAmoeba()
  {
	  return m_ParametersConvergenceToleranceAmoeba;
  }

  void OptimizerParameters::SetFunctionConvergenceToleranceAmoeba(float functionConvergenceTolerance)
  {
	  m_FunctionConvergenceToleranceAmoeba = functionConvergenceTolerance;
  }

  float OptimizerParameters::GetFunctionConvergenceToleranceAmoeba()
  {
	  return m_FunctionConvergenceToleranceAmoeba;
  }

  void OptimizerParameters::SetNumberOfIterationsAmoeba(int iterations)
  {
	  m_IterationsAmoeba = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsAmoeba()
  {
	  return m_IterationsAmoeba;
  }

  /********************************** for itk::ConjugateGradientOptimizer ****************************************/


  /********************************** for itk::LBFGSOptimizer ****************************************************/
  
  void OptimizerParameters::SetGradientConvergenceToleranceLBFGS(float gradientConvergenceTolerance)
  {
	  m_GradientConvergenceToleranceLBFGS = gradientConvergenceTolerance;
  }

  float OptimizerParameters::GetGradientConvergenceToleranceLBFGS()
  {
	  return m_GradientConvergenceToleranceLBFGS;
  }

  void OptimizerParameters::SetLineSearchAccuracyLBFGS(float lineSearchAccuracy)
  {
	  m_LineSearchAccuracyLBFGS = lineSearchAccuracy;
  }

  float OptimizerParameters::GetLineSearchAccuracyLBFGS()
  {
	  return m_LineSearchAccuracyLBFGS;
  }

  void OptimizerParameters::SetDefaultStepLengthLBFGS(float defaultStepLength)
  {
	  m_DefaultStepLengthLBFGS = defaultStepLength;
  }

  float OptimizerParameters::GetDefaultStepLengthLBFGS()
  {
	  return m_DefaultStepLengthLBFGS;
  }

  void OptimizerParameters::SetTraceOnLBFGS(bool traceOn)
  {
	  m_TraceOnLBFGS = traceOn;
  }

  bool OptimizerParameters::GetTraceOnLBFGS()
  {
	  return m_TraceOnLBFGS;
  }

  void OptimizerParameters::SetNumberOfIterationsLBFGS(int iterations)
  {
	  m_IterationsLBFGS = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsLBFGS()
  {
	  return m_IterationsLBFGS;
  }

  /********************************** for itk::SPSAOptimizer ****************************************************/
  
  void OptimizerParameters::SetaSPSA(float a)
  {
	  m_aSPSA = a;
  }

  float OptimizerParameters::GetaSPSA()
  {
	  return m_aSPSA;
  }

  void OptimizerParameters::SetASPSA(float A)
  {
	  m_ASPSA = A;
  }

  float OptimizerParameters::GetASPSA()
  {
	  return m_ASPSA;
  }

  void OptimizerParameters::SetAlphaSPSA(float alpha)
  {
	  m_AlphaSPSA = alpha;
  }

  float OptimizerParameters::GetAlphaSPSA()
  {
	  return m_AlphaSPSA;
  }

  void OptimizerParameters::SetcSPSA(float c)
  {
	  m_cSPSA = c;
  }

  float OptimizerParameters::GetcSPSA()
  {
	  return m_cSPSA;
  }

  void OptimizerParameters::SetGammaSPSA(float gamma)
  {
	  m_GammaSPSA = gamma;
  }

  float OptimizerParameters::GetGammaSPSA()
  {
	  return m_GammaSPSA;
  }

  void OptimizerParameters::SetToleranceSPSA(float tolerance)
  {
	  m_ToleranceSPSA = tolerance;
  }

  float OptimizerParameters::GetToleranceSPSA()
  {
	  return m_ToleranceSPSA;
  }

  void OptimizerParameters::SetStateOfConvergenceDecayRateSPSA(float stateOfConvergenceDecayRate)
  {
	  m_StateOfConvergenceDecayRateSPSA = stateOfConvergenceDecayRate;
  }

  float OptimizerParameters::GetStateOfConvergenceDecayRateSPSA()
  {
	  return m_StateOfConvergenceDecayRateSPSA;
  }

  void OptimizerParameters::SetMinimumNumberOfIterationsSPSA(int minimumNumberOfIterations)
  {
	  m_MinimumNumberOfIterationsSPSA = minimumNumberOfIterations;
  }

  int OptimizerParameters::GetMinimumNumberOfIterationsSPSA()
  {
	  return m_MinimumNumberOfIterationsSPSA;
  }

  void OptimizerParameters::SetNumberOfPerturbationsSPSA(int numberOfPerturbations)
  {
	  m_NumberOfPerturbationsSPSA = numberOfPerturbations;
  }

  int OptimizerParameters::GetNumberOfPerturbationsSPSA()
  {
	  return m_NumberOfPerturbationsSPSA;
  }

  void OptimizerParameters::SetNumberOfIterationsSPSA(int iterations)
  {
	  m_IterationsSPSA = iterations;
  }

  int OptimizerParameters::GetNumberOfIterationsSPSA()
  {
	  return m_IterationsSPSA;
  }

} // namespace mitk
