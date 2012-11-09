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

#ifndef QmitkResidualAnalysisWidget_H_
#define QmitkResidualAnalysisWidget_H_

#include "QmitkPlotWidget.h"

#include <org_mitk_gui_qt_diffusionimaging_Export.h>

//#include "QmitkHistogram.h"
#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include "itkVectorImage.h"


//#include <itkHistogram.h>


#include <QPainter>
#include <qwt_plot_picker.h>



/**
 * \brief Widget for displaying boxplots
 * framework
 */
class DIFFUSIONIMAGING_EXPORT QmitkResidualAnalysisWidget : public QmitkPlotWidget
{

Q_OBJECT

public:


  QmitkResidualAnalysisWidget( QWidget * parent);
  virtual ~QmitkResidualAnalysisWidget();




  QwtPlot* GetPlot()
  {
    return m_Plot;
  }

  QwtPlotPicker* m_PlotPicker;


  void SetMeans(std::vector< double > means)
  {
    m_Means = means;
  }

  void SetQ1(std::vector< double > q1)
  {
    m_Q1 = q1;
  }

  void SetQ3(std::vector< double > q3)
  {
    m_Q3 = q3;
  }

  void SetPercentagesOfOutliers(std::vector< double > perc)
  {
    m_PercentagesOfOutliers = perc;
  }


  void DrawMeans();
  void DrawPercentagesOfOutliers();


protected:
  std::vector< double > m_Means;
  std::vector< double > m_Q1;
  std::vector< double > m_Q3;
  std::vector< double > m_PercentagesOfOutliers;
};

#endif
