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

#include "QmitkSimilarity3DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkSimilarity3DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkSimilarity3DTransformView::QmitkSimilarity3DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkSimilarity3DTransformView::~QmitkSimilarity3DTransformView()
{
}

itk::Object::Pointer QmitkSimilarity3DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkSimilarity3DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 3)
  {
    typedef typename itk::Image< TPixelType, 3 >  FixedImage3DType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImage3DType;
    typename FixedImage3DType::Pointer fixedImage3D;
    mitk::CastToItkImage(m_FixedImage, fixedImage3D);
    typename MovingImage3DType::Pointer movingImage3D;
    mitk::CastToItkImage(m_MovingImage, movingImage3D);
    typename itk::Similarity3DTransform< double >::Pointer transformPointer = itk::Similarity3DTransform< double >::New();
    transformPointer->SetIdentity();
    if (m_Controls.m_CenterForInitializerSimilarity3D->isChecked())
    {
      typedef typename itk::Similarity3DTransform< double >    Similarity3DTransformType;
      typedef typename itk::CenteredTransformInitializer<Similarity3DTransformType, FixedImage3DType, MovingImage3DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage3D );
      transformInitializer->SetMovingImage( movingImage3D );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsSimilarity3D->isChecked())
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

itk::Array<double> QmitkSimilarity3DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(10);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesSimilarity3D->isChecked();
  transformValues[1] = m_Controls.m_ScalesSimilarity3DTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesSimilarity3DTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesSimilarity3DTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesSimilarity3DTransformScale4->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->text().toDouble();
  transformValues[7] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->text().toDouble();
  transformValues[8] = m_Controls.m_CenterForInitializerSimilarity3D->isChecked();
  transformValues[9] = m_Controls.m_MomentsSimilarity3D->isChecked();
  return transformValues;
}

void QmitkSimilarity3DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesSimilarity3D->setChecked(transformValues[0]);
  m_Controls.m_ScalesSimilarity3DTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesSimilarity3DTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesSimilarity3DTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesSimilarity3DTransformScale4->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->setText(QString::number(transformValues[6]));
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->setText(QString::number(transformValues[7]));
  m_Controls.m_CenterForInitializerSimilarity3D->setChecked(transformValues[8]);
  m_Controls.m_MomentsSimilarity3D->setChecked(transformValues[9]);
  m_Controls.m_GeometrySimilarity3D->setChecked(!transformValues[9]);
}

QString QmitkSimilarity3DTransformView::GetName()
{
  return "Similarity3D";
}

void QmitkSimilarity3DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesSimilarity3DTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkSimilarity3DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(7);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesSimilarity3D->isChecked())
  {
    scales[0] = m_Controls.m_ScalesSimilarity3DTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesSimilarity3DTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesSimilarity3DTransformScale3->text().toDouble();
    scales[3] = m_Controls.m_ScalesSimilarity3DTransformScale4->text().toDouble();
    scales[4] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationX->text().toDouble();
    scales[5] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationY->text().toDouble();
    scales[6] = m_Controls.m_ScalesSimilarity3DTransformScaleTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkSimilarity3DTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    itk::Similarity3DTransform<double>::Pointer similarityTransform = itk::Similarity3DTransform<double>::New();
    similarityTransform->SetParameters(transformParams);
    itk::Matrix<double, 3, 3> Matrix = similarityTransform->GetMatrix();
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

int QmitkSimilarity3DTransformView::GetNumberOfTransformParameters()
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
