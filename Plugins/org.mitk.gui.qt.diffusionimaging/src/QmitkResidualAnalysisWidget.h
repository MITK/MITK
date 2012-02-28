/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-15 18:09:46 +0200 (Fr, 15 Mai 2009) $
Version:   $Revision: 1.12 $
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
 
=========================================================================*/

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


  void DrawMeans();


protected:
  std::vector< double > m_Means;
  std::vector< double > m_Q1;
  std::vector< double > m_Q3;

};

#endif
