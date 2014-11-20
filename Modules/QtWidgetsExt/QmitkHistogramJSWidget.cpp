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


#include "QmitkHistogramJSWidget.h"
#include "mitkPixelTypeMultiplex.h"
#include <mitkImagePixelReadAccessor.h>
#include <mitkIntensityProfile.h>
#include "mitkRenderingManager.h"
#include "mitkBaseRenderer.h"
#include "mitkImageTimeSelector.h"
#include "mitkExtractSliceFilter.h"
#include <QWebFrame>


QmitkHistogramJSWidget::QmitkHistogramJSWidget(QWidget *parent) :
  QWebView(parent)
{
  // set histogram type to barchart in first instance
  m_UseLineGraph = false;
  m_Page = new QmitkJSWebPage(this);
  setPage(m_Page);
  // set html from source
  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(AddJSObject()));
  QUrl myUrl = QUrl("qrc:///QtWidgetsExt/Histogram.html");
  setUrl(myUrl);

  // set Scrollbars to be always disabled
  page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
  page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

  m_ParametricPath = ParametricPathType::New();

}

QmitkHistogramJSWidget::~QmitkHistogramJSWidget()
{
}

// adds an Object of Type QmitkHistogramJSWidget to the JavaScript, using QtWebkitBridge
void QmitkHistogramJSWidget::AddJSObject()
{
  page()->mainFrame()->addToJavaScriptWindowObject(QString("histogramData"), this);
}

// reloads WebView, everytime its size has been changed, so the size of the Histogram fits to the size of the widget
void QmitkHistogramJSWidget::resizeEvent(QResizeEvent* resizeEvent)
{
  QWebView::resizeEvent(resizeEvent);

  // workaround for Qt Bug: https://bugs.webkit.org/show_bug.cgi?id=75984
  page()->mainFrame()->evaluateJavaScript("disconnectSignals()");
  this->reload();
}

// method to expose data to JavaScript by using properties
void QmitkHistogramJSWidget::ComputeHistogram(HistogramType* histogram)
{
  m_Histogram = histogram;
  HistogramConstIteratorType startIt = m_Histogram->End();
  HistogramConstIteratorType endIt = m_Histogram->End();
  HistogramConstIteratorType it  = m_Histogram->Begin();
  ClearData();
  unsigned int i = 0;
  bool firstValue = false;
  // removes frequencies of 0, which are outside the first and last bin
  for (; it != m_Histogram->End(); ++it)
  {
    if (it.GetFrequency() > 0.0)
    {
      endIt = it;
      if (!firstValue)
      {
        firstValue = true;
        startIt = it;
      }
    }
  }
  ++endIt;
  // generating Lists of measurement and frequencies
  for (it = startIt ; it != endIt; ++it, ++i)
  {
    QVariant frequency = QVariant::fromValue(it.GetFrequency());
    QVariant measurement = it.GetMeasurementVector()[0];
    m_Frequency.insert(i, frequency);
    m_Measurement.insert(i, measurement);
  }
  m_IntensityProfile = false;
  this->SignalDataChanged();
}

void QmitkHistogramJSWidget::ClearData()
{
  m_Frequency.clear();
  m_Measurement.clear();
}

void QmitkHistogramJSWidget::ClearHistogram()
{
  this->ClearData();
  this->SignalDataChanged();
}

QList<QVariant> QmitkHistogramJSWidget::GetFrequency()
{
  return m_Frequency;
}

QList<QVariant> QmitkHistogramJSWidget::GetMeasurement()
{
  return m_Measurement;
}

bool QmitkHistogramJSWidget::GetUseLineGraph()
{
  return m_UseLineGraph;
}

void QmitkHistogramJSWidget::OnBarRadioButtonSelected()
{
  if (m_UseLineGraph)
  {
    m_UseLineGraph = false;
    this->SignalGraphChanged();
  }
}

void QmitkHistogramJSWidget::OnLineRadioButtonSelected()
{
  if (!m_UseLineGraph)
  {
    m_UseLineGraph = true;
    this->SignalGraphChanged();
  }
}

void QmitkHistogramJSWidget::SetImage(mitk::Image* image)
{
  m_Image = image;
}

void QmitkHistogramJSWidget::SetPlanarFigure(const mitk::PlanarFigure* planarFigure)
{
  m_PlanarFigure = planarFigure;
}

template <class PixelType>
void ReadPixel(mitk::PixelType, mitk::Image::Pointer image, itk::Index<3> indexPoint, double& value)
{
  if (image->GetDimension() == 2)
  {
    mitk::ImagePixelReadAccessor<PixelType,2> readAccess(image, image->GetSliceData(0));
    itk::Index<2> idx;
    idx[0] = indexPoint[0];
    idx[1] = indexPoint[1];
    value = readAccess.GetPixelByIndex(idx);
  }
  else if (image->GetDimension() == 3)
  {
    mitk::ImagePixelReadAccessor<PixelType,3> readAccess(image, image->GetVolumeData(0));
    itk::Index<3> idx;
    idx[0] = indexPoint[0];
    idx[1] = indexPoint[1];
    idx[2] = indexPoint[2];
    value = readAccess.GetPixelByIndex(idx);
  }
  else
  {
    //unhandled
  }
}

void QmitkHistogramJSWidget::ComputeIntensityProfile(unsigned int timeStep)
{
  this->ClearData();
  m_ParametricPath->Initialize();

  if (m_PlanarFigure.IsNull())
  {
    mitkThrow() << "PlanarFigure not set!";
  }

  if (m_Image.IsNull())
  {
    mitkThrow() << "Image not set!";
  }

  mitk::Image::Pointer image;

  if (m_Image->GetDimension() == 4)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(m_Image);
    timeSelector->SetTimeNr(timeStep);
    timeSelector->Update();

    image = timeSelector->GetOutput();
  }
  else
  {
    image = m_Image;
  }

  mitk::IntensityProfile::Pointer intensityProfile = mitk::ComputeIntensityProfile(image, const_cast<mitk::PlanarFigure*>(m_PlanarFigure.GetPointer()));

  m_Frequency.clear();
  m_Measurement.clear();

  int i = -1;
  mitk::IntensityProfile::ConstIterator end = intensityProfile->End();

  for (mitk::IntensityProfile::ConstIterator it = intensityProfile->Begin(); it != end; ++it)
  {
    m_Frequency.push_back(it.GetMeasurementVector()[0]);
    m_Measurement.push_back(++i);
  }

  m_IntensityProfile = true;
  m_UseLineGraph = true;
  this->SignalDataChanged();
}

bool QmitkHistogramJSWidget::GetIntensityProfile()
{
  return m_IntensityProfile;
}

