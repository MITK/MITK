/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef MITKRIGIDREGISTRATIONOBSERVER_H
#define MITKRIGIDREGISTRATIONOBSERVER_H

#include "itkSingleValuedNonLinearOptimizer.h"
#include "MitkRigidRegistrationExports.h"
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
  * \ingroup RigidRegistration
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
  class MITKRIGIDREGISTRATION_EXPORT RigidRegistrationObserver : public itk::Command
  {
    public:
      typedef  RigidRegistrationObserver   Self;
      typedef  itk::Command             Superclass;
      typedef itk::SmartPointer<Self>  Pointer;
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      typedef itk::SingleValuedNonLinearOptimizer OptimizerType;
      typedef OptimizerType * OptimizerPointer;

      /**
      * \brief Reacts on events from ITK optimizers.
      *
      * Stores the optimizer values, adds progress to the progress bar and sends a stop flag to stop the optimization process if it is
      * set in this class. Also emits a signal to inform listeners about new optimizer values.
      */
      void Execute(itk::Object *caller, const itk::EventObject & event) override;

      /**
      * \brief Not implemented...
      *
      */
      void Execute(const itk::Object * object, const itk::EventObject & event) override;

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

      void HandleOptimizationIterationEvent(OptimizerType *optimizer );

  private:
    double m_OptimizerValue;
    itk::Array<double> m_Params;
    bool m_StopOptimization;

  };

  template < class RegistrationType >
  class MITKRIGIDREGISTRATION_EXPORT RigidRegistrationPyramidObserver : public itk::Command
  {
  public:
    typedef itk::RegularStepGradientDescentBaseOptimizer OptimizerType;

    mitkClassMacroItkParent( RigidRegistrationPyramidObserver<RegistrationType>, itk::Command)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    void Execute(itk::Object *caller, const itk::EventObject & /*event*/) override
    {
      RegistrationType* registration = dynamic_cast< RegistrationType* >( caller );

      if( registration == NULL)
        return;

      OptimizerType* optimizer = dynamic_cast< OptimizerType* >(registration->GetOptimizer());

      if( optimizer == NULL)
      {
         MITK_WARN("Pyramid.Registration.Command") << "No step adaptation possible with given optimizer, cast failed! ";
            return;
      }


      MITK_DEBUG << "\t - Pyramid level " << registration->GetCurrentLevel();

      if( registration->GetCurrentLevel() == 0 )
      {
        MITK_INFO("Pyramid.Registration.Command") << "First pyramid resolution level: ";
        MITK_INFO("Pyramid.Registration.Command") << " Current settings: \n"
                                                  << " Step length: (" << optimizer->GetMinimumStepLength()
                                                  << "," << optimizer->GetMaximumStepLength() << "), Tolerance: "
                                                  << optimizer->GetGradientMagnitudeTolerance() << " Iterations: "
                                                  << optimizer->GetNumberOfIterations();
            return;
      }

      optimizer->SetMaximumStepLength( optimizer->GetMaximumStepLength() * 0.25f );
      optimizer->SetMinimumStepLength( optimizer->GetMinimumStepLength() * 0.1f );
      optimizer->SetGradientMagnitudeTolerance( optimizer->GetGradientMagnitudeTolerance() * 0.1f );
      optimizer->SetNumberOfIterations( optimizer->GetNumberOfIterations() * 1.5f );

      MITK_INFO("Pyramid.Registration.Command") << " Current settings: \n"
                                                << " Step length: (" << optimizer->GetMinimumStepLength()
                                                << "," << optimizer->GetMaximumStepLength() << "), Tolerance: "
                                                << optimizer->GetGradientMagnitudeTolerance() << " Iterations: "
                                                << optimizer->GetNumberOfIterations();

    }

    void Execute(const itk::Object * /*object*/, const itk::EventObject & /*event*/) override {}
  };


} // namespace mitk

#endif // MITKRIGIDREGISTRATIONOBSERVER_H
