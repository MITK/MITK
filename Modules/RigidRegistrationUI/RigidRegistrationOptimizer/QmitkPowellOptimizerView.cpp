/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkPowellOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkPowellOptimizer.h>

QmitkPowellOptimizerView::QmitkPowellOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkPowellOptimizerView::~QmitkPowellOptimizerView()
{
}

itk::Object::Pointer QmitkPowellOptimizerView::GetOptimizer()
{
  itk::PowellOptimizer::Pointer OptimizerPointer = itk::PowellOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetStepLength( m_Controls.m_StepLengthPowell->text().toFloat() );
  OptimizerPointer->SetStepTolerance( m_Controls.m_StepTolerancePowell->text().toFloat() );
  OptimizerPointer->SetValueTolerance( m_Controls.m_ValueTolerancePowell->text().toFloat() );
  OptimizerPointer->SetMaximumIteration( m_Controls.m_IterationsPowell->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkPowellOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(5);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_StepLengthPowell->text().toFloat();
  optimizerValues[2] = m_Controls.m_StepTolerancePowell->text().toFloat();
  optimizerValues[3] = m_Controls.m_ValueTolerancePowell->text().toFloat();
  optimizerValues[4] = m_Controls.m_IterationsPowell->text().toInt();
  return optimizerValues;
}

void QmitkPowellOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_StepLengthPowell->setText(QString::number(optimizerValues[1]));
  m_Controls.m_StepTolerancePowell->setText(QString::number(optimizerValues[2]));
  m_Controls.m_ValueTolerancePowell->setText(QString::number(optimizerValues[3]));
  m_Controls.m_IterationsPowell->setText(QString::number(optimizerValues[4]));
}

void QmitkPowellOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkPowellOptimizerView::GetName()
{
  return "Powell";
}

void QmitkPowellOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsPowell->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_StepLengthPowell->setValidator(validatorLineEditInputFloat);
  m_Controls.m_StepTolerancePowell->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ValueTolerancePowell->setValidator(validatorLineEditInputFloat);
}
