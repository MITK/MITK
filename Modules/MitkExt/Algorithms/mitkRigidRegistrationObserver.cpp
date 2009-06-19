/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkRigidRegistrationObserver.h"
#include "mitkProgressBar.h"

mitk::RigidRegistrationObserver::RigidRegistrationObserver() : m_OptimizerValue(0), m_StopOptimization(false)
{  
}

void mitk::RigidRegistrationObserver::Execute(itk::Object *caller, const itk::EventObject & event)
{
  if (typeid(event) == typeid(itk::IterationEvent))
  {
    OptimizerPointer optimizer = dynamic_cast<OptimizerPointer>(caller);
    ExhaustiveOptimizerPointer ExhaustiveOptimizer = dynamic_cast<ExhaustiveOptimizerPointer>(optimizer);
    GradientDescentOptimizerPointer GradientDescentOptimizer = dynamic_cast<GradientDescentOptimizerPointer>(optimizer);
    QuaternionRigidTransformGradientDescentOptimizerPointer QuaternionRigidTransformGradientDescentOptimizer = dynamic_cast<QuaternionRigidTransformGradientDescentOptimizerPointer>(optimizer);
    LBFGSBOptimizerPointer LBFGSBOptimizer = dynamic_cast<LBFGSBOptimizerPointer>(optimizer);
    OnePlusOneEvolutionaryOptimizerPointer OnePlusOneEvolutionaryOptimizer = dynamic_cast<OnePlusOneEvolutionaryOptimizerPointer>(optimizer);
    PowellOptimizerPointer PowellOptimizer = dynamic_cast<PowellOptimizerPointer>(optimizer);
    FRPROptimizerPointer FRPROptimizer = dynamic_cast<FRPROptimizerPointer>(optimizer);
    RegularStepGradientDescentOptimizerPointer RegularStepGradientDescentOptimizer = dynamic_cast<RegularStepGradientDescentOptimizerPointer>(optimizer);
    VersorRigid3DTransformOptimizerPointer VersorRigid3DTransformOptimizer = dynamic_cast<VersorRigid3DTransformOptimizerPointer>(optimizer);
    VersorTransformOptimizerPointer VersorTransformOptimizer = dynamic_cast<VersorTransformOptimizerPointer>(optimizer);
    AmoebaOptimizerPointer AmoebaOptimizer = dynamic_cast<AmoebaOptimizerPointer>(optimizer);
    ConjugateGradientOptimizerPointer ConjugateGradientOptimizer = dynamic_cast<ConjugateGradientOptimizerPointer>(optimizer);
    LBFGSOptimizerPointer LBFGSOptimizer = dynamic_cast<LBFGSOptimizerPointer>(optimizer);
    SPSAOptimizerPointer SPSAOptimizer = dynamic_cast<SPSAOptimizerPointer>(optimizer);
    if (ExhaustiveOptimizer != NULL)
    {
      m_OptimizerValue = ExhaustiveOptimizer->GetValue(ExhaustiveOptimizer->GetCurrentPosition());
      LOG_INFO << ExhaustiveOptimizer->GetValue(ExhaustiveOptimizer->GetCurrentPosition()) << "   "
               << ExhaustiveOptimizer->GetCurrentPosition() << std::endl;
      m_Params = ExhaustiveOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        //ExhaustiveOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (GradientDescentOptimizer != NULL)
    {
      m_OptimizerValue = GradientDescentOptimizer->GetValue();
      LOG_INFO << GradientDescentOptimizer->GetCurrentIteration() << "   "
               << GradientDescentOptimizer->GetValue() << "   "
               << GradientDescentOptimizer->GetCurrentPosition() << std::endl;
      m_Params = GradientDescentOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        GradientDescentOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (QuaternionRigidTransformGradientDescentOptimizer != NULL)
    {
      m_OptimizerValue = QuaternionRigidTransformGradientDescentOptimizer->GetValue();
      LOG_INFO << QuaternionRigidTransformGradientDescentOptimizer->GetCurrentIteration() << "   "
               << QuaternionRigidTransformGradientDescentOptimizer->GetValue() << "   "
               << QuaternionRigidTransformGradientDescentOptimizer->GetCurrentPosition() << std::endl;
      m_Params = QuaternionRigidTransformGradientDescentOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        QuaternionRigidTransformGradientDescentOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (LBFGSBOptimizer != NULL)
    {
      m_OptimizerValue = LBFGSBOptimizer->GetValue();
      LOG_INFO << LBFGSBOptimizer->GetCurrentIteration() << "   "
               << LBFGSBOptimizer->GetValue() << "   "
               << LBFGSBOptimizer->GetCurrentPosition() << std::endl;
      m_Params = LBFGSBOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        //LBFGSBOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (OnePlusOneEvolutionaryOptimizer != NULL)
    {
      m_OptimizerValue = OnePlusOneEvolutionaryOptimizer->GetValue();
      LOG_INFO << OnePlusOneEvolutionaryOptimizer->GetCurrentIteration() << "   "
               << OnePlusOneEvolutionaryOptimizer->GetValue() << "   "
               << OnePlusOneEvolutionaryOptimizer->GetCurrentPosition() << std::endl;
      m_Params = OnePlusOneEvolutionaryOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        OnePlusOneEvolutionaryOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (PowellOptimizer != NULL)
    {
      m_OptimizerValue = PowellOptimizer->GetValue();
      LOG_INFO << PowellOptimizer->GetCurrentIteration() << "   "
               << PowellOptimizer->GetValue() << "   "
               << PowellOptimizer->GetCurrentPosition() << std::endl;
      m_Params = PowellOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        PowellOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (FRPROptimizer != NULL)
    {
      m_OptimizerValue = FRPROptimizer->GetValue();
      LOG_INFO << FRPROptimizer->GetCurrentIteration() << "   "
               << FRPROptimizer->GetValue() << "   "
               << FRPROptimizer->GetCurrentPosition() << std::endl;
      m_Params = FRPROptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        FRPROptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (RegularStepGradientDescentOptimizer != NULL)
    {
      m_OptimizerValue = RegularStepGradientDescentOptimizer->GetValue();
      LOG_INFO << RegularStepGradientDescentOptimizer->GetCurrentIteration() << "   "
               << RegularStepGradientDescentOptimizer->GetValue() << "   "
               << RegularStepGradientDescentOptimizer->GetCurrentPosition() << std::endl;
      m_Params = RegularStepGradientDescentOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        RegularStepGradientDescentOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (VersorRigid3DTransformOptimizer != NULL)
    {
      m_OptimizerValue = VersorRigid3DTransformOptimizer->GetValue();
      LOG_INFO << VersorRigid3DTransformOptimizer->GetCurrentIteration() << "   "
               << VersorRigid3DTransformOptimizer->GetValue() << "   "
               << VersorRigid3DTransformOptimizer->GetCurrentPosition() << std::endl;
      m_Params = VersorRigid3DTransformOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        VersorRigid3DTransformOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (VersorTransformOptimizer != NULL)
    {
      m_OptimizerValue = VersorTransformOptimizer->GetValue();
      LOG_INFO << VersorTransformOptimizer->GetCurrentIteration() << "   "
               << VersorTransformOptimizer->GetValue() << "   "
               << VersorTransformOptimizer->GetCurrentPosition() << std::endl;
      m_Params = VersorTransformOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        VersorTransformOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (AmoebaOptimizer != NULL)
    {
      m_OptimizerValue = AmoebaOptimizer->GetCachedValue();
      LOG_INFO << AmoebaOptimizer->GetCachedValue() << "   "
               << AmoebaOptimizer->GetCachedCurrentPosition() << std::endl;
      m_Params = AmoebaOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        //AmoebaOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (ConjugateGradientOptimizer != NULL)
    {
      m_OptimizerValue = ConjugateGradientOptimizer->GetValue();
      LOG_INFO << ConjugateGradientOptimizer->GetCurrentIteration() << "   "
               << ConjugateGradientOptimizer->GetValue() << "   "
               << ConjugateGradientOptimizer->GetCurrentPosition() << std::endl;
      m_Params = ConjugateGradientOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        //ConjugateGradientOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (LBFGSOptimizer != NULL)
    {
      if(m_StopOptimization)
      {
        //LBFGSOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      //m_OptimizerValue = LBFGSOptimizer->GetValue();
      /*LOG_INFO << LBFGSOptimizer->GetCurrentIteration() << "   ";
      LOG_INFO << LBFGSOptimizer->GetValue() << "   ";
      LOG_INFO << LBFGSOptimizer->GetInfinityNormOfProjectedGradient() << std::endl;*/
      InvokeEvent(itk::ModifiedEvent());
    }
    else if (SPSAOptimizer != NULL)
    {
      m_OptimizerValue = SPSAOptimizer->GetValue();
      LOG_INFO << SPSAOptimizer->GetCurrentIteration() << "   "
               << SPSAOptimizer->GetValue() << "   "
               << SPSAOptimizer->GetCurrentPosition() << std::endl;
      m_Params = SPSAOptimizer->GetCurrentPosition();
      if(m_StopOptimization)
      {
        SPSAOptimizer->StopOptimization();
        m_StopOptimization = false;
      }
      InvokeEvent(itk::ModifiedEvent());
    }
  }
  else if (typeid(event) == typeid(itk::FunctionEvaluationIterationEvent))
  {
    OptimizerPointer optimizer = dynamic_cast<OptimizerPointer>(caller);
    AmoebaOptimizerPointer AmoebaOptimizer = dynamic_cast<AmoebaOptimizerPointer>(optimizer);
    if (AmoebaOptimizer != NULL)
    {
      m_OptimizerValue = AmoebaOptimizer->GetCachedValue();
      LOG_INFO << AmoebaOptimizer->GetCachedValue() << "   "
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
