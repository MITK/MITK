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

#include "QmitkScaleSkewVersor3DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkScaleSkewVersor3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <qvalidator>

QmitkScaleSkewVersor3DTransformView::QmitkScaleSkewVersor3DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkScaleSkewVersor3DTransformView::~QmitkScaleSkewVersor3DTransformView()
{
}

itk::Object::Pointer QmitkScaleSkewVersor3DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkScaleSkewVersor3DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 3)
  {
    typedef typename itk::Image< TPixelType, 3 >  FixedImage3DType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImage3DType;
    typename FixedImage3DType::Pointer fixedImage3D;
    mitk::CastToItkImage(m_FixedImage, fixedImage3D);
    typename MovingImage3DType::Pointer movingImage3D;
    mitk::CastToItkImage(m_MovingImage, movingImage3D);
    typename itk::ScaleSkewVersor3DTransform< double >::Pointer transformPointer = itk::ScaleSkewVersor3DTransform< double >::New();
    transformPointer->SetIdentity();
    if (m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->isChecked())
    {
      typedef typename itk::ScaleSkewVersor3DTransform< double >    ScaleSkewVersor3DTransformType;
      typedef typename itk::CenteredTransformInitializer<ScaleSkewVersor3DTransformType, FixedImage3DType, MovingImage3DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage3D );
      transformInitializer->SetMovingImage( movingImage3D );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsScaleSkewVersorRigid3D->isChecked())
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
  return NULL;
}

itk::Array<double> QmitkScaleSkewVersor3DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(18);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesScaleSkewVersorRigid3DTransform->isChecked();
  transformValues[1] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->text().toDouble();
  transformValues[5] = m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->text().toDouble();
  transformValues[7] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->text().toDouble();
  transformValues[8] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->text().toDouble();
  transformValues[9] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->text().toDouble();
  transformValues[10] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->text().toDouble();
  transformValues[11] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->text().toDouble();
  transformValues[12] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->text().toDouble();
  transformValues[13] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->text().toDouble();
  transformValues[14] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->text().toDouble();
  transformValues[15] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->text().toDouble();
  transformValues[16] = m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->isChecked();
  transformValues[17] = m_Controls.m_MomentsScaleSkewVersorRigid3D->isChecked();
  return transformValues;
}

void QmitkScaleSkewVersor3DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesScaleSkewVersorRigid3DTransform->setChecked(transformValues[0]);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[4]));
  m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[6]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->setText(QString::number(transformValues[7]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->setText(QString::number(transformValues[8]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->setText(QString::number(transformValues[9]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->setText(QString::number(transformValues[10]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->setText(QString::number(transformValues[11]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->setText(QString::number(transformValues[12]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->setText(QString::number(transformValues[13]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->setText(QString::number(transformValues[14]));
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->setText(QString::number(transformValues[15]));
  m_Controls.m_CenterForInitializerScaleSkewVersorRigid3D->setChecked(transformValues[16]);
  m_Controls.m_MomentsScaleSkewVersorRigid3D->setChecked(transformValues[17]);
  m_Controls.m_GeometryScaleSkewVersorRigid3D->setChecked(!transformValues[17]);
}

QString QmitkScaleSkewVersor3DTransformView::GetName()
{
  return "ScaleSkewVersor3D";
}

void QmitkScaleSkewVersor3DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkScaleSkewVersor3DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(15);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesScaleSkewVersorRigid3DTransform->isChecked())
  {
    scales[0] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationX->text().toDouble();
    scales[4] = m_Controls.m_ScaleScaleSkewVersorRigid3DTransformScaleTranslationY->text().toDouble();
    scales[5] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScaleTranslationZ->text().toDouble();
    scales[6] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale7->text().toDouble();
    scales[7] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale8->text().toDouble();
    scales[8] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale9->text().toDouble();
    scales[9] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale10->text().toDouble();
    scales[10] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale11->text().toDouble();
    scales[11] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale12->text().toDouble();
    scales[12] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale13->text().toDouble();
    scales[13] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale14->text().toDouble();
    scales[14] = m_Controls.m_ScalesScaleSkewVersorRigid3DTransformScale15->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkScaleSkewVersor3DTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    itk::ScaleSkewVersor3DTransform<double>::Pointer versorTransform = itk::ScaleSkewVersor3DTransform<double>::New();
    versorTransform->SetParameters(transformParams);
    itk::Matrix<double, 3, 3> Matrix = versorTransform->GetMatrix();
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
    vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[3]);
    vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[4]);
    vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[5]);
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkScaleSkewVersor3DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 0;
    else
      return 15;
  }
  else
    return 0;
}
