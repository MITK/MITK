/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkEuler2DTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkEuler2DTransform.h>
#include <itkCenteredTransformInitializer.h>
#include <QValidator>

QmitkEuler2DTransformView::QmitkEuler2DTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f), 
m_CenterX(0), m_CenterY(0), m_CenterZ(0)
{
}

QmitkEuler2DTransformView::~QmitkEuler2DTransformView()
{
}

itk::Object::Pointer QmitkEuler2DTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkEuler2DTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* /* itkImage1 */)
{
  if (VImageDimension == 2)
  {
    typedef typename itk::Image< TPixelType, 2 >  FixedImage2DType;
    typedef typename itk::Image< TPixelType, 2 >  MovingImage2DType;
    typename FixedImage2DType::Pointer fixedImage2D;
    mitk::CastToItkImage(m_FixedImage, fixedImage2D);
    typename MovingImage2DType::Pointer movingImage2D;
    mitk::CastToItkImage(m_MovingImage, movingImage2D);
    typename itk::Euler2DTransform< double >::Pointer transformPointer = itk::Euler2DTransform< double >::New();
    transformPointer->SetIdentity();
    if (m_Controls.m_CenterForInitializerEuler2D->isChecked())
    {
      typedef typename itk::Euler2DTransform< double > Euler2DTransformType;
      typedef typename itk::CenteredTransformInitializer<Euler2DTransformType, FixedImage2DType, MovingImage2DType> TransformInitializerType;
      typename TransformInitializerType::Pointer transformInitializer = TransformInitializerType::New();
      transformInitializer->SetFixedImage( fixedImage2D );
      transformInitializer->SetMovingImage( movingImage2D );
      transformInitializer->SetTransform( transformPointer );
      if (m_Controls.m_MomentsEuler2D->isChecked())
      {
        transformInitializer->MomentsOn();
      }
      else
      {
        transformInitializer->GeometryOn();
      }
      transformInitializer->InitializeTransform();        
    }
    m_TransformObject = transformPointer.GetPointer();
    return transformPointer.GetPointer();
  }  
  return NULL;
}

itk::Array<double> QmitkEuler2DTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(6);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesEuler2D->isChecked();
  transformValues[1] = m_Controls.m_RotationScaleEuler2D->text().toDouble();
  transformValues[2] = m_Controls.m_TranslationXScaleEuler2D->text().toDouble();
  transformValues[3] = m_Controls.m_TranslationYScaleEuler2D->text().toDouble();
  transformValues[4] = m_Controls.m_CenterForInitializerEuler2D->isChecked();
  transformValues[5] = m_Controls.m_MomentsEuler2D->isChecked();
  return transformValues;
}

void QmitkEuler2DTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesEuler2D->setChecked(transformValues[0]);
  m_Controls.m_RotationScaleEuler2D->setText(QString::number(transformValues[1]));
  m_Controls.m_TranslationXScaleEuler2D->setText(QString::number(transformValues[2]));
  m_Controls.m_TranslationYScaleEuler2D->setText(QString::number(transformValues[3]));
  m_Controls.m_CenterForInitializerEuler2D->setChecked(transformValues[4]);
  m_Controls.m_MomentsEuler2D->setChecked(transformValues[5]);
  m_Controls.m_GeometryEuler2D->setChecked(!transformValues[5]);
}

QString QmitkEuler2DTransformView::GetName()
{
  return "Euler2D";
}

void QmitkEuler2DTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_RotationScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationXScaleEuler2D->setValidator(validatorLineEditInputFloat);
  m_Controls.m_TranslationYScaleEuler2D->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkEuler2DTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(3);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesEuler2D->isChecked())
  {
    scales[0] = m_Controls.m_RotationScaleEuler2D->text().toDouble();
    scales[1] = m_Controls.m_TranslationXScaleEuler2D->text().toDouble();
    scales[2] = m_Controls.m_TranslationYScaleEuler2D->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkEuler2DTransformView::Transform(vtkMatrix4x4* /*vtkmatrix*/, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    mitk::ScalarType angle = transformParams[0] * 45.0 / atan(1.0);
    vtktransform->PostMultiply();
    vtktransform->RotateZ(angle);
    vtktransform->Translate(transformParams[1], transformParams[2], 0);
    vtktransform->PreMultiply();
  }
  return vtktransform;
}

int QmitkEuler2DTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
      return 3;
    else
      return 0;
  }
  else
    return 0;
}
