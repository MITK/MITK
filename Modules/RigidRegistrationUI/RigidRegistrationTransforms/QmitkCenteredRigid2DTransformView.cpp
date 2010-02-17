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

#include "QmitkCenteredRigid2DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkCenteredRigid2DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkCenteredRigid2DTransformView::QmitkCenteredRigid2DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkCenteredRigid2DTransformView::~QmitkCenteredRigid2DTransformView()
{
}

itk::Object::Pointer QmitkCenteredRigid2DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkCenteredRigid2DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* /* itkImage1 */)
{
  if (VImageDimension == 2)
  {
    typedef typename itk::Image< TPixelType, 2 >  FixedImage2DType;
    typedef typename itk::Image< TPixelType, 2 >  MovingImage2DType;
    typename FixedImage2DType::Pointer fixedImage2D;
    mitk::CastToItkImage(m_FixedImage, fixedImage2D);
    typename MovingImage2DType::Pointer movingImage2D;
    mitk::CastToItkImage(m_MovingImage, movingImage2D);
    typename itk::CenteredRigid2DTransform< double >::Pointer transformPointer = itk::CenteredRigid2DTransform< double >::New();
    transformPointer->SetIdentity();
    if (m_Controls.m_CenterForInitializerCenteredRigid2D->isChecked())
    {
      typedef typename itk::CenteredRigid2DTransform< double >    CenteredRigid2DTransformType;
      typedef typename itk::CenteredTransformInitializer<CenteredRigid2DTransformType, FixedImage2DType, MovingImage2DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage2D );
      transformInitializer->SetMovingImage( movingImage2D );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsCenteredRigid2D->isChecked())
      {
        transformInitializer->MomentsOn();
      }
      else
      {
        transformInitializer->GeometryOn();
      }
      transformInitializer->InitializeTransform();
    }
    transformPointer->SetAngle( m_Controls.m_AngleCenteredRigid2D->text().toFloat() );
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return NULL;
}

itk::Array<double> QmitkCenteredRigid2DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(9);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesCenteredRigid2D->isChecked();
  transformValues[1] = m_Controls.m_RotationScaleCenteredRigid2D->text().toDouble();
  transformValues[2] = m_Controls.m_CenterXScaleCenteredRigid2D->text().toDouble();
  transformValues[3] = m_Controls.m_CenterYScaleCenteredRigid2D->text().toDouble();
  transformValues[4] = m_Controls.m_TranslationXScaleCenteredRigid2D->text().toDouble();
  transformValues[5] = m_Controls.m_TranslationYScaleCenteredRigid2D->text().toDouble();
  transformValues[6] = m_Controls.m_AngleCenteredRigid2D->text().toFloat();
  transformValues[7] = m_Controls.m_CenterForInitializerCenteredRigid2D->isChecked();
  transformValues[8] = m_Controls.m_MomentsCenteredRigid2D->isChecked();
  return transformValues;
}

void QmitkCenteredRigid2DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesCenteredRigid2D->setChecked(transformValues[0]);
  m_Controls.m_RotationScaleCenteredRigid2D->setText(QString::number(transformValues[1]));
  m_Controls.m_CenterXScaleCenteredRigid2D->setText(QString::number(transformValues[2]));
  m_Controls.m_CenterYScaleCenteredRigid2D->setText(QString::number(transformValues[3]));
  m_Controls.m_TranslationXScaleCenteredRigid2D->setText(QString::number(transformValues[4]));
  m_Controls.m_TranslationYScaleCenteredRigid2D->setText(QString::number(transformValues[5]));
  m_Controls.m_AngleCenteredRigid2D->setText(QString::number(transformValues[6]));
  m_Controls.m_CenterForInitializerCenteredRigid2D->setChecked(transformValues[7]);
  m_Controls.m_MomentsCenteredRigid2D->setChecked(transformValues[8]);
  m_Controls.m_GeometryCenteredRigid2D->setChecked(!transformValues[8]);
}

QString QmitkCenteredRigid2DTransformView::GetName()
{
  return "CenteredRigid2D";
}

void QmitkCenteredRigid2DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_AngleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_RotationScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_CenterXScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_CenterYScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationXScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationYScaleCenteredRigid2D->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkCenteredRigid2DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(5);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesCenteredRigid2D->isChecked())
  {
    scales[0] = m_Controls.m_RotationScaleCenteredRigid2D->text().toDouble();
    scales[1] = m_Controls.m_CenterXScaleCenteredRigid2D->text().toDouble();
    scales[2] = m_Controls.m_CenterYScaleCenteredRigid2D->text().toDouble();
    scales[3] = m_Controls.m_TranslationXScaleCenteredRigid2D->text().toDouble();
    scales[4] = m_Controls.m_TranslationYScaleCenteredRigid2D->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkCenteredRigid2DTransformView::Transform(vtkMatrix4x4* /* vtkmatrix */, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    mitk::ScalarType angle = transformParams[0] * 45.0 / atan(1.0);;
    vtktransform->PostMultiply();
    vtktransform->Translate(-transformParams[1], -transformParams[2], 0);
    vtktransform->RotateZ(angle);
    vtktransform->Translate(transformParams[1], transformParams[2], 0);
    vtktransform->Translate(transformParams[3], transformParams[4], 0);
    vtktransform->PreMultiply();
  }
  return vtktransform;
}

int QmitkCenteredRigid2DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 5;
    else
      return 0;
  }
  else
    return 0;
}
