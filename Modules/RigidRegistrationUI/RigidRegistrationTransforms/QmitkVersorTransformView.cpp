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

#include "QmitkVersorTransformView.h"
#include "mitkImageAccessByItk.h"
#include <mitkImageCast.h>
#include <itkVersorTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkVersorTransformView::QmitkVersorTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f),
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkVersorTransformView::~QmitkVersorTransformView()
{
}

mitk::TransformParameters::TransformType QmitkVersorTransformView::GetTransformType()
{
  return mitk::TransformParameters::VERSORTRANSFORM;
}

itk::Object::Pointer QmitkVersorTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessFixedDimensionByItk(m_FixedImage, GetTransform2, 3);
    return m_TransformObject;
  }
  return nullptr;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkVersorTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 3)
  {
    typedef typename itk::Image< TPixelType, 3 >  FixedImage3DType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImage3DType;

    // the fixedImage is the input parameter (fix for Bug #14626)
    typename FixedImage3DType::Pointer fixedImage = itkImage1;

    // the movingImage type is known, use the ImageToItk filter (fix for Bug #14626)
    typename mitk::ImageToItk<MovingImage3DType>::Pointer movingImageToItk = mitk::ImageToItk<MovingImage3DType>::New();
    movingImageToItk->SetInput(m_MovingImage);
    movingImageToItk->Update();
    typename MovingImage3DType::Pointer movingImage = movingImageToItk->GetOutput();

    typename itk::VersorTransform< double >::Pointer transformPointer = itk::VersorTransform< double >::New();
    transformPointer->SetIdentity();
    typedef typename itk::VersorTransform< double >    VersorTransformType;
    if (m_Controls.m_CenterForInitializerVersor->isChecked())
    {
      typedef typename itk::CenteredTransformInitializer<VersorTransformType, FixedImage3DType, MovingImage3DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage );
      transformInitializer->SetMovingImage( movingImage );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsVersor->isChecked())
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

itk::Array<double> QmitkVersorTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(6);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesVersor->isChecked();
  transformValues[1] = m_Controls.m_ScalesVersorTransformScale1->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesVersorTransformScale2->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesVersorTransformScale3->text().toDouble();
  transformValues[4] = m_Controls.m_CenterForInitializerVersor->isChecked();
  transformValues[5] = m_Controls.m_MomentsVersor->isChecked();
  return transformValues;
}

void QmitkVersorTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesVersor->setChecked(transformValues[0]);
  m_Controls.m_ScalesVersorTransformScale1->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesVersorTransformScale2->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesVersorTransformScale3->setText(QString::number(transformValues[3]));
  m_Controls.m_CenterForInitializerVersor->setChecked(transformValues[4]);
  m_Controls.m_MomentsVersor->setChecked(transformValues[5]);
  m_Controls.m_GeometryVersor->setChecked(!transformValues[5]);
}

QString QmitkVersorTransformView::GetName()
{
  return "Versor";
}

void QmitkVersorTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesVersorTransformScale1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorTransformScale2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesVersorTransformScale3->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkVersorTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(3);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesVersor->isChecked())
  {
    scales[0] = m_Controls.m_ScalesVersorTransformScale1->text().toDouble();
    scales[1] = m_Controls.m_ScalesVersorTransformScale2->text().toDouble();
    scales[2] = m_Controls.m_ScalesVersorTransformScale3->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkVersorTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    itk::VersorTransform<double>::Pointer versorTransform = itk::VersorTransform<double>::New();
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
    vtkmatrix->SetElement(0, 3, -translation[0] + center[0]);
    vtkmatrix->SetElement(1, 3, -translation[1] + center[1]);
    vtkmatrix->SetElement(2, 3, -translation[2] + center[2]);
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkVersorTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 0;
    else
      return 3;
  }
  else
    return 0;
}
