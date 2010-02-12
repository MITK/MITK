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

#include "QmitkScaleTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkScaleTransform.h>
#include <QValidator>

QmitkScaleTransformView::QmitkScaleTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f)
{
}

QmitkScaleTransformView::~QmitkScaleTransformView()
{
}

itk::Object::Pointer QmitkScaleTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkScaleTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::ScaleTransform< double, VImageDimension>::Pointer transformPointer = itk::ScaleTransform< double, VImageDimension>::New();
  transformPointer->SetIdentity();
  m_TransformObject = transformPointer.GetPointer();
  return transformPointer.GetPointer();
}

itk::Array<double> QmitkScaleTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(4);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesScale->isChecked();
  transformValues[1] = m_Controls.m_ScalesScaleTransformScaleX->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesScaleTransformScaleY->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesScaleTransformScaleZ->text().toDouble();
  return transformValues;
}

void QmitkScaleTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesScale->setChecked(transformValues[0]);
  m_Controls.m_ScalesScaleTransformScaleX->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesScaleTransformScaleY->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesScaleTransformScaleZ->setText(QString::number(transformValues[3]));
}

QString QmitkScaleTransformView::GetName()
{
  return "Scale";
}

void QmitkScaleTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesScaleTransformScaleX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleTransformScaleY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesScaleTransformScaleZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkScaleTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(3);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesScale->isChecked())
  {
    scales[0] = m_Controls.m_ScalesScaleTransformScaleX->text().toDouble();
    scales[1] = m_Controls.m_ScalesScaleTransformScaleY->text().toDouble();
    scales[2] = m_Controls.m_ScalesScaleTransformScaleZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkScaleTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
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

int QmitkScaleTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
    {
      m_Controls.m_ScalesScaleTransformScaleZ->hide();
      m_Controls.textLabel3_5->hide();
      return 2;
    }
    else
    {
      m_Controls.m_ScalesScaleTransformScaleZ->show();
      m_Controls.textLabel3_5->show();
      this->repaint();
      return 3;
    }
  }
  else 
    return 0;
}
