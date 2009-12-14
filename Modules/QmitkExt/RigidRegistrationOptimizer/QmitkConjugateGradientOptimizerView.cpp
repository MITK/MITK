/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkConjugateGradientOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkConjugateGradientOptimizer.h>

QmitkConjugateGradientOptimizerView::QmitkConjugateGradientOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkConjugateGradientOptimizerView::~QmitkConjugateGradientOptimizerView()
{
}

itk::Object::Pointer QmitkConjugateGradientOptimizerView::GetOptimizer()
{
  itk::ConjugateGradientOptimizer::Pointer OptimizerPointer = itk::ConjugateGradientOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
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

itk::Array<double> QmitkConjugateGradientOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(1);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  return optimizerValues;
}

void QmitkConjugateGradientOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
}

void QmitkConjugateGradientOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkConjugateGradientOptimizerView::GetName()
{
  return "ConjugateGradient";
}

void QmitkConjugateGradientOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
}
