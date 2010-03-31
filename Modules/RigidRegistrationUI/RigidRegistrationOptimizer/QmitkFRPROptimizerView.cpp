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

#include "QmitkFRPROptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkFRPROptimizer.h>

QmitkFRPROptimizerView::QmitkFRPROptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkFRPROptimizerView::~QmitkFRPROptimizerView()
{
}

itk::Object::Pointer QmitkFRPROptimizerView::GetOptimizer()
{
  itk::FRPROptimizer::Pointer OptimizerPointer = itk::FRPROptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetStepLength(m_Controls.m_StepLengthFRPR->text().toFloat());
  if (m_Controls.m_FletchReevesFRPR->isChecked())
  {
    OptimizerPointer->SetToFletchReeves();
  }
  else
  {
    OptimizerPointer->SetToPolakRibiere();
  }
  OptimizerPointer->SetMaximumIteration( m_Controls.m_IterationsFRPR->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkFRPROptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(4);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_FletchReevesFRPR->isChecked();
  optimizerValues[2] = m_Controls.m_StepLengthFRPR->text().toFloat();
  optimizerValues[3] = m_Controls.m_IterationsFRPR->text().toInt();
  return optimizerValues;
}

void QmitkFRPROptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_FletchReevesFRPR->setChecked(optimizerValues[1]);
  m_Controls.m_PolakRibiereFRPR->setChecked(!optimizerValues[1]);
  m_Controls.m_StepLengthFRPR->setText(QString::number(optimizerValues[2]));
  m_Controls.m_IterationsFRPR->setText(QString::number(optimizerValues[3]));
}

void QmitkFRPROptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkFRPROptimizerView::GetName()
{
  return "FRPR";
}

void QmitkFRPROptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsFRPR->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_StepLengthFRPR->setValidator(validatorLineEditInputFloat);
}
