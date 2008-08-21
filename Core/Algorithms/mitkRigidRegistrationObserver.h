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

  /**
  * \brief Observer to react on rigid registration optimizer events. 
  * 
  * \sa ProgressBar
  *
  * \ingroup Registration
  *
  * This class reacts on events sent by ITK optimizers. These events will be sent for every iteration the optimizer performs.
  * This class also takes care for the progress bar for every iteration step.
  *
  * The current optimizer values will be stored and a modified event will be sent to listeners registered to this observer.  
  * These listeners have the possibility to get the current optimizer parameters.
  * 
  * The optimization process can be stopped by setting stopOptimization to true. The optimization will be stopped after the next
  * iteration step of the optimizer. Unfortunately this is not implemented for ExhaustiveOptimizer, LBFGSBOptimizer, AmoebaOptimizer, 
  * ConjugateGradientOptimizer and LBFGSOptimizer in ITK.
  *
  * \author Daniel Stein
  */
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

      /**
      * \brief Reacts on events from ITK optimizers.
      * 
      * Stores the optimizer values, adds progress to the progress bar and sends a stop flag to stop the optimization process if it is
      * set in this class. Also emits a signal to inform listeners about new optimizer values.
      */
      void Execute(itk::Object *caller, const itk::EventObject & event);
      
      /**
      * \brief Not implemented...
      *
      */
      void Execute(const itk::Object * object, const itk::EventObject & event);

      /**
      * \brief Add new steps to the progress bar.
      *
      */
      void AddStepsToDo(int steps);
      
      /**
      * \brief Sets the remaining progress to the progress bar when the optimization process is done.
      *
      */
      void SetRemainingProgress(int steps);
      
      /**
      * \brief Returns the current optimizer value. This value is calculated by the used metric and shows, how good the images are aligned.
      *
      * 
      */
      double GetCurrentOptimizerValue();
      
      /**
      * \brief Returns the current transformation parameters for the moving image to this iteration step.
      *
      * These can include parameters for translation, scaling, rotation and shearing.
      */
      itk::Array<double> GetCurrentTranslation();
      
      /**
      * \brief Sets the stop optimization flag, which is used to call the StopOptimization() method of the optimizer.
      *
      * Unfortunately it is not implemented for ExhaustiveOptimizer, LBFGSBOptimizer, AmoebaOptimizer, 
      * ConjugateGradientOptimizer and LBFGSOptimizer in ITK.
      */
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
