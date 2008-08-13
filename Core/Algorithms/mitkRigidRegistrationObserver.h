/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
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

#ifndef MITKRIGIDREGISTRATIONOBSERVER_H
#define MITKRIGIDREGISTRATIONOBSERVER_H

#include "itkSingleValuedNonLinearOptimizer.h"
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
#include "itkCommand.h"
#include "mitkCommon.h"

namespace mitk {

  class MITK_CORE_EXPORT RigidRegistrationObserver : public itk::Command
  {
    public:
      typedef  RigidRegistrationObserver   Self;
      typedef  itk::Command             Superclass;
      typedef itk::SmartPointer<Self>  Pointer;
      itkNewMacro( Self );

      typedef itk::SingleValuedNonLinearOptimizer OptimizerType;
      typedef OptimizerType * OptimizerPointer;

      typedef itk::ExhaustiveOptimizer ExhaustiveOptimizerType;
      typedef ExhaustiveOptimizerType * ExhaustiveOptimizerPointer;

      typedef itk::GradientDescentOptimizer GradientDescentOptimizerType;
      typedef GradientDescentOptimizerType * GradientDescentOptimizerPointer;

      typedef itk::QuaternionRigidTransformGradientDescentOptimizer QuaternionRigidTransformGradientDescentOptimizerType;
      typedef QuaternionRigidTransformGradientDescentOptimizerType * QuaternionRigidTransformGradientDescentOptimizerPointer;

      typedef itk::LBFGSBOptimizer LBFGSBOptimizerType;
      typedef LBFGSBOptimizerType * LBFGSBOptimizerPointer;

      typedef itk::OnePlusOneEvolutionaryOptimizer OnePlusOneEvolutionaryOptimizerType;
      typedef OnePlusOneEvolutionaryOptimizerType * OnePlusOneEvolutionaryOptimizerPointer;

      typedef itk::PowellOptimizer PowellOptimizerType;
      typedef PowellOptimizerType * PowellOptimizerPointer;

      typedef itk::FRPROptimizer FRPROptimizerType;
      typedef FRPROptimizerType * FRPROptimizerPointer;

      typedef itk::RegularStepGradientDescentOptimizer RegularStepGradientDescentOptimizerType;
      typedef RegularStepGradientDescentOptimizerType * RegularStepGradientDescentOptimizerPointer;

      typedef itk::VersorRigid3DTransformOptimizer VersorRigid3DTransformOptimizerType;
      typedef VersorRigid3DTransformOptimizerType * VersorRigid3DTransformOptimizerPointer;

      typedef itk::VersorTransformOptimizer VersorTransformOptimizerType;
      typedef VersorTransformOptimizerType * VersorTransformOptimizerPointer;

      typedef itk::AmoebaOptimizer AmoebaOptimizerType;
      typedef AmoebaOptimizerType * AmoebaOptimizerPointer;

      typedef itk::ConjugateGradientOptimizer ConjugateGradientOptimizerType;
      typedef ConjugateGradientOptimizerType * ConjugateGradientOptimizerPointer;

      typedef itk::LBFGSOptimizer LBFGSOptimizerType;
      typedef LBFGSOptimizerType * LBFGSOptimizerPointer;

      typedef itk::SPSAOptimizer SPSAOptimizerType;
      typedef SPSAOptimizerType * SPSAOptimizerPointer;


      void Execute(itk::Object *caller, const itk::EventObject & event);
      void Execute(const itk::Object * object, const itk::EventObject & event);

      void AddStepsToDo(int steps);
      void SetRemainingProgress(int steps);
      double GetCurrentOptimizerValue();
      itk::Array<double> GetCurrentTranslation();
      void SetStopOptimization(bool stopOptimization);

    protected:
      RigidRegistrationObserver();

  private:
    double m_OptimizerValue;
    itk::Array<double> m_Params;
    bool m_StopOptimization;

  };  

} // namespace mitk

#endif // MITKRIGIDREGISTRATIONOBSERVER_H
