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

#include "QmitkCenteredSimilarity2DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkCenteredSimilarity2DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <qvalidator>

QmitkCenteredSimilarity2DTransformView::QmitkCenteredSimilarity2DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkCenteredSimilarity2DTransformView::~QmitkCenteredSimilarity2DTransformView()
{
}

itk::Object::Pointer QmitkCenteredSimilarity2DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkCenteredSimilarity2DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 2)
  {
    typedef typename itk::Image< TPixelType, 2 >  FixedImage2DType;
    typedef typename itk::Image< TPixelType, 2 >  MovingImage2DType;
    typename FixedImage2DType::Pointer fixedImage2D;
    mitk::CastToItkImage(m_FixedImage, fixedImage2D);
    typename MovingImage2DType::Pointer movingImage2D;
    mitk::CastToItkImage(m_MovingImage, movingImage2D);

    typename itk::CenteredSimilarity2DTransform< double >::Pointer transformPointer = itk::CenteredSimilarity2DTransform< double >::New();
    transformPointer->SetIdentity();
    if (m_Controls.m_CenterForInitializerCenteredSimilarity2D->isChecked())
    {
      typedef typename itk::CenteredSimilarity2DTransform< double >    CenteredSimilarity2DTransformType;
      typedef typename itk::CenteredTransformInitializer<CenteredSimilarity2DTransformType, FixedImage2DType, MovingImage2DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage2D );
      transformInitializer->SetMovingImage( movingImage2D );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsCenteredSimilarity2D->isChecked())
      {
        transformInitializer->MomentsOn();
      }
      else
      {
        transformInitializer->GeometryOn();
      }
      transformInitializer->InitializeTransform();        
    }
    transformPointer->SetScale( m_Controls.m_InitialScaleCenteredSimilarity2D->text().toFloat() );
    transformPointer->SetAngle( m_Controls.m_AngleCenteredSimilarity2D->text().toFloat() );
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return NULL;
}

itk::Array<double> QmitkCenteredSimilarity2DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(11);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesCenteredSimilarity2D->isChecked();
  transformValues[1] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->text().toDouble();
  transformValues[7] = m_Controls.m_InitialScaleCenteredSimilarity2D->text().toFloat();
  transformValues[8] = m_Controls.m_AngleCenteredSimilarity2D->text().toFloat();
  transformValues[9] = m_Controls.m_CenterForInitializerCenteredSimilarity2D->isChecked();
  transformValues[10] = m_Controls.m_MomentsCenteredSimilarity2D->isChecked();
  return transformValues;
}

void QmitkCenteredSimilarity2DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesCenteredSimilarity2D->setChecked(transformValues[0]);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
  m_Controls.m_InitialScaleCenteredSimilarity2D->setText(QString::number(transformValues[7]));
  m_Controls.m_AngleCenteredSimilarity2D->setText(QString::number(transformValues[8]));
  m_Controls.m_CenterForInitializerCenteredSimilarity2D->setChecked(transformValues[9]);
  m_Controls.m_MomentsCenteredSimilarity2D->setChecked(transformValues[10]);
  m_Controls.m_GeometryCenteredSimilarity2D->setChecked(!transformValues[10]);
}

QString QmitkCenteredSimilarity2DTransformView::GetName()
{
  return "CenteredSimilarity2D";
}

void QmitkCenteredSimilarity2DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkCenteredSimilarity2DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(6);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesCenteredSimilarity2D->isChecked())
  {
    scales[0] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesCenteredSimilarity2DTransformScale4->text().toDouble();
    scales[4] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationX->text().toDouble();
    scales[5] = m_Controls.m_ScalesCenteredSimilarity2DTransformScaleTranslationY->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkCenteredSimilarity2DTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    mitk::ScalarType angle = transformParams[1] * 45.0 / atan(1.0);
    vtktransform->PostMultiply();
    vtktransform->Translate(-transformParams[2], -transformParams[3], 0);
    vtktransform->Scale(transformParams[0], transformParams[0], 1);
    vtktransform->RotateZ(angle);
    vtktransform->Translate(transformParams[2], transformParams[3], 0);
    vtktransform->Translate(transformParams[4], transformParams[5], 0);
    vtktransform->PreMultiply();
  }
  return vtktransform;
}

int QmitkCenteredSimilarity2DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 6;
    else
      return 0;
  }
  else
    return 0;
}
