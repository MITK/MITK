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

#include "QmitkVersorRigid3DTransformOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkVersorRigid3DTransformOptimizer.h>

QmitkVersorRigid3DTransformOptimizerView::QmitkVersorRigid3DTransformOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkVersorRigid3DTransformOptimizerView::~QmitkVersorRigid3DTransformOptimizerView()
{
}

itk::Object::Pointer QmitkVersorRigid3DTransformOptimizerView::GetOptimizer()
{
  itk::VersorRigid3DTransformOptimizer::Pointer OptimizerPointer = itk::VersorRigid3DTransformOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetGradientMagnitudeTolerance( m_Controls.m_GradientMagnitudeToleranceVersorRigid3DTransform->text().toFloat() );
  OptimizerPointer->SetMinimumStepLength( m_Controls.m_MinimumStepLengthVersorRigid3DTransform->text().toFloat() );
  OptimizerPointer->SetMaximumStepLength( m_Controls.m_MaximumStepLengthVersorRigid3DTransform->text().toFloat() );
  OptimizerPointer->SetNumberOfIterations( m_Controls.m_IterationsVersorRigid3DTransform->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkVersorRigid3DTransformOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(5);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_GradientMagnitudeToleranceVersorRigid3DTransform->text().toFloat();
  optimizerValues[2] = m_Controls.m_MinimumStepLengthVersorRigid3DTransform->text().toFloat();
  optimizerValues[3] = m_Controls.m_MaximumStepLengthVersorRigid3DTransform->text().toFloat();
  optimizerValues[4] = m_Controls.m_IterationsVersorRigid3DTransform->text().toInt();
  return optimizerValues;
}

void QmitkVersorRigid3DTransformOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_GradientMagnitudeToleranceVersorRigid3DTransform->setText(QString::number(optimizerValues[1]));
  m_Controls.m_MinimumStepLengthVersorRigid3DTransform->setText(QString::number(optimizerValues[2]));
  m_Controls.m_MaximumStepLengthVersorRigid3DTransform->setText(QString::number(optimizerValues[3]));
  m_Controls.m_IterationsVersorRigid3DTransform->setText(QString::number(optimizerValues[4]));
}

void QmitkVersorRigid3DTransformOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkVersorRigid3DTransformOptimizerView::GetName()
{
  return "VersorRigid3DTransform";
}

void QmitkVersorRigid3DTransformOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsVersorRigid3DTransform->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_GradientMagnitudeToleranceVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MinimumStepLengthVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MaximumStepLengthVersorRigid3DTransform->setValidator(validatorLineEditInputFloat);
}
