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

#include "QmitkFixedCenterOfRotationAffineTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkFixedCenterOfRotationAffineTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkFixedCenterOfRotationAffineTransformView::QmitkFixedCenterOfRotationAffineTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkFixedCenterOfRotationAffineTransformView::~QmitkFixedCenterOfRotationAffineTransformView()
{
}

itk::Object::Pointer QmitkFixedCenterOfRotationAffineTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkFixedCenterOfRotationAffineTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename FixedImageType::Pointer fixedImage;
  mitk::CastToItkImage(m_FixedImage, fixedImage);
  typename MovingImageType::Pointer movingImage;
  mitk::CastToItkImage(m_MovingImage, movingImage);
  typedef typename itk::FixedCenterOfRotationAffineTransform< double, VImageDimension >    CenteredAffineTransformType;
  typename itk::FixedCenterOfRotationAffineTransform< double, VImageDimension>::Pointer transformPointer = itk::FixedCenterOfRotationAffineTransform< double, VImageDimension>::New();
  transformPointer->SetIdentity();
  if (m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->isChecked())
  {
    typedef typename itk::FixedCenterOfRotationAffineTransform< double, VImageDimension >    FixedCenterOfRotationAffineTransformType;
    typedef typename itk::CenteredTransformInitializer<FixedCenterOfRotationAffineTransformType, FixedImageType, MovingImageType> TransformInitializerType;
    typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
    transformInitializer->SetFixedImage( fixedImage );
    transformInitializer->SetMovingImage( movingImage );
    transformInitializer->SetTransform( transformPointer );
    if (m_Controls.m_MomentsFixedCenterOfRotationAffine->isChecked())
    {
      transformInitializer->MomentsOn();
    }
    else
    {
      transformInitializer->GeometryOn();
    }
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_CenterZ = transformPointer->GetCenter()[2];
    transformInitializer->InitializeTransform();        
  }
  m_TransformObject = transformPointer.GetPointer();
  return transformPointer.GetPointer();
}

itk::Array<double> QmitkFixedCenterOfRotationAffineTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(15);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked();
  transformValues[1] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->text().toDouble();
  transformValues[7] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->text().toDouble();
  transformValues[8] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->text().toDouble();
  transformValues[9] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->text().toDouble();
  transformValues[10] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->text().toDouble();
  transformValues[11] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->text().toDouble();
  transformValues[12] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->text().toDouble();
  transformValues[13] = m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->isChecked();
  transformValues[14] = m_Controls.m_MomentsFixedCenterOfRotationAffine->isChecked();
  return transformValues;
}

void QmitkFixedCenterOfRotationAffineTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->setChecked(transformValues[0]);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->setText(QString::number(transformValues[6]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->setText(QString::number(transformValues[7]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->setText(QString::number(transformValues[8]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->setText(QString::number(transformValues[9]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->setText(QString::number(transformValues[10]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->setText(QString::number(transformValues[11]));
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->setText(QString::number(transformValues[12]));
  m_Controls.m_CenterForInitializerFixedCenterOfRotationAffine->setChecked(transformValues[13]);
  m_Controls.m_MomentsFixedCenterOfRotationAffine->setChecked(transformValues[14]);
  m_Controls.m_GeometryFixedCenterOfRotationAffine->setChecked(!transformValues[14]);
}

QString QmitkFixedCenterOfRotationAffineTransformView::GetName()
{
  return "FixedCenterOfRotationAffine";
}

void QmitkFixedCenterOfRotationAffineTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkFixedCenterOfRotationAffineTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(12);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesFixedCenterOfRotationAffine->isChecked())
  {
    scales[0] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale4->text().toDouble();
    scales[4] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale5->text().toDouble();
    scales[5] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale6->text().toDouble();
    scales[6] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->text().toDouble();
    scales[7] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->text().toDouble();
    scales[8] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->text().toDouble();
    scales[9] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->text().toDouble();
    scales[10] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->text().toDouble();
    scales[11] = m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkFixedCenterOfRotationAffineTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    int m = 0;
    for (unsigned int i = 0; i < m_FixedImage->GetDimension(); i++)
    {
      for (unsigned int j = 0; j < m_FixedImage->GetDimension(); j++)
      {
        vtkmatrix->SetElement(i, j, transformParams[m]);
        m++;
      }
    }
    float center[4];
    float translation[4];
    center[0] = m_CenterX;
    center[1] = m_CenterY;
    center[2] = m_CenterZ;
    center[3] = 1;
    vtkmatrix->MultiplyPoint(center, translation);
    if (m_FixedImage->GetDimension() == 2)
    {
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[4]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[5]);
    }
    else if (m_FixedImage->GetDimension() == 3)
    {
      vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[9]);
      vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[10]);
      vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[11]);
    }
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkFixedCenterOfRotationAffineTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
    {
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->hide();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->hide();
      m_Controls.textLabel2_7_2_2->setText("Translation Scale X:");
      m_Controls.textLabel3_6_2_2->setText("Translation Scale Y:");
      m_Controls.textLabel4_4_3_2->hide();
      m_Controls.textLabel5_4_2_2->hide();
      m_Controls.textLabel6_4_2_2->hide();
      m_Controls.textLabel11_3_2_2->hide();
      m_Controls.textLabel12_3_2_2->hide();
      m_Controls.textLabel13_2_2_2->hide();
      return 6;
    }
    else
    {
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale7->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale8->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScale9->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationX->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationY->show();
      m_Controls.m_ScalesFixedCenterOfRotationAffineTransformScaleTranslationZ->show();
      m_Controls.textLabel2_7_2_2->setText("Scale 5:");
      m_Controls.textLabel3_6_2_2->setText("Scale 6:");
      m_Controls.textLabel4_4_3_2->show();
      m_Controls.textLabel5_4_2_2->show();
      m_Controls.textLabel6_4_2_2->show();
      m_Controls.textLabel11_3_2_2->show();
      m_Controls.textLabel12_3_2_2->show();
      m_Controls.textLabel13_2_2_2->show();
      return 12;
    }
  }
  else 
    return 0;
}
