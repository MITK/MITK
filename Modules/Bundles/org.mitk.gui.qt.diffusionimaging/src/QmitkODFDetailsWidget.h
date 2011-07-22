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

#ifndef QmitkODFDetailsWidget_H_
#define QmitkODFDetailsWidget_H_

#include "QmitkPlotWidget.h"
#include "MitkDiffusionImagingExports.h"

#include "QmitkHistogram.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"

#include <qlayout.h>
#include <qtextedit.h>
#include <qdialog.h>

#include <itkHistogram.h>

#include <vtkQtChartWidget.h>
#include <vtkQtBarChart.h>

#include <QStandardItemModel>


/**
 * \brief Widget for displaying image histograms based on the vtkQtChart
 * framework
 */
class QmitkODFDetailsWidget : public QmitkPlotWidget
{
Q_OBJECT

public:

  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  void SetParameters( std::vector<double> odfVals );

  QmitkODFDetailsWidget( QWidget * /*parent = 0 */);
  virtual ~QmitkODFDetailsWidget();

  std::vector<double> m_Vals;

private:

  std::vector<double> vec(vnl_vector<double> vector);

};

#endif /* QmitkODFDetailsWidget_H_ */
