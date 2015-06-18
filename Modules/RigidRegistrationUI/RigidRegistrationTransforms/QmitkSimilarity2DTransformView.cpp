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

#include "QmitkSimilarity2DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <mitkImageCast.h>
#include <itkSimilarity2DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkSimilarity2DTransformView::QmitkSimilarity2DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f),
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkSimilarity2DTransformView::~QmitkSimilarity2DTransformView()
{
}

mitk::TransformParameters::TransformType QmitkSimilarity2DTransformView::GetTransformType()
{
  return mitk::TransformParameters::SIMILARITY2DTRANSFORM;
}

itk::Object::Pointer QmitkSimilarity2DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessFixedDimensionByItk(m_FixedImage, GetTransform2, 2);
    return m_TransformObject;
  }
  return nullptr;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkSimilarity2DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 2)
  {
    typedef typename itk::Image< TPixelType, 2 >  FixedImage2DType;
    typedef typename itk::Image< TPixelType, 2 >  MovingImage2DType;

    // the fixedImage is the input parameter (fix for Bug #14626)
    typename FixedImage2DType::Pointer fixedImage2D = itkImage1;

    // the movingImage type is known, use the ImageToItk filter (fix for Bug #14626)
    typename mitk::ImageToItk<MovingImage2DType>::Pointer movingImageToItk =
        mitk::ImageToItk<MovingImage2DType>::New();
    movingImageToItk->SetInput(m_MovingImage);
    movingImageToItk->Update();
    typename MovingImage2DType::Pointer movingImage2D = movingImageToItk->GetOutput();


    typename itk::Similarity2DTransform< double >::Pointer transformPointer = itk::Similarity2DTransform< double >::New();
    transformPointer->SetIdentity();
    if (m_Controls.m_CenterForInitializerSimilarity2D->isChecked())
    {
      typedef typename itk::Similarity2DTransform< double > Similarity2DTransformType;
      typedef typename itk::CenteredTransformInitializer<Similarity2DTransformType, FixedImage2DType, MovingImage2DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage2D );
      transformInitializer->SetMovingImage( movingImage2D );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsSimilarity2D->isChecked())
      {
        transformInitializer->MomentsOn();
      }
      else
      {
        transformInitializer->GeometryOn();
      }
      transformInitializer->InitializeTransform();
    }
    transformPointer->SetScale( m_Controls.m_InitialScaleSimilarity2D->text().toFloat() );
    transformPointer->SetAngle( m_Controls.m_AngleSimilarity2D->text().toFloat() );
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }
  return nullptr;
}

itk::Array<double> QmitkSimilarity2DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(9);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesSimilarity2D->isChecked();
  transformValues[1] = m_Controls.m_ScalingScaleSimilarity2D->text().toDouble();
  transformValues[2] = m_Controls.m_RotationScaleSimilarity2D->text().toDouble();
  transformValues[3] = m_Controls.m_TranslationXScaleSimilarity2D->text().toDouble();
  transformValues[4] = m_Controls.m_TranslationYScaleSimilarity2D->text().toDouble();
  transformValues[5] = m_Controls.m_InitialScaleSimilarity2D->text().toFloat();
  transformValues[6] = m_Controls.m_AngleSimilarity2D->text().toFloat();
  transformValues[7] = m_Controls.m_CenterForInitializerSimilarity2D->isChecked();
  transformValues[8] = m_Controls.m_MomentsSimilarity2D->isChecked();
  return transformValues;
}

void QmitkSimilarity2DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesSimilarity2D->setChecked(transformValues[0]);
  m_Controls.m_ScalingScaleSimilarity2D->setText(QString::number(transformValues[1]));
  m_Controls.m_RotationScaleSimilarity2D->setText(QString::number(transformValues[2]));
  m_Controls.m_TranslationXScaleSimilarity2D->setText(QString::number(transformValues[3]));
  m_Controls.m_TranslationYScaleSimilarity2D->setText(QString::number(transformValues[4]));
  m_Controls.m_InitialScaleSimilarity2D->setText(QString::number(transformValues[5]));
  m_Controls.m_AngleSimilarity2D->setText(QString::number(transformValues[6]));
  m_Controls.m_CenterForInitializerSimilarity2D->setChecked(transformValues[7]);
  m_Controls.m_MomentsSimilarity2D->setChecked(transformValues[8]);
  m_Controls.m_GeometrySimilarity2D->setChecked(!transformValues[8]);
}

QString QmitkSimilarity2DTransformView::GetName()
{
  return "Similarity2D";
}

void QmitkSimilarity2DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalingScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_RotationScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationXScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationYScaleSimilarity2D->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkSimilarity2DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(4);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesSimilarity2D->isChecked())
  {
    scales[0] = m_Controls.m_ScalingScaleSimilarity2D->text().toDouble();
    scales[1] = m_Controls.m_RotationScaleSimilarity2D->text().toDouble();
    scales[2] = m_Controls.m_TranslationXScaleSimilarity2D->text().toDouble();
    scales[3] = m_Controls.m_TranslationYScaleSimilarity2D->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkSimilarity2DTransformView::Transform(vtkMatrix4x4* /* vtkmatrix */, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    mitk::ScalarType angle = transformParams[1] * 45.0 / atan(1.0);
    vtktransform->PostMultiply();
    vtktransform->Scale(transformParams[0], transformParams[0], 1);
    vtktransform->RotateZ(angle);
    vtktransform->Translate(transformParams[2], transformParams[3], 0);
    vtktransform->PreMultiply();
  }
  return vtktransform;
}

int QmitkSimilarity2DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 4;
    else
      return 0;
  }
  else
    return 0;
}
