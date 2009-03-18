/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

#ifndef QMITKHISTOGRAMVALUESWIDGET_H_
#define QMITKHISTOGRAMVALUESWIDGET_H_

#include <qlayout.h>
#include <qtextedit.h>
#include <qdialog.h>
#include <qwt_plot.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_picker.h>

#include "QmitkHistogram.h"
#include "mitkImage.h"

#include <itkHistogram.h>

/** 
 \brief Shows a histogram in a Qwt_Plot and a report (selectable) in a QTextEdit
 Makes the data Values from a itk::histogram suitable for a Qwt_Plot.
 Offers zoom possibility. 
 A marker can be set and shows the frequency at his position.
 */
class QMITK_EXPORT QmitkHistogramValuesWidget: public QDialog
{
Q_OBJECT

public:

  //overloaded constructor to choose between showing only the histogram or the histogram and the report
  QmitkHistogramValuesWidget(QWidget *parent = 0, bool showreport = false);
  virtual ~QmitkHistogramValuesWidget();

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

  void OnSelect( const QwtDoublePoint &pos );

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

#endif /* QMITKHISTOGRAMVALUESWIDGET_H_ */

