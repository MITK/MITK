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

#include "QmitkRegularStepGradientDescentOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkRegularStepGradientDescentOptimizer.h>

QmitkRegularStepGradientDescentOptimizerView::QmitkRegularStepGradientDescentOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkRegularStepGradientDescentOptimizerView::~QmitkRegularStepGradientDescentOptimizerView()
{
}

itk::Object::Pointer QmitkRegularStepGradientDescentOptimizerView::GetOptimizer()
{
  itk::RegularStepGradientDescentOptimizer::Pointer OptimizerPointer = itk::RegularStepGradientDescentOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetGradientMagnitudeTolerance( m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->text().toFloat() );
  OptimizerPointer->SetMinimumStepLength( m_Controls.m_MinimumStepLengthRegularStepGradientDescent->text().toFloat() );
  OptimizerPointer->SetMaximumStepLength( m_Controls.m_MaximumStepLengthRegularStepGradientDescent->text().toFloat() );
  OptimizerPointer->SetRelaxationFactor( m_Controls.m_RelaxationFactorRegularStepGradientDescent->text().toFloat() );
  OptimizerPointer->SetNumberOfIterations( m_Controls.m_IterationsRegularStepGradientDescent->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkRegularStepGradientDescentOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(6);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->text().toFloat();
  optimizerValues[2] = m_Controls.m_MinimumStepLengthRegularStepGradientDescent->text().toFloat();
  optimizerValues[3] = m_Controls.m_MaximumStepLengthRegularStepGradientDescent->text().toFloat();
  optimizerValues[4] = m_Controls.m_RelaxationFactorRegularStepGradientDescent->text().toFloat();
  optimizerValues[5] = m_Controls.m_IterationsRegularStepGradientDescent->text().toInt();
  return optimizerValues;
}

void QmitkRegularStepGradientDescentOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->setText(QString::number(optimizerValues[1]));
  m_Controls.m_MinimumStepLengthRegularStepGradientDescent->setText(QString::number(optimizerValues[2]));
  m_Controls.m_MaximumStepLengthRegularStepGradientDescent->setText(QString::number(optimizerValues[3]));
  m_Controls.m_RelaxationFactorRegularStepGradientDescent->setText(QString::number(optimizerValues[4]));
  m_Controls.m_IterationsRegularStepGradientDescent->setText(QString::number(optimizerValues[5]));
}

void QmitkRegularStepGradientDescentOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkRegularStepGradientDescentOptimizerView::GetName()
{
  return "RegularStepGradientDescent";
}

void QmitkRegularStepGradientDescentOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsRegularStepGradientDescent->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_GradientMagnitudeToleranceRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MinimumStepLengthRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MaximumStepLengthRegularStepGradientDescent->setValidator(validatorLineEditInputFloat);
  QValidator* validatorLineEditInputFloat0to1 = new QDoubleValidator(0.000001, 0.999999, 8, this);
  m_Controls.m_RelaxationFactorRegularStepGradientDescent->setValidator(validatorLineEditInputFloat0to1);
}
