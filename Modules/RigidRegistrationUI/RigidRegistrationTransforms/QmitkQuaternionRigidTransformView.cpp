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

#include "QmitkQuaternionRigidTransformView.h"
#include "mitkImageAccessByItk.h"
#include <mitkImageCast.h>
#include <itkQuaternionRigidTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkQuaternionRigidTransformView::QmitkQuaternionRigidTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f),
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkQuaternionRigidTransformView::~QmitkQuaternionRigidTransformView()
{
}

mitk::TransformParameters::TransformType QmitkQuaternionRigidTransformView::GetTransformType()
{
  return mitk::TransformParameters::QUATERNIONRIGIDTRANSFORM;
}

itk::Object::Pointer QmitkQuaternionRigidTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessFixedDimensionByItk(m_FixedImage, GetTransform2, 3);
    return m_TransformObject;
  }
  return nullptr;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkQuaternionRigidTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 3)
  {
    typedef typename itk::Image< TPixelType, 3 >  FixedImageType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImageType;

    // the fixedImage is the input parameter (fix for Bug #14626)
    typename FixedImageType::Pointer fixedImage = itkImage1;

    // the movingImage type is known, use the ImageToItk filter (fix for Bug #14626)
    typename mitk::ImageToItk<MovingImageType>::Pointer movingImageToItk = mitk::ImageToItk<MovingImageType>::New();
    movingImageToItk->SetInput(m_MovingImage);
    movingImageToItk->Update();
    typename MovingImageType::Pointer movingImage = movingImageToItk->GetOutput();

    typename itk::QuaternionRigidTransform< double >::Pointer transformPointer = itk::QuaternionRigidTransform< double >::New();
    transformPointer->SetIdentity();
    typedef typename itk::QuaternionRigidTransform< double >    QuaternionRigidTransformType;
    if (m_Controls.m_CenterForInitializerQuaternionRigid->isChecked())
    {
      typedef typename itk::CenteredTransformInitializer<QuaternionRigidTransformType, FixedImageType, MovingImageType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage );
      transformInitializer->SetMovingImage( movingImage );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsQuaternionRigid->isChecked())
      {
        transformInitializer->MomentsOn();
      }
      else
      {
        transformInitializer->GeometryOn();
      }
      transformInitializer->InitializeTransform();
    }
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_CenterZ = transformPointer->GetCenter()[2];
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return nullptr;
}

itk::Array<double> QmitkQuaternionRigidTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(10);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesQuaternionRigid->isChecked();
  transformValues[1] = m_Controls.m_ScalesQuaternionRigidTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesQuaternionRigidTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesQuaternionRigidTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesQuaternionRigidTransformScale4->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->text().toDouble();
  transformValues[7] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->text().toDouble();
  transformValues[8] = m_Controls.m_CenterForInitializerQuaternionRigid->isChecked();
  transformValues[9] = m_Controls.m_MomentsQuaternionRigid->isChecked();
  return transformValues;
}

void QmitkQuaternionRigidTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesQuaternionRigid->setChecked(transformValues[0]);
  m_Controls.m_ScalesQuaternionRigidTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesQuaternionRigidTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesQuaternionRigidTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesQuaternionRigidTransformScale4->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->setText(QString::number(transformValues[6]));
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
  m_Controls.m_CenterForInitializerQuaternionRigid->setChecked(transformValues[8]);
  m_Controls.m_MomentsQuaternionRigid->setChecked(transformValues[9]);
  m_Controls.m_GeometryQuaternionRigid->setChecked(!transformValues[9]);
}

QString QmitkQuaternionRigidTransformView::GetName()
{
  return "QuaternionRigid";
}

void QmitkQuaternionRigidTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesQuaternionRigidTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkQuaternionRigidTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(7);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesQuaternionRigid->isChecked())
  {
    scales[0] = m_Controls.m_ScalesQuaternionRigidTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesQuaternionRigidTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesQuaternionRigidTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesQuaternionRigidTransformScale4->text().toDouble();
    scales[4] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationX->text().toDouble();
    scales[5] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationY->text().toDouble();
    scales[6] = m_Controls.m_ScalesQuaternionRigidTransformScaleTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkQuaternionRigidTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    itk::QuaternionRigidTransform<double>::Pointer quaternionTransform = itk::QuaternionRigidTransform<double>::New();
    quaternionTransform->SetParameters(transformParams);
    itk::Matrix<double, 3, 3> Matrix = quaternionTransform->GetMatrix();
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
      {
        vtkmatrix->SetElement(i, j, Matrix[i][j]);
      }
    }
    float center[4];
    float translation[4];
    center[0] = m_CenterX;
    center[1] = m_CenterY;
    center[2] = m_CenterZ;
    center[3] = 1;
    vtkmatrix->MultiplyPoint(center, translation);
    vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[4]);
    vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[5]);
    vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[6]);
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkQuaternionRigidTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 0;
    else
      return 7;
  }
  else
    return 0;
}
