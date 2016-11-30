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
#ifndef QmitkD3jsWidget_h
#define QmitkD3jsWidget_h

#include <QWidget>
#include <QVariant>

#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include <MitkC3jsExports.h>
#include <mitkImageStatisticsCalculator.h>

class MITKC3JS_EXPORT QmitkC3jsWidget : public QWidget
{
  Q_OBJECT

public:
  explicit QmitkC3jsWidget(QWidget* parent = nullptr);
  QmitkC3jsWidget(const QString& id, QObject* object, QWidget* parent = nullptr);
  virtual ~QmitkC3jsWidget();

  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  /**
  * \brief Calculates the histogram.
  *
  * This function removes all frequencies of 0 until the first bin and behind the last bin.
  * It writes the measurement and frequency, which are given from the HistogramType, into
  * m_Measurement and m_Frequency.
  * initializeJavaScriptChart is then called, to update the information, which is displayed in the webframe.
  */
  void ComputeHistogram(HistogramType* histogram, bool useLineChart, bool showSubChart);

  void ComputeIntensityProfile(unsigned int timeStep, bool computeStatistics);

  void ClearHistogram();

  mitk::Image::Pointer GetImage() const;
  void SetImage(const mitk::Image::Pointer image);

  mitk::PlanarFigure::ConstPointer GetPlanarFigure() const;
  void SetPlanarFigure(const mitk::PlanarFigure::ConstPointer planarFigure);

  void SetAppearance(bool useLineChart, bool showSubChart);

  void SendCommand(QString command);
  
  void TransformView(QString transformTo);

  mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer GetStatistics()
  {
    return m_Statistics;
  };


private:
  class Impl;
  Impl* m_Impl;

  mitk::ImageStatisticsCalculator::StatisticsContainer::Pointer m_Statistics;

public slots:
  void OnLoadFinished(bool isLoadSuccessfull);

signals:
  void PageSuccessfullyLoaded();
};

#endif
