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

#include <QmitkC3jsWidget.h>
#include <QGridLayout>
#include <QWebChannel>
#include <QWebEngineView>

#include <QmitkC3Data.h>

#include <mitkIntensityProfile.h>

#include "mitkImageTimeSelector.h"

class QmitkC3jsWidget::Impl final
{
public:
  explicit Impl(QWidget* parent);
  ~Impl();

  Impl(const Impl&) = delete;
  Impl& operator=(const Impl&) = delete;

  QWebChannel* GetWebChannel();

  void ClearJavaScriptChart();
  void initializeJavaScriptChart();
  void callJavaScriptFuntion(QString command);

  QmitkC3Data* GetC3Data() { return &m_c3Data; };

  mitk::Image::Pointer GetImage() { return m_Image; }; const
  void SetImage(const mitk::Image::Pointer image) { m_Image = image; };

  mitk::PlanarFigure::ConstPointer GetPlanarFigure() { return m_PlanarFigure; }; const
  void SetPlanarFigure(const mitk::PlanarFigure::ConstPointer planarFigure) { m_PlanarFigure = planarFigure; };

private:
  QWidget* m_Parent;
  QWebChannel* m_WebChannel;
  QWebEngineView* m_WebEngineView;

  QmitkC3Data m_c3Data;

  /**
  * \brief Reference image.
  *
  * Holds the image to calculate an intensity profile.
  */
  mitk::Image::Pointer m_Image;

  /**
  * \brief Pathelement.
  *
  * Holds a not closed planar figure to calculate an intensity profile.
  */
  mitk::PlanarFigure::ConstPointer m_PlanarFigure;
};

QmitkC3jsWidget::Impl::Impl(QWidget* parent)
  : m_WebChannel(new QWebChannel(parent)),
  m_WebEngineView(new QWebEngineView(parent)),
  m_Parent(parent)
{
  //disable context menu for QWebEngineView
  m_WebEngineView->setContextMenuPolicy(Qt::NoContextMenu);

  //Set the webengineview to an initial empty page. The actual chart will be loaded once the data is calculated.
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
  m_WebEngineView->page()->setWebChannel(m_WebChannel);

  connect( m_WebEngineView, SIGNAL( loadFinished(bool) ), parent, SLOT( OnLoadFinished(bool) ) );

  auto layout = new QGridLayout(parent);
  layout->setMargin(0);
  layout->addWidget(m_WebEngineView);

  parent->setLayout(layout);
}

QmitkC3jsWidget::Impl::~Impl()
{
}

QWebChannel* QmitkC3jsWidget::Impl::GetWebChannel()
{
  return m_WebChannel;
}

QmitkC3jsWidget::QmitkC3jsWidget(QWidget* parent)
  : QWidget(parent),
  m_Impl(new Impl(this))
{
  m_Statistics = mitk::ImageStatisticsCalculator::StatisticsContainer::New();
}

void QmitkC3jsWidget::Impl::callJavaScriptFuntion(QString command)
{
  m_WebEngineView->page()->runJavaScript(command);
}

void QmitkC3jsWidget::Impl::ClearJavaScriptChart()
{
  m_WebEngineView->setUrl(QUrl(QStringLiteral("qrc:///C3js/empty.html")));
}

void QmitkC3jsWidget::Impl::initializeJavaScriptChart()
{
  m_WebChannel->registerObject(QStringLiteral("initValues"), &m_c3Data);
  m_WebEngineView->load(QUrl(QStringLiteral("qrc:///C3js/QmitkC3jsWidget.html")));
}

QmitkC3jsWidget::QmitkC3jsWidget(const QString& id, QObject* object, QWidget* parent)
  : QWidget(parent),
  m_Impl(new Impl(this))
{
  if (!id.isEmpty() && object != nullptr)
    m_Impl->GetWebChannel()->registerObject(id, object);

  m_Statistics = mitk::ImageStatisticsCalculator::StatisticsContainer::New();
}

QmitkC3jsWidget::~QmitkC3jsWidget()
{
  delete m_Impl;
}

void QmitkC3jsWidget::OnLoadFinished(bool isLoadSuccessfull)
{
  emit PageSuccessfullyLoaded();
}

void QmitkC3jsWidget::TransformView(QString transformTo)
{
  QString command = QString("transformView('" + transformTo + "')");
  m_Impl->callJavaScriptFuntion(command);
}

void QmitkC3jsWidget::SendCommand(QString command)
{
  m_Impl->callJavaScriptFuntion(command);
}

void QmitkC3jsWidget::SetAppearance(bool useLineChart, bool showSubChart)
{
  this->m_Impl->GetC3Data()->SetAppearance(useLineChart, showSubChart);
}

// method to expose data to JavaScript by using properties
void QmitkC3jsWidget::ComputeHistogram(HistogramType* histogram, bool useLineChart, bool showSubChart)
{
  this->m_Impl->GetC3Data()->SetHistogram(histogram);

  SetAppearance(useLineChart, showSubChart);
  HistogramConstIteratorType startIt = this->m_Impl->GetC3Data()->GetHistogram()->End();
  HistogramConstIteratorType endIt = this->m_Impl->GetC3Data()->GetHistogram()->End();
  HistogramConstIteratorType it = this->m_Impl->GetC3Data()->GetHistogram()->Begin();



  //Clear old data befor loading new data.
  this->m_Impl->GetC3Data()->ClearData();

  unsigned int i = 0;
  bool firstValue = false;
  // removes frequencies of 0, which are outside the first and last bin
  for (; it != this->m_Impl->GetC3Data()->GetHistogram()->End(); ++it)
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
  for (it = startIt; it != endIt; ++it, ++i)
  {
    QVariant frequency = QVariant::fromValue(it.GetFrequency());
    QVariant measurement = it.GetMeasurementVector()[0];
    this->m_Impl->GetC3Data()->GetYDataPointer()->insert(i, frequency);
    this->m_Impl->GetC3Data()->GetXDataPointer()->insert(i, measurement);
  }

  m_Impl->initializeJavaScriptChart();
}

void QmitkC3jsWidget::ComputeIntensityProfile(unsigned int timeStep, bool computeStatistics)
{
  this->ClearHistogram();
  //m_Impl->GetC3Data()->ClearData();
  //m_ParametricPath->Initialize();

  if (m_Impl->GetPlanarFigure().IsNull())
  {
    mitkThrow() << "PlanarFigure not set!";
  }

  if (m_Impl->GetImage().IsNull())
  {
    mitkThrow() << "Image not set!";
  }

  mitk::Image::Pointer image;

  if (m_Impl->GetImage()->GetDimension() == 4)
  {
    mitk::ImageTimeSelector::Pointer timeSelector = mitk::ImageTimeSelector::New();
    timeSelector->SetInput(m_Impl->GetImage());
    timeSelector->SetTimeNr(timeStep);
    timeSelector->Update();

    image = timeSelector->GetOutput();
  }
  else
  {
    image = m_Impl->GetImage();
  }

  mitk::IntensityProfile::Pointer intensityProfile = mitk::ComputeIntensityProfile(
    image, const_cast<mitk::PlanarFigure*>(m_Impl->GetPlanarFigure().GetPointer()));

  //m_Frequency.clear();
  //m_Measurement.clear();

  int i = -1;
  mitk::IntensityProfile::ConstIterator end = intensityProfile->End();

  for (mitk::IntensityProfile::ConstIterator it = intensityProfile->Begin(); it != end; ++it)
  {
    m_Impl->GetC3Data()->GetYDataPointer()->push_back(it.GetMeasurementVector()[0]);
    //m_Impl->GetC3Data()->GetFrequencyPointer()->push_back(50000);
    m_Impl->GetC3Data()->GetXDataPointer()->push_back(++i);
  }

  if (computeStatistics)
  {
    mitk::ComputeIntensityProfileStatistics(intensityProfile, m_Statistics);
  }

  m_Impl->initializeJavaScriptChart();  
}

void QmitkC3jsWidget::ClearHistogram()
{
  m_Impl->GetC3Data()->ClearData();
  m_Impl->ClearJavaScriptChart();
}

mitk::Image::Pointer QmitkC3jsWidget::GetImage() const 
{ 
  return m_Impl->GetImage();
}

void QmitkC3jsWidget::SetImage(const mitk::Image::Pointer image) 
{ 
  m_Impl->SetImage(image); 
}

mitk::PlanarFigure::ConstPointer QmitkC3jsWidget::GetPlanarFigure() const
{ 
  return m_Impl->GetPlanarFigure(); 
}

void QmitkC3jsWidget::SetPlanarFigure(const mitk::PlanarFigure::ConstPointer planarFigure) 
{ 
  m_Impl->SetPlanarFigure(planarFigure); 
}