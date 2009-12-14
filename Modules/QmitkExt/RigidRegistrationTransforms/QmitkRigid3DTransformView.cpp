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

#include "QmitkRigid3DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkRigid3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkRigid3DTransformView::QmitkRigid3DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkRigid3DTransformView::~QmitkRigid3DTransformView()
{
}

itk::Object::Pointer QmitkRigid3DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkRigid3DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  if (VImageDimension == 3)
  {
    typename itk::Rigid3DTransform< double >::Pointer transformPointer = itk::Rigid3DTransform< double >::New();
    transformPointer->SetIdentity();
    m_CenterX = transformPointer->GetCenter()[0];
    m_CenterY = transformPointer->GetCenter()[1];
    m_CenterZ = transformPointer->GetCenter()[2];
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return NULL;
}

itk::Array<double> QmitkRigid3DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(15);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesRigid3D->isChecked();
  transformValues[1] = m_Controls.m_ScalesRigid3DTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesRigid3DTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesRigid3DTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesRigid3DTransformScale4->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesRigid3DTransformScale5->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesRigid3DTransformScale6->text().toDouble();
  transformValues[7] = m_Controls.m_ScalesRigid3DTransformScale7->text().toDouble();
  transformValues[8] = m_Controls.m_ScalesRigid3DTransformScale8->text().toDouble();
  transformValues[9] = m_Controls.m_ScalesRigid3DTransformScale9->text().toDouble();
  transformValues[10] = m_Controls.m_ScalesRigid3DTransformScaleTranslationX->text().toDouble();
  transformValues[11] = m_Controls.m_ScalesRigid3DTransformScaleTranslationY->text().toDouble();
  transformValues[12] = m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->text().toDouble();
  transformValues[13] = m_Controls.m_CenterForInitializerRigid3D->isChecked();
  transformValues[14] = m_Controls.m_MomentsRigid3D->isChecked();
  return transformValues;
}

void QmitkRigid3DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesRigid3D->setChecked(transformValues[0]);
  m_Controls.m_ScalesRigid3DTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesRigid3DTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesRigid3DTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesRigid3DTransformScale4->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesRigid3DTransformScale5->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesRigid3DTransformScale6->setText(QString::number(transformValues[6]));
  m_Controls.m_ScalesRigid3DTransformScale7->setText(QString::number(transformValues[7]));
  m_Controls.m_ScalesRigid3DTransformScale8->setText(QString::number(transformValues[8]));
  m_Controls.m_ScalesRigid3DTransformScale9->setText(QString::number(transformValues[9]));
  m_Controls.m_ScalesRigid3DTransformScaleTranslationX->setText(QString::number(transformValues[10]));
  m_Controls.m_ScalesRigid3DTransformScaleTranslationY->setText(QString::number(transformValues[11]));
  m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->setText(QString::number(transformValues[12]));
  m_Controls.m_CenterForInitializerRigid3D->setChecked(transformValues[13]);
  m_Controls.m_MomentsRigid3D->setChecked(transformValues[14]);
  m_Controls.m_GeometryRigid3D->setChecked(!transformValues[14]);
}

QString QmitkRigid3DTransformView::GetName()
{
  return "Rigid3D";
}

void QmitkRigid3DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesRigid3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkRigid3DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(12);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesRigid3D->isChecked())
  {
    scales[0] = m_Controls.m_ScalesRigid3DTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesRigid3DTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesRigid3DTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesRigid3DTransformScale4->text().toDouble();
    scales[4] = m_Controls.m_ScalesRigid3DTransformScale5->text().toDouble();
    scales[5] = m_Controls.m_ScalesRigid3DTransformScale6->text().toDouble();
    scales[6] = m_Controls.m_ScalesRigid3DTransformScale7->text().toDouble();
    scales[7] = m_Controls.m_ScalesRigid3DTransformScale8->text().toDouble();
    scales[8] = m_Controls.m_ScalesRigid3DTransformScale9->text().toDouble();
    scales[9] = m_Controls.m_ScalesRigid3DTransformScaleTranslationX->text().toDouble();
    scales[10] = m_Controls.m_ScalesRigid3DTransformScaleTranslationY->text().toDouble();
    scales[11] = m_Controls.m_ScalesRigid3DTransformScaleTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkRigid3DTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    int m = 0;
    for (int i = 0; i < 3; i++)
    {
      for (int j = 0; j < 3; j++)
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
    vtkmatrix->SetElement(0, 3, -translation[0] + center[0] + transformParams[9]);
    vtkmatrix->SetElement(1, 3, -translation[1] + center[1] + transformParams[10]);
    vtkmatrix->SetElement(2, 3, -translation[2] + center[2] + transformParams[11]);
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkRigid3DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 6;
    else
      return 12;
  }
  else
    return 0;
}
