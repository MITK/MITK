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


#ifndef QMITKHISTOGRAMJSWIDGET_H
#define QMITKHISTOGRAMJSWIDGET_H

#include <QWidget>
#include <QUrl>
#include <QtWebKit/QtWebKit>
#include "QmitkExtExports.h"
#include <QVariant>
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include <itkPolyLineParametricPath.h>

/**
* \brief Widget which shows a histogram using JavaScript.
*
* This class is a QWebView. It shows the histogram for a selected image
* or segmentation. It also can also display an intesity profile for
* path elements, which lay over an image.
*/
class QmitkExt_EXPORT QmitkHistogramJSWidget : public QWebView
{
  Q_OBJECT

  /**
  * \brief Measurement property.
  *
  * This property is used in JavaScript as member of current object.
  * @see GetMeasurement()
  */
  Q_PROPERTY(QList<QVariant> measurement
             READ GetMeasurement)

  /**
  * \brief Frequency property.
  *
  * This property is used in JavaScript as member of current object.
  * @see GetFrequency()
  */
  Q_PROPERTY(QList<QVariant> frequency
             READ GetFrequency)

  /**
  * \brief Line graph property.
  *
  * This property is used in JavaScript as member of current object.
  * @see GetUseLineGraph()
  */
  Q_PROPERTY(bool useLineGraph
             READ GetUseLineGraph)

  /**
  * @brief Intesity profile property.
  *
  * This property is used in JavaScript as member of current object.
  * @see GetIntensityProfile()
  */
  Q_PROPERTY(bool intensityProfile
             READ GetIntensityProfile)

public:
  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;
  typedef itk::PolyLineParametricPath< 3 > ParametricPathType;
  typedef itk::ParametricPath< 3 >::Superclass PathType;

  explicit QmitkHistogramJSWidget(QWidget *parent = 0);


  ~QmitkHistogramJSWidget();

  /**
  * \brief Event which notifies a change of the widget size.
  *
  * Reimplemented event from QWebView::resizeEvent(),
  * reloads the webframe
  */
  void resizeEvent(QResizeEvent* resizeEvent);

  /**
  * \brief Calculates the histogram.
  *
  * This function filters all frequencies of 0 to the first and after the last bin.
  * Writes the measurement and frequency, which are given from the HistogramType, into
  * m_Measurement and m_Frequency.
  * Calls the signal DataChanged to update the shown information in the webframe.
  */
  void ComputeHistogram(HistogramType* histogram);

  /**
  * \brief Calculates the intesityprofile.
  *
  * m_Image and m_PlanarFigure must be set first!
  * Calculates an intensity profile for a pathelement which lies over an image.
  * Sets m_IntensityProfile and m_UseLineGraph to true.
  * Calls SignalDataChanged to update the shown information in the webframe.
  */
  void ComputeIntensityProfile();

  /**
  * \brief Clears the Histogram.
  *
  * This function clears the data and calls the signal DataChanged to update
  * the shown information in the webframe.
  */
  void ClearHistogram();

  /**
  * \brief Get operation for measurement.
  *
  * @return List of measurements.
  */
  QList<QVariant> GetMeasurement();

  /**
  * \brief Get operation for frequency.
  *
  * @return List of frequencies.
  */
  QList<QVariant> GetFrequency();

  /**
  * \brief Get operation for uselineGraph.
  *
  * @return True if a linegraph should be used.
  */
  bool GetUseLineGraph();

  /**
  * \brief Get operation for intensity profile.
  *
  * @return True if current histogram is an intesityprofile
  */
  bool GetIntensityProfile();

  /**
  * \brief Set operation for reference image.
  *
  * @param image The corresponding image for an intensity profile.
  */
  void SetImage(mitk::Image* image);

  /**
  * \brief Set operation for  planarFigure.
  *
  * @param planarFigure The pathelement for an intensity profile.
  */
  void SetPlanarFigure(const mitk::PlanarFigure* planarFigure);

private:

  /**
  * \brief List of frequencies.
  *
  * A QList which holds the frequencies of the current histogram
  * or holds the intesities of current intensity profile.
  */
  QList<QVariant> m_Frequency;

  /**
  * \brief List of measurements.
  *
  * A QList which holds the measurements of the current histogram
  * or holds the distances of current intensity profile.
  */
  QList<QVariant> m_Measurement;

  /**
  * \brief Reference image.
  *
  * Holds the image to calculate an intesity profile.
  */
  mitk::Image::Pointer m_Image;

  /**
  * \brief Pathelement.
  *
  * Holds a not closed planar figure to calculate an intesity profile.
  */
  mitk::PlanarFigure::ConstPointer m_PlanarFigure;

  bool m_UseLineGraph;
  bool m_IntensityProfile;

  /**
  * Holds the current histogram
  */
  HistogramType::ConstPointer m_Histogram;

  /**
  * Path derived either form user-specified path or from PlanarFigure-generated
  * path
  */
  PathType::ConstPointer m_DerivedPath;

  /**
  * Parametric path as generated from PlanarFigure
  */
  ParametricPathType::Pointer m_ParametricPath;

  /**
  * \brief Clears data.
  *
  * Clears the QLists m_Measurement and m_Frequency
  */
  void ClearData();

private slots:

  /**
  * \brief Adds an object to JavaScript.
  *
  * Adds an object of the widget to JavaScript.
  * By using this object JavaScript can react to the signals of the widget
  * and can access the QProperties as members of the object.
  */
  void AddJSObject();

public slots:

  /**
  * \brief Slot for radiobutton m_barRadioButton.
  *
  * Sets m_UseLineGraph to false.
  * Calls signal GraphChanged to update the graph in the webframe.
  */
  void OnBarRadioButtonSelected();

  /**
  * \brief Slot for radiobutton m_lineRadioButton.
  *
  * Sets m_UseLineGraph to true.
  * Calls signal GraphChanged to update the graph in the webframe.
  */
  void OnLineRadioButtonSelected();

signals:

  /**
  * \brief Signal data has changed.
  *
  * It has to be called when the data of the histogram or intesity profile has changed.
  */
  void SignalDataChanged();

  /**
  * \brief Signal graph has changed.
  *
  * It has to be called when the graph changed from barchart to linegraph. Vice versa.
  */
  void SignalGraphChanged();
};

#endif // QMITKHISTOGRAMJSWIDGET_H
