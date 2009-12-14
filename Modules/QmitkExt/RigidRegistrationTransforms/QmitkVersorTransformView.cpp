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

#include "QmitkVersorTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkVersorTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <qvalidator>

QmitkVersorTransformView::QmitkVersorTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkVersorTransformView::~QmitkVersorTransformView()
{
}

itk::Object::Pointer QmitkVersorTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkVersorTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  if (VImageDimension == 3)
  {
    typedef typename itk::Image< TPixelType, 3 >  FixedImage3DType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImage3DType;
    typename FixedImage3DType::Pointer fixedImage3D;
    mitk::CastToItkImage(m_FixedImage, fixedImage3D);
    typename MovingImage3DType::Pointer movingImage3D;
    mitk::CastToItkImage(m_MovingImage, movingImage3D);
    typename itk::VersorTransform< double >::Pointer transformPointer = itk::VersorTransform< double >::New();
    transformPointer->SetIdentity();
    typedef typename itk::VersorTransform< double >    VersorTransformType;
    if (m_Controls.m_CenterForInitializerVersor->isChecked())
    {
      typedef typename itk::CenteredTransformInitializer<VersorTransformType, FixedImage3DType, MovingImage3DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage3D );
      transformInitializer->SetMovingImage( movingImage3D );
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
  return NULL;
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
