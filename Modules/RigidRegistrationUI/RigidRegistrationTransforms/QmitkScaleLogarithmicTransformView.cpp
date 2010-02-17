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

#include "QmitkScaleLogarithmicTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkScaleLogarithmicTransform.h>
#include <QValidator>

QmitkScaleLogarithmicTransformView::QmitkScaleLogarithmicTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f)
{
}

QmitkScaleLogarithmicTransformView::~QmitkScaleLogarithmicTransformView()
{
}

itk::Object::Pointer QmitkScaleLogarithmicTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkScaleLogarithmicTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::ScaleLogarithmicTransform< double, VImageDimension>::Pointer transformPointer = itk::ScaleLogarithmicTransform< double, VImageDimension>::New();
  transformPointer->SetIdentity();
  m_TransformObject = transformPointer.GetPointer();
  return transformPointer.GetPointer();
}

itk::Array<double> QmitkScaleLogarithmicTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(4);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesScaleLogarithmic->isChecked();
  transformValues[1] = m_Controls.m_ScalesScaleLogarithmicTransformScaleX->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesScaleLogarithmicTransformScaleY->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->text().toDouble();
  return transformValues;
}

void QmitkScaleLogarithmicTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesScaleLogarithmic->setChecked(transformValues[0]);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleX->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesScaleLogarithmicTransformScaleY->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->setText(QString::number(transformValues[3]));
}

QString QmitkScaleLogarithmicTransformView::GetName()
{
  return "ScaleLogarithmic";
}

void QmitkScaleLogarithmicTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkScaleLogarithmicTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(3);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesScaleLogarithmic->isChecked())
  {
    scales[0] = m_Controls.m_ScalesScaleLogarithmicTransformScaleX->text().toDouble();
    scales[1] = m_Controls.m_ScalesScaleLogarithmicTransformScaleY->text().toDouble();
    scales[2] = m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkScaleLogarithmicTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    for( unsigned int i=0; i<transformParams.size(); i++)
    {
      vtkmatrix->SetElement(i, i, transformParams[i]);
    }
    vtktransform->SetMatrix(vtkmatrix);
  }
  return vtktransform;
}

int QmitkScaleLogarithmicTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
    {
      m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->hide();
      m_Controls.textLabel3_5_3->hide();
      return 2;
    }
    else
    {
      m_Controls.m_ScalesScaleLogarithmicTransformScaleZ->show();
      m_Controls.textLabel3_5_3->show();
      return 3;
    }
  }
  else 
    return 0;
}
