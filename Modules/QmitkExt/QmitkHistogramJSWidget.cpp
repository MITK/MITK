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
#include "mitkRenderingManager.h"
#include "mitkBaseRenderer.h"
#include "mitkImageTimeSelector.h"
#include "mitkExtractSliceFilter.h"


QmitkHistogramJSWidget::QmitkHistogramJSWidget(QWidget *parent) :
  QWebView(parent)
{
  // set histogram type to barchart in first instance
  m_UseLineGraph = false;
  m_Page = new QmitkJSWebPage(this);
  setPage(m_Page);
  // set html from source
  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(AddJSObject()));
  QUrl myUrl = QUrl("qrc:/qmitk/Histogram.html");
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
void ReadPixel(mitk::PixelType ptype, mitk::Image::Pointer image, mitk::Index3D indexPoint, double& value)
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

  // Get 2D geometry frame of PlanarFigure
  mitk::Geometry2D* planarFigureGeometry2D = dynamic_cast<mitk::Geometry2D*>(m_PlanarFigure->GetGeometry(0));
  if (planarFigureGeometry2D == NULL)
  {
    mitkThrow() << "PlanarFigure has no valid geometry!";
  }

  // Get 3D geometry from Image (needed for conversion of point to index)
  mitk::Geometry3D* imageGeometry = m_Image->GetGeometry(0);
  if (imageGeometry == NULL)
  {
    mitkThrow() << "Image has no valid geometry!";
  }

  // Get first poly-line of PlanarFigure (other possible poly-lines in PlanarFigure
  // are not supported)
  const VertexContainerType vertexContainer = m_PlanarFigure->GetPolyLine(0);

  VertexContainerType::const_iterator it;
  for (it = vertexContainer.begin(); it != vertexContainer.end(); ++it)
  {
    // Map PlanarFigure 2D point to 3D point
    mitk::Point3D point3D;
    planarFigureGeometry2D->Map(it->Point, point3D);

    // Convert world to index coordinates
    mitk::Point3D indexPoint3D;
    imageGeometry->WorldToIndex(point3D, indexPoint3D);

    ParametricPathType::OutputType index;
    index[0] = indexPoint3D[0];
    index[1] = indexPoint3D[1];
    index[2] = indexPoint3D[2];

    // Add index to parametric path
    m_ParametricPath->AddVertex(index);
  }

  m_DerivedPath = m_ParametricPath;

  if (m_DerivedPath.IsNull())
  {
    mitkThrow() << "No path set!";
  }

  // Fill item model with line profile data
  double distance = 0.0;
  mitk::Point3D currentWorldPoint;

  double t;
  unsigned int i = 0;
  for (i = 0, t = m_DerivedPath->StartOfInput(); ;++i)
  {
    const PathType::OutputType &continousIndex = m_DerivedPath->Evaluate(t);

    mitk::Point3D worldPoint;
    imageGeometry->IndexToWorld(continousIndex, worldPoint);

    if (i == 0)
    {
      currentWorldPoint = worldPoint;
    }


    distance += currentWorldPoint.EuclideanDistanceTo(worldPoint);
    mitk::Index3D indexPoint;
    imageGeometry->WorldToIndex(worldPoint, indexPoint);
    const mitk::PixelType ptype = m_Image->GetPixelType();
    double intensity = 0.0;
    if (m_Image->GetDimension() == 4)
    {
      mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
      timeSelector->SetInput(m_Image);
      timeSelector->SetTimeNr(timeStep);
      timeSelector->Update();
      mitk::Image::Pointer image = timeSelector->GetOutput();
      mitkPixelTypeMultiplex3( ReadPixel, ptype, image, indexPoint, intensity);
    }
    else
    {
      mitkPixelTypeMultiplex3( ReadPixel, ptype, m_Image, indexPoint, intensity);
    }

    m_Measurement.insert(i, distance);
    m_Frequency.insert(i, intensity);

    // Go to next index; when iteration offset reaches zero, iteration is finished
    PathType::OffsetType offset = m_DerivedPath->IncrementInput(t);
    if (!(offset[0] || offset[1] || offset[2]))
    {
      break;
    }

    currentWorldPoint = worldPoint;
  }

  m_IntensityProfile = true;
  m_UseLineGraph = true;
  this->SignalDataChanged();
}

bool QmitkHistogramJSWidget::GetIntensityProfile()
{
  return m_IntensityProfile;
}
