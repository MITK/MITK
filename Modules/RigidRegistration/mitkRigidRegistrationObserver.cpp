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

#include "mitkRigidRegistrationObserver.h"
#include "mitkProgressBar.h"

mitk::RigidRegistrationObserver::RigidRegistrationObserver() : m_OptimizerValue(0), m_StopOptimization(false)
{

}

void mitk::RigidRegistrationObserver::HandleOptimizationIterationEvent(OptimizerType *optimizer)
{
  unsigned int iteration = 0;

  itk::RegularStepGradientDescentBaseOptimizer* rsgdbase = dynamic_cast< itk::RegularStepGradientDescentBaseOptimizer* >( optimizer );
  if( rsgdbase != nullptr )
  {
    m_OptimizerValue = rsgdbase->GetValue();
    m_Params = rsgdbase->GetCurrentPosition();
    iteration = rsgdbase->GetCurrentIteration();

    MITK_INFO << "("<< iteration << ") "   << m_OptimizerValue << " :: "<<  m_Params << std::endl;

    if( this->m_StopOptimization )
    {
      rsgdbase->StopOptimization();
      m_StopOptimization = false;
    }
    return;
  }

  itk::GradientDescentOptimizer* gdbase = dynamic_cast< itk::GradientDescentOptimizer* >( optimizer );
  if( gdbase != nullptr )
  {
    m_OptimizerValue = gdbase->GetValue();
    m_Params = gdbase->GetCurrentPosition();
    iteration = gdbase->GetCurrentIteration();

    MITK_INFO << "("<< iteration << ") "   << m_OptimizerValue << " :: "<<  m_Params << std::endl;

    if( this->m_StopOptimization )
    {
      gdbase->StopOptimization();
      m_StopOptimization = false;
    }
    return;
  }

  itk::PowellOptimizer* powbase = dynamic_cast< itk::PowellOptimizer* >( optimizer );
  if( powbase != nullptr )
  {
    m_OptimizerValue = powbase->GetValue();
    m_Params = powbase->GetCurrentPosition();
    iteration = powbase->GetCurrentIteration();

    MITK_INFO << "("<< iteration << ") "   << m_OptimizerValue << " :: "<<  m_Params << std::endl;

    if( this->m_StopOptimization )
    {
      powbase->StopOptimization();
      m_StopOptimization = false;
    }
    return;
  }

  itk::OnePlusOneEvolutionaryOptimizer* opluso = dynamic_cast< itk::OnePlusOneEvolutionaryOptimizer* >( optimizer );
  if( opluso != nullptr )
  {
    m_OptimizerValue = opluso->GetValue();
    m_Params = opluso->GetCurrentPosition();
    iteration = opluso->GetCurrentIteration();

    MITK_INFO << "("<< iteration << ") "   << m_OptimizerValue << " :: "<<  m_Params << std::endl;

    if( this->m_StopOptimization )
    {
      powbase->StopOptimization();
      m_StopOptimization = false;
    }
    return;
  }

}

void mitk::RigidRegistrationObserver::Execute(itk::Object *caller, const itk::EventObject & event)
{
  if (typeid(event) == typeid(itk::IterationEvent))
  {
    OptimizerPointer optimizer = dynamic_cast<OptimizerPointer>(caller);

    if (optimizer != nullptr)
    {
      this->HandleOptimizationIterationEvent(optimizer);

      InvokeEvent( itk::ModifiedEvent() );
    }

  }
  else if (typeid(event) == typeid(itk::FunctionEvaluationIterationEvent))
  {
    OptimizerPointer optimizer = dynamic_cast<OptimizerPointer>(caller);
    itk::AmoebaOptimizer* AmoebaOptimizer = dynamic_cast<itk::AmoebaOptimizer*>(optimizer);
    if (AmoebaOptimizer != nullptr)
    {
      m_OptimizerValue = AmoebaOptimizer->GetCachedValue();
      MITK_INFO << AmoebaOptimizer->GetCachedValue() << "   "
               << AmoebaOptimizer->GetCachedCurrentPosition() << std::endl;
      m_Params = AmoebaOptimizer->GetCachedCurrentPosition();
      if(m_StopOptimization)
      {
        //AmoebaOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
  }
  mitk::ProgressBar::GetInstance()->AddStepsToDo(1);
  mitk::ProgressBar::GetInstance()->Progress();
}

void mitk::RigidRegistrationObserver::Execute(const itk::Object* /*caller*/, const itk::EventObject& /*event*/)
{
}

void mitk::RigidRegistrationObserver::AddStepsToDo(int steps)
{
  mitk::ProgressBar::GetInstance()->AddStepsToDo(steps);
}

void mitk::RigidRegistrationObserver::SetRemainingProgress(int steps)
{
  mitk::ProgressBar::GetInstance()->Progress(steps);
}

double mitk::RigidRegistrationObserver::GetCurrentOptimizerValue()
{
  return m_OptimizerValue;
}

itk::Array<double> mitk::RigidRegistrationObserver::GetCurrentTranslation()
{
  return m_Params;
}

// Sets the stop optimization flag, which is used to call the StopOptimization() method of the optimizer.
// Unfortunately it is not implemented for ExhaustiveOptimizer, LBFGSBOptimizer, AmoebaOptimizer, ConjugateGradientOptimizer and LBFGSOptimizer in ITK.
void mitk::RigidRegistrationObserver::SetStopOptimization(bool stopOptimization)
{
  m_StopOptimization = stopOptimization;
}
