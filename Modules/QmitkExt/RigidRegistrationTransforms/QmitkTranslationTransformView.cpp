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

#include "QmitkTranslationTransformView.h"
#include "mitkImageAccessByItk.h"
#include <itkTranslationTransform.h>
#include <qvalidator>

QmitkTranslationTransformView::QmitkTranslationTransformView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationTransformsGUIBase(parent, f)
{
}

QmitkTranslationTransformView::~QmitkTranslationTransformView()
{
}

itk::Object::Pointer QmitkTranslationTransformView::GetTransform()
{
  if (m_FixedImage.IsNotNull())
  {
    AccessByItk(m_FixedImage, GetTransform2);
    return m_TransformObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkTranslationTransformView::GetTransform2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::TranslationTransform< double, VImageDimension>::Pointer TransformPointer = itk::TranslationTransform< double, VImageDimension>::New();
  TransformPointer->SetIdentity();
  m_TransformObject = TransformPointer.GetPointer();
  return TransformPointer.GetPointer();
}

itk::Array<double> QmitkTranslationTransformView::GetTransformParameters()
{
  itk::Array<double> transformValues;
  transformValues.SetSize(4);
  transformValues.fill(0);
  transformValues[0] = m_Controls.m_UseOptimizerScalesTranslation->isChecked();
  transformValues[1] = m_Controls.m_ScalesTranslationTransformTranslationX->text().toDouble();
  transformValues[2] = m_Controls.m_ScalesTranslationTransformTranslationY->text().toDouble();
  transformValues[3] = m_Controls.m_ScalesTranslationTransformTranslationZ->text().toDouble();
  return transformValues;
}

void QmitkTranslationTransformView::SetTransformParameters(itk::Array<double> transformValues)
{
  m_Controls.m_UseOptimizerScalesTranslation->setChecked(transformValues[0]);
  m_Controls.m_ScalesTranslationTransformTranslationX->setText(QString::number(transformValues[1]));
  m_Controls.m_ScalesTranslationTransformTranslationY->setText(QString::number(transformValues[2]));
  m_Controls.m_ScalesTranslationTransformTranslationZ->setText(QString::number(transformValues[3]));
}

QString QmitkTranslationTransformView::GetName()
{
  return "Translation";
}

void QmitkTranslationTransformView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ScalesTranslationTransformTranslationX->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesTranslationTransformTranslationY->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ScalesTranslationTransformTranslationZ->setValidator(validatorLineEditInputFloat);
}

itk::Array<double> QmitkTranslationTransformView::GetScales()
{
  itk::Array<double> scales;
  scales.SetSize(3);
  scales.Fill(1.0);
  if (m_Controls.m_UseOptimizerScalesTranslation->isChecked())
  {
    scales[0] = m_Controls.m_ScalesTranslationTransformTranslationX->text().toDouble();
    scales[1] = m_Controls.m_ScalesTranslationTransformTranslationY->text().toDouble();
    scales[2] = m_Controls.m_ScalesTranslationTransformTranslationZ->text().toDouble();
  }
  return scales;
}

vtkTransform* QmitkTranslationTransformView::Transform(vtkMatrix4x4* vtkmatrix, vtkTransform* vtktransform, itk::Array<double> transformParams)
{
  if (m_MovingImage.IsNotNull())
  {
    if (transformParams.size() == 2)
    {
      vtktransform->Translate(transformParams[0], transformParams[1], 0);
    }
    else if (transformParams.size() == 3)
    {
      vtktransform->Translate(transformParams[0], transformParams[1], transformParams[2]);
      std::cout<<"Translation is: "<<transformParams[0] << transformParams[1] << transformParams[2] << std::endl;
    }
  }
  return vtktransform;
}

int QmitkTranslationTransformView::GetNumberOfTransformParameters()
{
  if (m_FixedImage.IsNotNull())
  {
    if (m_FixedImage->GetDimension() == 2)
    {
      m_Controls.m_ScalesTranslationTransformTranslationZ->hide();
      m_Controls.textLabel4_4_2->hide();

      return 2;
    }
    else
    {
      m_Controls.m_ScalesTranslationTransformTranslationZ->show();
      m_Controls.textLabel4_4_2->show();
      
      return 3;
    }
  }
  else 
    return 0; 
}
