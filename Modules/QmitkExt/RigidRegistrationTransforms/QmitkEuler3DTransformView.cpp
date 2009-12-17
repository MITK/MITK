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

#include "QmitkEuler3DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkEuler3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <qvalidator>

QmitkEuler3DTransformView::QmitkEuler3DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkEuler3DTransformView::~QmitkEuler3DTransformView()
{
}

itk::Object::Pointer QmitkEuler3DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkEuler3DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 3)
  {
    typename itk::Euler3DTransform< double >::Pointer transformPointer = itk::Euler3DTransform< double >::New();
    transformPointer->SetIdentity();
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_CenterZ = transformPointer->GetCenter()[2];
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return NULL;
}

itk::Array<double> QmitkEuler3DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(9);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesEuler3D->isChecked();
  transformValues[1] = m_Controls.m_ScalesEuler3DTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesEuler3DTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesEuler3DTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesEuler3DTransformScaleTranslationX->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesEuler3DTransformScaleTranslationY->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->text().toDouble();
  transformValues[7] = m_Controls.m_CenterForInitializerEuler3D->isChecked();
  transformValues[8] = m_Controls.m_MomentsEuler3D->isChecked();
  return transformValues;
}

void QmitkEuler3DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesEuler3D->setChecked(transformValues[0]);
  m_Controls.m_ScalesEuler3DTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesEuler3DTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesEuler3DTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesEuler3DTransformScaleTranslationX->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesEuler3DTransformScaleTranslationY->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->setText(QString::number(transformValues[6]));
  m_Controls.m_CenterForInitializerEuler3D->setChecked(transformValues[7]);
  m_Controls.m_MomentsEuler3D->setChecked(transformValues[8]);
  m_Controls.m_GeometryEuler3D->setChecked(!transformValues[8]);
}

QString QmitkEuler3DTransformView::GetName()
{
  return "Euler3D";
}

void QmitkEuler3DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesEuler3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkEuler3DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(6);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesEuler3D->isChecked())
  {
    scales[0] = m_Controls.m_ScalesEuler3DTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesEuler3DTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesEuler3DTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesEuler3DTransformScaleTranslationX->text().toDouble();
    scales[4] = m_Controls.m_ScalesEuler3DTransformScaleTranslationY->text().toDouble();
    scales[5] = m_Controls.m_ScalesEuler3DTransformScaleTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkEuler3DTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    mitk::ScalarType angleX = transformParams[0] * 45.0 / atan(1.0);
    mitk::ScalarType angleY = transformParams[1] * 45.0 / atan(1.0);
    mitk::ScalarType angleZ = transformParams[2] * 45.0 / atan(1.0);
    vtktransform->PostMultiply();
    vtktransform->Translate(-m_CenterX, -m_CenterY, -m_CenterZ);
    vtktransform->RotateX(angleX);
    vtktransform->RotateY(angleY);
    vtktransform->RotateZ(angleZ);
    vtktransform->Translate(m_CenterX, m_CenterY, m_CenterZ);
    vtktransform->Translate(transformParams[3], transformParams[4], transformParams[5]);
    vtktransform->PreMultiply();
  }
  return vtktransform;
}

int QmitkEuler3DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 4;
    else
      return 6;
  }
  else
    return 0;
}
