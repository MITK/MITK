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

#include "QmitkExhaustiveOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkExhaustiveOptimizer.h>

QmitkExhaustiveOptimizerView::QmitkExhaustiveOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
  m_NumberTransformParameters(16)
{

}

QmitkExhaustiveOptimizerView::~QmitkExhaustiveOptimizerView()
{
}

itk::Object::Pointer QmitkExhaustiveOptimizerView::GetOptimizer()
{
  itk::ExhaustiveOptimizer::Pointer OptimizerPointer = itk::ExhaustiveOptimizer::New();
  OptimizerPointer->SetStepLength( m_Controls.m_StepLengthExhaustive->text().toFloat() );
  itk::ExhaustiveOptimizer::StepsType steps( m_NumberTransformParameters );
  for (int i = 0; i < m_NumberTransformParameters; i++)
  {
    steps[i] = m_Controls.m_NumberOfStepsExhaustive->text().toInt();
  }
  OptimizerPointer->SetNumberOfSteps( steps );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkExhaustiveOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(1);
  optimizerValues.fill(0);
  optimizerValues[0] = 1;
  optimizerValues[1] = m_Controls.m_StepLengthExhaustive->text().toFloat();
  optimizerValues[2] = m_Controls.m_NumberOfStepsExhaustive->text().toInt();
  return optimizerValues;
}

void QmitkExhaustiveOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  //m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_StepLengthExhaustive->setText(QString::number(optimizerValues[1]));
  m_Controls.m_NumberOfStepsExhaustive->setText(QString::number(optimizerValues[2]));
}

void QmitkExhaustiveOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkExhaustiveOptimizerView::GetName()
{
  return "Exhaustive";
}

void QmitkExhaustiveOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfStepsExhaustive->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_StepLengthExhaustive->setValidator(validatorLineEditInputFloat);
}
