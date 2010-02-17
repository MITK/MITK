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

#include "QmitkCenteredEuler3DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkCenteredEuler3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkCenteredEuler3DTransformView::QmitkCenteredEuler3DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkCenteredEuler3DTransformView::~QmitkCenteredEuler3DTransformView()
{
}

itk::Object::Pointer QmitkCenteredEuler3DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkCenteredEuler3DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  if (VImageDimension == 3)
  {
    typename itk::CenteredEuler3DTransform< double >::Pointer transformPointer = itk::CenteredEuler3DTransform< double >::New();
    transformPointer->SetIdentity();
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_CenterZ = transformPointer->GetCenter()[2];
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return NULL;
}

itk::Array<double> QmitkCenteredEuler3DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(9);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesCenteredEuler3D->isChecked();
  transformValues[1] = m_Controls.m_ScalesCenteredEuler3DTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesCenteredEuler3DTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesCenteredEuler3DTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->text().toDouble();
  transformValues[7] = m_Controls.m_CenterForInitializerCenteredEuler3D->isChecked();
  transformValues[8] = m_Controls.m_MomentsCenteredEuler3D->isChecked();
  return transformValues;
}

void QmitkCenteredEuler3DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesCenteredEuler3D->setChecked(transformValues[0]);
  m_Controls.m_ScalesCenteredEuler3DTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesCenteredEuler3DTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesCenteredEuler3DTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->setText(QString::number(transformValues[6]));
  m_Controls.m_CenterForInitializerCenteredEuler3D->setChecked(transformValues[7]);
  m_Controls.m_MomentsCenteredEuler3D->setChecked(transformValues[8]);
  m_Controls.m_GeometryCenteredEuler3D->setChecked(!transformValues[8]);
}

QString QmitkCenteredEuler3DTransformView::GetName()
{
  return "CenteredEuler3D";
}

void QmitkCenteredEuler3DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesCenteredEuler3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkCenteredEuler3DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(6);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesCenteredEuler3D->isChecked())
  {
    scales[0] = m_Controls.m_ScalesCenteredEuler3DTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesCenteredEuler3DTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesCenteredEuler3DTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationX->text().toDouble();
    scales[4] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationY->text().toDouble();
    scales[5] = m_Controls.m_ScalesCenteredEuler3DTransformScaleTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkCenteredEuler3DTransformView::Transform(vtkMatrix4x4* /*vtkmatrix*/, vtkTransform* vtktransform, itk::Array<double> transformParams)
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

int QmitkCenteredEuler3DTransformView::GetNumberOfTransformParameters()
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
