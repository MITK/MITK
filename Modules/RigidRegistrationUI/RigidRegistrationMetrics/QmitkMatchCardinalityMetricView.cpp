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

#include "QmitkMatchCardinalityMetricView.h"
#include <itkMatchCardinalityImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

QmitkMatchCardinalityMetricView::QmitkMatchCardinalityMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{

}

QmitkMatchCardinalityMetricView::~QmitkMatchCardinalityMetricView()
{
}

itk::Object::Pointer QmitkMatchCardinalityMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMatchCardinalityMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::MatchCardinalityImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MatchCardinalityImageToImageMetric<FixedImageType, MovingImageType>::New();
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMatchCardinalityMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(1);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  return metricValues;
}

void QmitkMatchCardinalityMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
}

QString QmitkMatchCardinalityMetricView::GetName()
{
  return "MatchCardinality";
}

void QmitkMatchCardinalityMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
}
