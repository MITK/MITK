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

#include "QmitkAffineTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkAffineTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkAffineTransformView::QmitkAffineTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkAffineTransformView::~QmitkAffineTransformView()
{
}

itk::Object::Pointer QmitkAffineTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkAffineTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename FixedImageType::Pointer fixedImage;
  mitk::CastToItkImage(m_FixedImage, fixedImage);
  typename MovingImageType::Pointer movingImage;
  mitk::CastToItkImage(m_MovingImage, movingImage);
  typename itk::AffineTransform< double, VImageDimension>::Pointer transformPointer = itk::AffineTransform< double, VImageDimension>::New();    
  transformPointer->SetIdentity();
  if (m_Controls.m_CenterForInitializerAffine->isChecked())
  {
    typedef typename itk::AffineTransform< double, VImageDimension >    AffineTransformType;
    typedef typename itk::CenteredTransformInitializer<AffineTransformType, FixedImageType, MovingImageType> TransformInitializerType;
    typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
    transformInitializer->SetFixedImage( fixedImage );
    transformInitializer->SetMovingImage( movingImage );
    transformInitializer->SetTransform( transformPointer );
    if (m_Controls.m_MomentsAffine->isChecked())
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

itk::Array<double> QmitkAffineTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(15);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesAffine->isChecked();
  transformValues[1] = m_Controls.m_ScalesAffineTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesAffineTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesAffineTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_ScalesAffineTransformScale4->text().toDouble();
  transformValues[5] = m_Controls.m_ScalesAffineTransformScale5->text().toDouble();
  transformValues[6] = m_Controls.m_ScalesAffineTransformScale6->text().toDouble();
  transformValues[7] = m_Controls.m_ScalesAffineTransformScale7->text().toDouble();
  transformValues[8] = m_Controls.m_ScalesAffineTransformScale8->text().toDouble();
  transformValues[9] = m_Controls.m_ScalesAffineTransformScale9->text().toDouble();
  transformValues[10] = m_Controls.m_ScalesAffineTransformScaleTranslationX->text().toDouble();
  transformValues[11] = m_Controls.m_ScalesAffineTransformScaleTranslationY->text().toDouble();
  transformValues[12] = m_Controls.m_ScalesAffineTransformScaleTranslationZ->text().toDouble();
  transformValues[13] = m_Controls.m_CenterForInitializerAffine->isChecked();
  transformValues[14] = m_Controls.m_MomentsAffine->isChecked();
  return transformValues;
}

void QmitkAffineTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesAffine->setChecked(transformValues[0]);
  m_Controls.m_ScalesAffineTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesAffineTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesAffineTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_ScalesAffineTransformScale4->setText(QString::number(transformValues[4]));
  m_Controls.m_ScalesAffineTransformScale5->setText(QString::number(transformValues[5]));
  m_Controls.m_ScalesAffineTransformScale6->setText(QString::number(transformValues[6]));
  m_Controls.m_ScalesAffineTransformScale7->setText(QString::number(transformValues[7]));
  m_Controls.m_ScalesAffineTransformScale8->setText(QString::number(transformValues[8]));
  m_Controls.m_ScalesAffineTransformScale9->setText(QString::number(transformValues[9]));
  m_Controls.m_ScalesAffineTransformScaleTranslationX->setText(QString::number(transformValues[10]));
  m_Controls.m_ScalesAffineTransformScaleTranslationY->setText(QString::number(transformValues[11]));
  m_Controls.m_ScalesAffineTransformScaleTranslationZ->setText(QString::number(transformValues[12]));
  m_Controls.m_CenterForInitializerAffine->setChecked(transformValues[13]);
  m_Controls.m_MomentsAffine->setChecked(transformValues[14]);
  m_Controls.m_GeometryAffine->setChecked(!transformValues[14]);
}

QString QmitkAffineTransformView::GetName()
{
  return "Affine";
}

void QmitkAffineTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesAffineTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScale9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScaleTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScaleTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesAffineTransformScaleTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkAffineTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(12);
  scales.Fill(1.0);
  scales[0] = m_Controls.m_ScalesAffineTransformScale1->text().toDouble();
  scales[1] = m_Controls.m_ScalesAffineTransformScale2->text().toDouble();
  scales[2] = m_Controls.m_ScalesAffineTransformScale3->text().toDouble();
  scales[3] = m_Controls.m_ScalesAffineTransformScale4->text().toDouble();
  scales[4] = m_Controls.m_ScalesAffineTransformScale5->text().toDouble();
  scales[5] = m_Controls.m_ScalesAffineTransformScale6->text().toDouble();
  scales[6] = m_Controls.m_ScalesAffineTransformScale7->text().toDouble();
  scales[7] = m_Controls.m_ScalesAffineTransformScale8->text().toDouble();
  scales[8] = m_Controls.m_ScalesAffineTransformScale9->text().toDouble();
  scales[9] = m_Controls.m_ScalesAffineTransformScaleTranslationX->text().toDouble();
  scales[10] = m_Controls.m_ScalesAffineTransformScaleTranslationY->text().toDouble();
  scales[11] = m_Controls.m_ScalesAffineTransformScaleTranslationZ->text().toDouble();
  return scales;
}

vtkTransform* QmitkAffineTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    // -  the 9 rotation-coefficients are copied
    //    directly to the top left part of the matrix
    int m = 0;
    for (int i = 0; i < m_FixedImage->GetDimension(); i++)
    {
      for (int j = 0; j < m_FixedImage->GetDimension(); j++)
      {
        vtkmatrix->SetElement(i, j, transformParams[m]);
        m++;
      }
    }

    // -  the 3 translation-coefficients are corrected to take
    //    into account the center of the transformation
    float center[4];
    center[0] = m_CenterX;
    center[1] = m_CenterY;
    center[2] = m_CenterZ;
    center[3] = 1;
    std::cout<< "rotation center: " << center[0] << " " << center[1] << " " << center [2] << std::endl;

    float translation[4];
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

    // set the transform matrix to init the transform
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkAffineTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
    {
      m_Controls.m_ScalesAffineTransformScale7->hide();
      m_Controls.m_ScalesAffineTransformScale8->hide();
      m_Controls.m_ScalesAffineTransformScale9->hide();
      m_Controls.m_ScalesAffineTransformScaleTranslationX->hide();
      m_Controls.m_ScalesAffineTransformScaleTranslationY->hide();
      m_Controls.m_ScalesAffineTransformScaleTranslationZ->hide();
      m_Controls.textLabel2_7->setText("Translation Scale X:");
      m_Controls.textLabel3_6->setText("Translation Scale Y:");
      m_Controls.textLabel4_4->hide();
      m_Controls.textLabel5_4->hide();
      m_Controls.textLabel6_4->hide();
      m_Controls.textLabel11_3->hide();
      m_Controls.textLabel12_3->hide();
      m_Controls.textLabel13_2->hide();
      return 6;
    }
    else
    {
      m_Controls.m_ScalesAffineTransformScale7->show();
      m_Controls.m_ScalesAffineTransformScale8->show();
      m_Controls.m_ScalesAffineTransformScale9->show();
      m_Controls.m_ScalesAffineTransformScaleTranslationX->show();
      m_Controls.m_ScalesAffineTransformScaleTranslationY->show();
      m_Controls.m_ScalesAffineTransformScaleTranslationZ->show();
      m_Controls.textLabel2_7->setText("Scale 5:");
      m_Controls.textLabel3_6->setText("Scale 6:");
      m_Controls.textLabel4_4->show();
      m_Controls.textLabel5_4->show();
      m_Controls.textLabel6_4->show();
      m_Controls.textLabel11_3->show();
      m_Controls.textLabel12_3->show();
      m_Controls.textLabel13_2->show();
      return 12;
    }
  }
  else 
    return 0;
}
