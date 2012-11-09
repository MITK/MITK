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

#ifndef QmitkNetworkHistogramCanvas_H_INCLUDED
#define QmitkNetworkHistogramCanvas_H_INCLUDED


#include <mitkConnectomicsHistogramBase.h>
#include <mitkRenderingManager.h>

#include "QmitkPlotWidget.h"

#include <vtkQtBarChart.h>
#include <QStandardItemModel>
#include <qwt_plot.h>
#include <QPainter>
#include <qwt_plot_picker.h>

  //##Documentation
  //## @brief A widget for displaying the data in a ConnectomicsHistogramBase

class QmitkNetworkHistogramCanvas : public QmitkPlotWidget
{

  Q_OBJECT

public:

  QmitkNetworkHistogramCanvas( QWidget * parent=0, Qt::WindowFlags f = 0 );
  ~QmitkNetworkHistogramCanvas();

   /** @brief Returns the histogram the canvas is using  */
  mitk::ConnectomicsHistogramBase* GetHistogram()
  {
    return m_Histogram;
  }

  /** @brief Set the histogram the canvas is to use  */
  void SetHistogram(mitk::ConnectomicsHistogramBase *histogram)
  {
    m_Histogram = histogram;
  }

  /** @brief Draw the histogram  */
  void DrawProfiles();

  /** @brief Return the plot */
  QwtPlot* GetPlot()
  {
    return m_Plot;
  }


protected:

  /** @brief The histogram to be plotted  */
  mitk::ConnectomicsHistogramBase *m_Histogram;

};
#endif

