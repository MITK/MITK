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

#include <itkObject.h>
#include "itkArray.h"
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT OptimizerParameters : public itk::Object
  {
  public:
    itkTypeMacro(ProgressBar, itk::Object);

    static OptimizerParameters* GetInstance(); //singleton

    static const int EXHAUSTIVEOPTIMIZER = 0;
    static const int GRADIENTDESCENTOPTIMIZER = 1;
    static const int QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER = 2;
    static const int LBFGSBOPTIMIZER = 3;
    static const int ONEPLUSONEEVOLUTIONARYOPTIMIZER = 4;
    static const int POWELLOPTIMIZER = 5;
    static const int FRPROPTIMIZER = 6;
    static const int REGULARSTEPGRADIENTDESCENTOPTIMIZER = 7;
    static const int VERSORTRANSFORMOPTIMIZER = 8;
    static const int AMOEBAOPTIMIZER = 9;
    static const int CONJUGATEGRADIENTOPTIMIZER = 10;
    static const int LBFGSOPTIMIZER = 11;
    static const int SPSAOPTIMIZER = 12;
    static const int VERSORRIGID3DTRANSFORMOPTIMIZER = 13;

    // for all Optimizer
    void SetOptimizer(int optimizer);
    int GetOptimizer();

    void SetDimension(unsigned int dimension);
    unsigned int GetDimension();

    void SetMaximize(bool maximize);
    bool GetMaximize();


    // for itk::ExhaustiveOptimizer
    void SetStepLengthExhaustive(float stepLength);
    float GetStepLengthExhaustive();

    void SetNumberOfStepsExhaustive(int numberOfSteps);
    int GetNumberOfStepsExhaustive();

    // for itk::GradientDescentOptimizer
    void SetLearningRateGradientDescent(float learningRate);
    float GetLearningRateGradientDescent();
    
    void SetNumberOfIterationsGradientDescent(int iterations);
    int GetNumberOfIterationsGradientDescent();

    // for itk::QuaternionRigidTransformGradientDescentOptimizer
    void SetLearningRateQuaternionRigidTransformGradientDescent(float learningRate);
    float GetLearningRateQuaternionRigidTransformGradientDescent();

    void SetNumberOfIterationsQuaternionRigidTransformGradientDescent(int iterations);
    int GetNumberOfIterationsQuaternionRigidTransformGradientDescent();

    // for itk::OnePlusOneEvolutionaryOptimizer

    void SetGrowthFactorOnePlusOneEvolutionary(float growthFactor);
    float GetGrowthFactorOnePlusOneEvolutionary();

    void SetShrinkFactorOnePlusOneEvolutionary(float shrinkFactor);
    float GetShrinkFactorOnePlusOneEvolutionary();

    void SetEpsilonOnePlusOneEvolutionary(float epsilon);
    float GetEpsilonOnePlusOneEvolutionary();

    void SetInitialRadiusOnePlusOneEvolutionary(float initialRadius);
    float GetInitialRadiusOnePlusOneEvolutionary();

    void SetNumberOfIterationsOnePlusOneEvolutionary(int iterations);
    int GetNumberOfIterationsOnePlusOneEvolutionary();

    // for itk::PowellOptimizer
    void SetStepLengthPowell(float stepLength);
    float GetStepLengthPowell();

    void SetStepTolerancePowell(float stepTolerance);
    float GetStepTolerancePowell();

    void SetValueTolerancePowell(float valueTolerance);
    float GetValueTolerancePowell();

    void SetNumberOfIterationsPowell(int iterations);
    int GetNumberOfIterationsPowell();

    // for itk::FRPROptimizer
    void SetStepLengthFRPR(float stepLength);
    float GetStepLengthFRPR();

    void SetToFletchReevesFRPR(bool fletchReeves);
    bool GetFletchReevesFRPR();

    void SetToPolakRibiereFRPR(bool polakRibiere);
    bool GetPolakRibiereFRPR();
    
    void SetNumberOfIterationsFRPR(int iterations);
    int GetNumberOfIterationsFRPR();

    // for itk::RegularStepGradientDescentOptimizer
    void SetGradientMagnitudeToleranceRegularStepGradientDescent(float gradientMagnitudeTolerance);
    float GetGradientMagnitudeToleranceRegularStepGradientDescent();

    void SetMinimumStepLengthRegularStepGradientDescent(float minimumStepLength);
    float GetMinimumStepLengthRegularStepGradientDescent();

    void SetMaximumStepLengthRegularStepGradientDescent(float minimumStepLength);
    float GetMaximumStepLengthRegularStepGradientDescent();

    void SetNumberOfIterationsRegularStepGradientDescent(int iterations);
    int GetNumberOfIterationsRegularStepGradientDescent();

    void SetRelaxationFactorRegularStepGradientDescent(double relaxationFactor);
    double GetRelaxationFactorRegularStepGradientDescent();

    // for itk::VersorRigid3DTransformOptimizer
    void SetGradientMagnitudeToleranceVersorRigid3DTransform(float gradientMagnitudeTolerance);
    float GetGradientMagnitudeToleranceVersorRigid3DTransform();

    void SetMinimumStepLengthVersorRigid3DTransform(float minimumStepLength);
    float GetMinimumStepLengthVersorRigid3DTransform();

    void SetMaximumStepLengthVersorRigid3DTransform(float minimumStepLength);
    float GetMaximumStepLengthVersorRigid3DTransform();

    void SetNumberOfIterationsVersorRigid3DTransform(int iterations);
    int GetNumberOfIterationsVersorRigid3DTransform();

    // for itk::VersorTransformOptimizer
    void SetGradientMagnitudeToleranceVersorTransform(float gradientMagnitudeTolerance);
    float GetGradientMagnitudeToleranceVersorTransform();

    void SetMinimumStepLengthVersorTransform(float minimumStepLength);
    float GetMinimumStepLengthVersorTransform();

    void SetMaximumStepLengthVersorTransform(float minimumStepLength);
    float GetMaximumStepLengthVersorTransform();

    void SetNumberOfIterationsVersorTransform(int iterations);
    int GetNumberOfIterationsVersorTransform();

    // for itk::AmoebaOptimizer
    void SetSimplexDeltaAmoeba(itk::Array<double> simplexDelta);
    itk::Array<double> GetSimplexDeltaAmoeba();

    void SetParametersConvergenceToleranceAmoeba(float parametersConvergenceTolerance);
    float GetParametersConvergenceToleranceAmoeba();

    void SetFunctionConvergenceToleranceAmoeba(float functionConvergenceTolerance);
    float GetFunctionConvergenceToleranceAmoeba();

    void SetNumberOfIterationsAmoeba(int iterations);
    int GetNumberOfIterationsAmoeba();

    // for itk::ConjugateGradientOptimizer

    // for itk::LBFGSOptimizer
    void SetGradientConvergenceToleranceLBFGS(float gradientConvergenceTolerance);
    float GetGradientConvergenceToleranceLBFGS();

    void SetLineSearchAccuracyLBFGS(float lineSearchAccuracy);
    float GetLineSearchAccuracyLBFGS();

    void SetDefaultStepLengthLBFGS(float DefaultStepLength);
    float GetDefaultStepLengthLBFGS();

    void SetTraceOnLBFGS(bool traceOn);
    bool GetTraceOnLBFGS();

    void SetNumberOfIterationsLBFGS(int iterations);
    int GetNumberOfIterationsLBFGS();

    // for itk::SPSAOptimizer
    void SetaSPSA(float a);
    float GetaSPSA();
  
    void SetASPSA(float A);
    float GetASPSA();

    void SetAlphaSPSA(float alpha);
    float GetAlphaSPSA();

    void SetcSPSA(float c);
    float GetcSPSA();

    void SetGammaSPSA(float gamma);
    float GetGammaSPSA();

    void SetToleranceSPSA(float tolerance);
    float GetToleranceSPSA();

    void SetStateOfConvergenceDecayRateSPSA(float stateOfConvergenceDecayRate);
    float GetStateOfConvergenceDecayRateSPSA();

    void SetMinimumNumberOfIterationsSPSA(int minimumNumberOfIterations);
    int GetMinimumNumberOfIterationsSPSA();

    void SetNumberOfPerturbationsSPSA(int numberOfPerturbations);
    int GetNumberOfPerturbationsSPSA();

    void SetNumberOfIterationsSPSA(int iterations);
    int GetNumberOfIterationsSPSA();

  protected:
    OptimizerParameters(); // hidden, access through GetInstance()
    ~OptimizerParameters();
    static OptimizerParameters* m_Instance;

    // for all Optimizer
    int m_Optimizer;
    unsigned int m_Dimension;
    bool m_Maximize;
    
    // for itk::ExhaustiveOptimizer
    float m_StepLengthExhaustive;
    int m_NumberOfStepsExhaustive;

    // for itk::GradientDescentOptimizer
    float m_LearningRateGradientDescent;
    int m_IterationsGradientDescent;

    // for itk::QuaternionRigidTransformGradientDescentOptimizer
    float m_LearningRateQuaternionRigidTransformGradientDescent;
    int m_IterationsQuaternionRigidTransformGradientDescent;

    // for itk::LBFGSBOptimizer

    // for itk::OnePlusOneEvolutionaryOptimizer
    float m_GrowthFactorOnePlusOneEvolutionary;
    float m_ShrinkFactorOnePlusOneEvolutionary;
    float m_EpsilonOnePlusOneEvolutionary;
    float m_InitialRadiusOnePlusOneEvolutionary;
    int m_IterationsOnePlusOneEvolutionary;

    // for itk::PowellOptimizer
    float m_StepLengthPowell;
    float m_StepTolerancePowell;
    float m_ValueTolerancePowell;
    int m_IterationsPowell;

    // for itk::FRPROptimizer
    float m_StepLengthFRPR;
    bool m_FletchReevesFRPR;
    bool m_PolakRibiereFRPR;
    int m_IterationsFRPR;

    // for itk::RegularStepGradientDescentOptimizer
    float m_GradientMagnitudeToleranceRegularStepGradientDescent;
    float m_MinimumStepLengthRegularStepGradientDescent;
    float m_MaximumStepLengthRegularStepGradientDescent;
    int m_IterationsRegularStepGradientDescent;
    double m_RelaxationFactorRegularStepGradientDescent;

    // for itk::VersorRigid3DTransformOptimizer
    float m_GradientMagnitudeToleranceVersorRigid3DTransform;
    float m_MinimumStepLengthVersorRigid3DTransform;
    float m_MaximumStepLengthVersorRigid3DTransform;
    int m_IterationsVersorRigid3DTransform;
    
    // for itk::VersorTransformOptimizer
    float m_GradientMagnitudeToleranceVersorTransform;
    float m_MinimumStepLengthVersorTransform;
    float m_MaximumStepLengthVersorTransform;
    int m_IterationsVersorTransform;

    // for itk::AmoebaOptimizer
    itk::Array<double> m_SimplexDeltaAmoeba;
    float m_ParametersConvergenceToleranceAmoeba;
    float m_FunctionConvergenceToleranceAmoeba;
    int m_IterationsAmoeba;

    // for itk::ConjugateGradientOptimizer

    // for itk::LBFGSOptimizer
    float m_GradientConvergenceToleranceLBFGS;
    float m_LineSearchAccuracyLBFGS;
    float m_DefaultStepLengthLBFGS;
    bool m_TraceOnLBFGS;
    int m_IterationsLBFGS;

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
    int m_IterationsSPSA;

  };

} // namespace mitk

#endif // MITKOPTIMIZERPARAMETERS_H
