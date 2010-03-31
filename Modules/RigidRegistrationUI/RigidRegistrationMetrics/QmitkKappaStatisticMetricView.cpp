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

#include "QmitkKappaStatisticMetricView.h"
#include <itkKappaStatisticImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

QmitkKappaStatisticMetricView::QmitkKappaStatisticMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{

}

QmitkKappaStatisticMetricView::~QmitkKappaStatisticMetricView()
{
}

itk::Object::Pointer QmitkKappaStatisticMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkKappaStatisticMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::KappaStatisticImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::KappaStatisticImageToImageMetric<FixedImageType, MovingImageType>::New();
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkKappaStatisticMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(1);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  return metricValues;
}

void QmitkKappaStatisticMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
}

QString QmitkKappaStatisticMetricView::GetName()
{
  return "KappaStatistic";
}

void QmitkKappaStatisticMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
}
