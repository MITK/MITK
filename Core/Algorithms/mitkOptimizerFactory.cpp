/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkOptimizerFactory.h"
#include "mitkOptimizerParameters.h"

#include <itkExhaustiveOptimizer.h>
#include <itkGradientDescentOptimizer.h>
#include <itkQuaternionRigidTransformGradientDescentOptimizer.h>
#include <itkLBFGSBOptimizer.h>
#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkNormalVariateGenerator.h>
#include <itkPowellOptimizer.h>
#include <itkFRPROptimizer.h>
#include <itkRegularStepGradientDescentOptimizer.h>
#include <itkVersorRigid3DTransformOptimizer.h>
#include <itkVersorTransformOptimizer.h>
#include <itkAmoebaOptimizer.h>
#include <itkConjugateGradientOptimizer.h>
#include <itkLBFGSOptimizer.h>
#include <itkSPSAOptimizer.h>

namespace mitk {

  OptimizerFactory::OptimizerFactory() : m_OptimizerParameters(NULL), m_NumberTransformParameters(16)
  {
  }

  OptimizerFactory::~OptimizerFactory()
  {
  }

  void OptimizerFactory::SetNumberOfTransformParameters(int numberTransformParameters)
  {
    m_NumberTransformParameters = numberTransformParameters;
  }

  OptimizerFactory::OptimizerType::Pointer OptimizerFactory::GetOptimizer( )
  {
    if( m_OptimizerParameters.IsNull() ) 
    {
      std::cerr << "No parameters set! Returning\n";
      return NULL;
    }
    int optimizer = m_OptimizerParameters->GetOptimizer();
    if (optimizer == OptimizerParameters::EXHAUSTIVEOPTIMIZER)
    {
      itk::ExhaustiveOptimizer::Pointer OptimizerPointer = itk::ExhaustiveOptimizer::New();
      OptimizerPointer->SetStepLength( m_OptimizerParameters->GetStepLengthExhaustive() );
      itk::ExhaustiveOptimizer::StepsType steps( m_NumberTransformParameters );
      for (int i = 0; i < m_NumberTransformParameters; i++)
      {
        steps[i] = m_OptimizerParameters->GetNumberOfStepsExhaustive();
      }
      
      OptimizerPointer->SetNumberOfSteps( steps );

      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::GRADIENTDESCENTOPTIMIZER)
    {
      itk::GradientDescentOptimizer::Pointer OptimizerPointer = itk::GradientDescentOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize());
      OptimizerPointer->SetLearningRate(m_OptimizerParameters->GetLearningRateGradientDescent());
      OptimizerPointer->SetNumberOfIterations( m_OptimizerParameters->GetNumberOfIterationsGradientDescent() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER)
    {
      itk::QuaternionRigidTransformGradientDescentOptimizer::Pointer OptimizerPointer = itk::QuaternionRigidTransformGradientDescentOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize());  
      OptimizerPointer->SetLearningRate(m_OptimizerParameters->GetLearningRateQuaternionRigidTransformGradientDescent());
      OptimizerPointer->SetNumberOfIterations( m_OptimizerParameters->GetNumberOfIterationsQuaternionRigidTransformGradientDescent() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::LBFGSBOPTIMIZER)
    {
      itk::LBFGSBOptimizer::Pointer OptimizerPointer = itk::LBFGSBOptimizer::New();
      // Set up boundary conditions
      itk::LBFGSBOptimizer::BoundValueType lower(12);
      itk::LBFGSBOptimizer::BoundValueType upper(12);
      itk::LBFGSBOptimizer::BoundSelectionType select(12);

      lower.Fill( -1 );
      upper.Fill( 10 );
      select.Fill( 2 );

      OptimizerPointer->SetLowerBound( lower );
      OptimizerPointer->SetUpperBound( upper );
      OptimizerPointer->SetBoundSelection( select );
      OptimizerPointer->SetCostFunctionConvergenceFactor(1e+1);
      OptimizerPointer->SetMaximumNumberOfCorrections(5);
      OptimizerPointer->SetProjectedGradientTolerance(1e-5);
      OptimizerPointer->SetMaximumNumberOfEvaluations(500);
      OptimizerPointer->SetMaximumNumberOfIterations( 200 );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::ONEPLUSONEEVOLUTIONARYOPTIMIZER)
    {
      itk::OnePlusOneEvolutionaryOptimizer::Pointer OptimizerPointer = itk::OnePlusOneEvolutionaryOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      itk::Statistics::NormalVariateGenerator::Pointer generator = itk::Statistics::NormalVariateGenerator::New();
      generator->Initialize(12345);
      OptimizerPointer->SetNormalVariateGenerator(generator);
      OptimizerPointer->SetGrowthFactor(m_OptimizerParameters->GetGrowthFactorOnePlusOneEvolutionary());
      OptimizerPointer->SetShrinkFactor(m_OptimizerParameters->GetShrinkFactorOnePlusOneEvolutionary());
      OptimizerPointer->SetEpsilon(m_OptimizerParameters->GetEpsilonOnePlusOneEvolutionary());
      OptimizerPointer->SetInitialRadius(m_OptimizerParameters->GetInitialRadiusOnePlusOneEvolutionary());
      OptimizerPointer->SetMaximumIteration(m_OptimizerParameters->GetNumberOfIterationsOnePlusOneEvolutionary());
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::POWELLOPTIMIZER)
    {
      itk::PowellOptimizer::Pointer OptimizerPointer = itk::PowellOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetStepLength( m_OptimizerParameters->GetStepLengthPowell() );
      OptimizerPointer->SetStepTolerance( m_OptimizerParameters->GetStepTolerancePowell() );
      OptimizerPointer->SetValueTolerance( m_OptimizerParameters->GetValueTolerancePowell() );
      OptimizerPointer->SetMaximumIteration( m_OptimizerParameters->GetNumberOfIterationsPowell() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::FRPROPTIMIZER)
    {
      itk::FRPROptimizer::Pointer OptimizerPointer = itk::FRPROptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetStepLength(m_OptimizerParameters->GetStepLengthFRPR());
      if (m_OptimizerParameters->GetFletchReevesFRPR())
      {
        OptimizerPointer->SetToFletchReeves();
      }
      else if (m_OptimizerParameters->GetPolakRibiereFRPR())
      {
        OptimizerPointer->SetToPolakRibiere();
      }
      OptimizerPointer->SetMaximumIteration( m_OptimizerParameters->GetNumberOfIterationsFRPR() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::REGULARSTEPGRADIENTDESCENTOPTIMIZER)
    {
      itk::RegularStepGradientDescentOptimizer::Pointer OptimizerPointer = itk::RegularStepGradientDescentOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetGradientMagnitudeTolerance( m_OptimizerParameters->GetGradientMagnitudeToleranceRegularStepGradientDescent() );
      OptimizerPointer->SetMinimumStepLength( m_OptimizerParameters->GetMinimumStepLengthRegularStepGradientDescent() );
      OptimizerPointer->SetMaximumStepLength( m_OptimizerParameters->GetMaximumStepLengthRegularStepGradientDescent() );
      OptimizerPointer->SetNumberOfIterations( m_OptimizerParameters->GetNumberOfIterationsRegularStepGradientDescent() );
      OptimizerPointer->SetRelaxationFactor( m_OptimizerParameters->GetRelaxationFactorRegularStepGradientDescent() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::VERSORRIGID3DTRANSFORMOPTIMIZER)
    {
      itk::VersorRigid3DTransformOptimizer::Pointer OptimizerPointer = itk::VersorRigid3DTransformOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetGradientMagnitudeTolerance( m_OptimizerParameters->GetGradientMagnitudeToleranceVersorRigid3DTransform() );
      OptimizerPointer->SetMinimumStepLength( m_OptimizerParameters->GetMinimumStepLengthVersorRigid3DTransform() );
      OptimizerPointer->SetMaximumStepLength( m_OptimizerParameters->GetMaximumStepLengthVersorRigid3DTransform() );
      OptimizerPointer->SetNumberOfIterations( m_OptimizerParameters->GetNumberOfIterationsVersorRigid3DTransform() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::VERSORTRANSFORMOPTIMIZER)
    {
      itk::VersorTransformOptimizer::Pointer OptimizerPointer = itk::VersorTransformOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetGradientMagnitudeTolerance( m_OptimizerParameters->GetGradientMagnitudeToleranceVersorTransform() );
      OptimizerPointer->SetMinimumStepLength( m_OptimizerParameters->GetMinimumStepLengthVersorTransform() );
      OptimizerPointer->SetMaximumStepLength( m_OptimizerParameters->GetMaximumStepLengthVersorTransform() );
      OptimizerPointer->SetNumberOfIterations( m_OptimizerParameters->GetNumberOfIterationsVersorTransform() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::AMOEBAOPTIMIZER)
    {
      itk::AmoebaOptimizer::Pointer OptimizerPointer = itk::AmoebaOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetParametersConvergenceTolerance(m_OptimizerParameters->GetParametersConvergenceToleranceAmoeba());
      OptimizerPointer->SetFunctionConvergenceTolerance(m_OptimizerParameters->GetFunctionConvergenceToleranceAmoeba());      
      itk::Array<double> simplexDeltaAmoeba = m_OptimizerParameters->GetSimplexDeltaAmoeba();
      OptimizerType::ParametersType simplexDelta( m_NumberTransformParameters );
      for (int i = 0; i < m_NumberTransformParameters; i++)
      {
        simplexDelta[i] = simplexDeltaAmoeba[i];
      }
      OptimizerPointer->AutomaticInitialSimplexOff();
      OptimizerPointer->SetInitialSimplexDelta( simplexDelta );
      OptimizerPointer->SetMaximumNumberOfIterations( m_OptimizerParameters->GetNumberOfIterationsAmoeba() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::CONJUGATEGRADIENTOPTIMIZER)
    {
      itk::ConjugateGradientOptimizer::Pointer OptimizerPointer = itk::ConjugateGradientOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      /*typedef  itk::ConjugateGradientOptimizer::InternalOptimizerType  vnlOptimizerType;
      vnlOptimizerType * vnlOptimizer = OptimizerPointer->GetOptimizer();
      vnlOptimizer->set_f_tolerance( 1e-3 );
      vnlOptimizer->set_g_tolerance( 1e-4 );
      vnlOptimizer->set_x_tolerance( 1e-8 ); 
      vnlOptimizer->set_epsilon_function( 1e-10 );
      vnlOptimizer->set_max_function_evals( m_Iterations );
      vnlOptimizer->set_check_derivatives( 3 );*/

      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::LBFGSOPTIMIZER)
    {
      itk::LBFGSOptimizer::Pointer OptimizerPointer = itk::LBFGSOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->SetGradientConvergenceTolerance( m_OptimizerParameters->GetGradientConvergenceToleranceLBFGS() );
      OptimizerPointer->SetLineSearchAccuracy( m_OptimizerParameters->GetLineSearchAccuracyLBFGS() );
      OptimizerPointer->SetDefaultStepLength( m_OptimizerParameters->GetDefaultStepLengthLBFGS() );
      OptimizerPointer->SetTrace(m_OptimizerParameters->GetTraceOnLBFGS());
      OptimizerPointer->SetMaximumNumberOfFunctionEvaluations( m_OptimizerParameters->GetNumberOfIterationsLBFGS() );
      return OptimizerPointer.GetPointer();
    }
    else if (optimizer == OptimizerParameters::SPSAOPTIMIZER)
    {
      itk::SPSAOptimizer::Pointer OptimizerPointer = itk::SPSAOptimizer::New();
      OptimizerPointer->SetMaximize( m_OptimizerParameters->GetMaximize() );
      OptimizerPointer->Seta( m_OptimizerParameters->GetaSPSA() );
      OptimizerPointer->SetA( m_OptimizerParameters->GetASPSA() );
      OptimizerPointer->SetAlpha( m_OptimizerParameters->GetAlphaSPSA() );
      OptimizerPointer->Setc( m_OptimizerParameters->GetcSPSA() );
      OptimizerPointer->SetGamma( m_OptimizerParameters->GetGammaSPSA() );
      OptimizerPointer->SetTolerance(m_OptimizerParameters->GetToleranceSPSA());
      OptimizerPointer->SetStateOfConvergenceDecayRate(m_OptimizerParameters->GetStateOfConvergenceDecayRateSPSA());
      OptimizerPointer->SetMinimumNumberOfIterations(m_OptimizerParameters->GetMinimumNumberOfIterationsSPSA());
      OptimizerPointer->SetNumberOfPerturbations(m_OptimizerParameters->GetNumberOfPerturbationsSPSA());
      OptimizerPointer->SetMaximumNumberOfIterations(m_OptimizerParameters->GetNumberOfIterationsSPSA());
      return OptimizerPointer.GetPointer();
    }
    return NULL;
  }
} // end namespace
