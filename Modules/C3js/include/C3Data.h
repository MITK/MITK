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

#ifndef QmitkC3Data_h
#define QmitkC3Data_h

#include <QmitkC3jsWidget.h>

/** /brief This class holds the relevant data for the chart generation with C3.
* Documentation: This class holds the relevant data for the chart generation with C3. It forwards the frequencies and the measurements to JavaScript.
* Those are then displayed in the histogram chart.
* It is derived from QObject, because we need Q_PROPERTIES to send Data via QWebChannel to JavaScript.
*/
class QmitkC3Data : public QObject
{
  Q_OBJECT

    typedef mitk::Image::HistogramType HistogramType;
  Q_PROPERTY(QList<QVariant> m_Frequency READ GetFrequency WRITE SetFrequency NOTIFY OnFrequencyChaned);
  Q_PROPERTY(QList<QVariant> m_Measurement READ GetMeasurement WRITE SetMeasurement NOTIFY OnMeasurementChaned);

public:
  Q_INVOKABLE QList<QVariant> GetFrequency() { return m_Frequency; };
  Q_INVOKABLE void SetFrequency(QList<QVariant> frequency) { m_Frequency = frequency; };

  Q_INVOKABLE QList<QVariant> GetMeasurement() { return m_Measurement; };
  Q_INVOKABLE void SetMeasurement(QList<QVariant> measurement) { m_Measurement = measurement; };

  /**
  * \brief Clears the Data.
  *
  * This function clears the data.
  */
  void ClearData();

  HistogramType::ConstPointer GetHistogram() { return m_Histogram; };
  void SetHistogram(HistogramType::ConstPointer histogram) { m_Histogram = histogram; };

  //QList<QVariant> GetFrequency() { return m_Frequency; };
  QList<QVariant>* GetFrequencyPointer() { return &m_Frequency; };
  //void SetFrequency(QList<QVariant> frequency) { m_Frequency = frequency; };

  //QList<QVariant> GetMeasurement() { return m_Frequency; };
  QList<QVariant>* GetMeasurementPointer() { return &m_Measurement; };
  //void SetMeasurement(QList<QVariant> measurement) { m_Measurement = measurement; };

signals:
  void OnFrequencyChaned(const QList<QVariant> frequency);
  void OnMeasurementChaned(const QList<QVariant> measurement);

private:
  /**
  * Holds the current histogram
  */
  HistogramType::ConstPointer m_Histogram;

  /**
  * \brief List of frequencies.
  *
  * A QList which holds the frequencies of the current histogram
  * or holds the intensities of current intensity profile.
  */
  QList<QVariant> m_Frequency;

  /**
  * \brief List of measurements.
  *
  * A QList which holds the measurements of the current histogram
  * or holds the distances of current intensity profile.
  */
  QList<QVariant> m_Measurement;
};

#endif  //QmitkC3Data_h
