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

QmitkHistogramJSWidget::QmitkHistogramJSWidget(QWidget *parent) :
  QWebView(parent)
{
  // set histogram to barchart in first instance
  m_UseLineGraph = false;

  // prepare html for use
  connect(page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(addJSObject()));
  QUrl myUrl = QUrl("qrc:/qmitk/Histogram.html");
  setUrl(myUrl);

  // set Scrollbars to always disabled
  page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
  page()->mainFrame()->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);

  m_ParametricPath = ParametricPathType::New();
}

QmitkHistogramJSWidget::~QmitkHistogramJSWidget()
{

}

// adds an Object of Type QmitkHistogramJSWidget to the JavaScript
void QmitkHistogramJSWidget::addJSObject()
{
  page()->mainFrame()->addToJavaScriptWindowObject(QString("histogramData"), this);
}


// reloads WebView, everytime his size has been changed, so the size of the Histogram fits to the size of the widget
void QmitkHistogramJSWidget::resizeEvent(QResizeEvent* resizeEvent)
{
  QWebView::resizeEvent(resizeEvent);
  this->reload();
}

// method to expose data to JavaScript by using properties
void QmitkHistogramJSWidget::ComputeHistogram(HistogramType* histogram)
{
  m_Histogram = histogram;
  HistogramConstIteratorType startIt = m_Histogram->End();
  HistogramConstIteratorType endIt = m_Histogram->End();
  HistogramConstIteratorType it;
  clearData();
  unsigned int i = 0;
  bool firstValue = false;
  // filter frequencies of 0 to guarantee a better view while using a mask
  for (it = m_Histogram->Begin() ; it != m_Histogram->End(); ++it)
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
  for (it = startIt ; it != endIt; ++it, ++i)
  {
    QVariant frequency = it.GetFrequency();
    QVariant measurement = it.GetMeasurementVector()[0];
    m_Frequency.insert(i, frequency);
    m_Measurement.insert(i, measurement);
  }
  m_IntensityProfile = false;
  this->DataChanged();
}

void QmitkHistogramJSWidget::clearData()
{
  m_Frequency.clear();
  m_Measurement.clear();
}

void QmitkHistogramJSWidget::clearHistogram()
{
  this->clearData();
  this->DataChanged();
}

QList<QVariant> QmitkHistogramJSWidget::getFrequency()
{
  return m_Frequency;
}

QList<QVariant> QmitkHistogramJSWidget::getMeasurement()
{
  return m_Measurement;
}

bool QmitkHistogramJSWidget::getUseLineGraph()
{
  return m_UseLineGraph;
}

// slots for radiobuttons
void QmitkHistogramJSWidget::histogramToBarChart()
{
  m_UseLineGraph = false;
  this->GraphChanged();
}

void QmitkHistogramJSWidget::histogramToLineGraph()
{
  m_UseLineGraph = true;
  this->GraphChanged();
}

void QmitkHistogramJSWidget::setImage(mitk::Image* image)
{
  m_Image = image;
}

void QmitkHistogramJSWidget::setPlanarFigure(const mitk::PlanarFigure* planarFigure)
{
  m_PlanarFigure = planarFigure;
}

void QmitkHistogramJSWidget::ComputeIntensityProfile()
{
  this->clearData();
  m_ParametricPath->Initialize();

  if (m_PlanarFigure.IsNull())
  {
    throw std::invalid_argument("PlanarFigure not set!");
  }

  if (m_Image.IsNull())
  {
    throw std::invalid_argument("Image not set!");
  }

  mitk::Geometry2D* planarFigureGeometry2D = dynamic_cast<mitk::Geometry2D*>(m_PlanarFigure->GetGeometry(0));
  if (planarFigureGeometry2D == NULL)
  {
    throw std::invalid_argument("PlanarFigure has no valid geometry!");
  }

  mitk::Geometry3D* imageGeometry = m_Image->GetGeometry(0);
  if (imageGeometry == NULL)
  {
    throw std::invalid_argument("Image has no valid geometry!");
  }

  typedef mitk::PlanarFigure::PolyLineType VertexContainerType;
  const VertexContainerType vertexContainer = m_PlanarFigure->GetPolyLine(0);

  VertexContainerType::const_iterator it;
  for (it = vertexContainer.begin(); it != vertexContainer.end(); ++it)
  {
    mitk::Point3D point3D;
    planarFigureGeometry2D->Map(it->Point, point3D);

    mitk::Point3D indexPoint3D;
    imageGeometry->WorldToIndex(point3D, indexPoint3D);

    ParametricPathType::OutputType index;
    index[0] = indexPoint3D[0];
    index[1] = indexPoint3D[1];
    index[2] = indexPoint3D[2];

    m_ParametricPath->AddVertex(index);
  }

  m_DerivedPath = m_ParametricPath;

  if (m_DerivedPath.IsNull())
  {
    throw std::invalid_argument("No path set!");
  }

  double distance = 0.0;
  mitk::Point3D currentWorldPoint;

  double t;
  unsigned int i = 0;
  int k = 0;
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
    double intensity = m_Image->GetPixelValueByIndex(indexPoint);

    m_Measurement.insert(i, distance);
    m_Frequency.insert(i, intensity);

    PathType::OffsetType offset = m_DerivedPath->IncrementInput(t);
    if (!(offset[0] || offset[1] || offset[2]))
    {
      break;
    }

    currentWorldPoint = worldPoint;
  }

  m_IntensityProfile = true;
  m_UseLineGraph = true;
  this->DataChanged();
}

bool QmitkHistogramJSWidget::getIntensityProfile()
{
  return m_IntensityProfile;
}
