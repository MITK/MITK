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

#ifndef QMITKHistogramWidget_H_
#define QMITKHistogramWidget_H_

#include "MitkQtWidgetsExtExports.h"

#include <QDialog>

#include "mitkImage.h"

class QTextEdit;

class QwtPlot;
class QwtPlotMarker;
class QwtPlotPicker;
class QwtPlotZoomer;

class QmitkHistogram;


/**
 \brief Shows a histogram in a Qwt_Plot and a report (selectable) in a QTextEdit
 Makes the data Values from a itk::histogram suitable for a Qwt_Plot.
 Offers zoom possibility.
 A marker can be set and shows the frequency at his position.
 */
class MitkQtWidgetsExt_EXPORT QmitkHistogramWidget: public QDialog
{
  Q_OBJECT

public:

  //overloaded constructor to choose between showing only the histogram or the histogram and the report
  QmitkHistogramWidget(QWidget *parent = 0, bool showreport = false);
  virtual ~QmitkHistogramWidget();

  typedef mitk::Image::HistogramType   HistogramType;

  /**
   - gets the data values from either a itk histogram or a mitk image
   - copies them into data arrays suitable for qwt
   - creates a plot dialog, fill it with the data and shows it
   */
  void SetHistogram(HistogramType::ConstPointer histogram);
  void SetHistogram(mitk::Image* mitkImage);

  void SetReport(std::string report);

  double GetMarkerPosition();

protected slots:

  void OnSelect( const QPointF &pos );

protected:

  // convenience fct. for rounding doubles to integral numbers
  double Round(double val);

  void InitializeMarker();
  void InitializeZoomer();

  QwtPlot*        m_Plot;
  QTextEdit*      m_Textedit;
  QwtPlotMarker*  m_Marker;
  QwtPlotPicker*  m_Picker;
  QwtPlotZoomer*  m_Zoomer;
  QmitkHistogram* m_Histogram;
};

#endif

