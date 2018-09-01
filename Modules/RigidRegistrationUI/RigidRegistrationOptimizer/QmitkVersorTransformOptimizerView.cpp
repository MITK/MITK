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

#include "QmitkVersorTransformOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkVersorTransformOptimizer.h>

#include <QIntValidator>
#include <QDoubleValidator>

QmitkVersorTransformOptimizerView::QmitkVersorTransformOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkVersorTransformOptimizerView::~QmitkVersorTransformOptimizerView()
{
}

mitk::OptimizerParameters::OptimizerType QmitkVersorTransformOptimizerView::GetOptimizerType()
{
  return mitk::OptimizerParameters::VERSORTRANSFORMOPTIMIZER;
}

itk::Object::Pointer QmitkVersorTransformOptimizerView::GetOptimizer()
{
  itk::VersorTransformOptimizer::Pointer OptimizerPointer = itk::VersorTransformOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetGradientMagnitudeTolerance( m_Controls.m_GradientMagnitudeToleranceVersorTransform->text().toFloat() );
  OptimizerPointer->SetMinimumStepLength( m_Controls.m_MinimumStepLengthVersorTransform->text().toFloat() );
  OptimizerPointer->SetMaximumStepLength( m_Controls.m_MaximumStepLengthVersorTransform->text().toFloat() );
  OptimizerPointer->SetNumberOfIterations( m_Controls.m_IterationsVersorTransform->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkVersorTransformOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(5);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_GradientMagnitudeToleranceVersorTransform->text().toFloat();
  optimizerValues[2] = m_Controls.m_MinimumStepLengthVersorTransform->text().toFloat();
  optimizerValues[3] = m_Controls.m_MaximumStepLengthVersorTransform->text().toFloat();
  optimizerValues[4] = m_Controls.m_IterationsVersorTransform->text().toInt();
  return optimizerValues;
}

void QmitkVersorTransformOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_GradientMagnitudeToleranceVersorTransform->setText(QString::number(optimizerValues[1]));
  m_Controls.m_MinimumStepLengthVersorTransform->setText(QString::number(optimizerValues[2]));
  m_Controls.m_MaximumStepLengthVersorTransform->setText(QString::number(optimizerValues[3]));
  m_Controls.m_IterationsVersorTransform->setText(QString::number(optimizerValues[4]));
}

void QmitkVersorTransformOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkVersorTransformOptimizerView::GetName()
{
  return "VersorTransform";
}

void QmitkVersorTransformOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsVersorTransform->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_GradientMagnitudeToleranceVersorTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MinimumStepLengthVersorTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MaximumStepLengthVersorTransform->setValidator(validatorLineEditInputFloat);
}
