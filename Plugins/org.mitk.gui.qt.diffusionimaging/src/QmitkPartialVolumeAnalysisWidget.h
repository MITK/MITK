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

#ifndef QmitkPartialVolumeAnalysisWidget_H_
#define QmitkPartialVolumeAnalysisWidget_H_

#include "QmitkPlotWidget.h"
#include <org_mitk_gui_qt_diffusionimaging_Export.h>

#include "QmitkHistogram.h"
#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"
#include "mitkPartialVolumeAnalysisClusteringCalculator.h"

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
class DIFFUSIONIMAGING_EXPORT QmitkPartialVolumeAnalysisWidget : public QmitkPlotWidget
{
  Q_OBJECT

public:
  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  typedef mitk::PartialVolumeAnalysisClusteringCalculator ClusteringType;
  typedef ClusteringType::ParamsType ParamsType;
  typedef ClusteringType::ClusterResultType ResultsType;
  typedef ClusteringType::HistType HistType;

  /** \brief Set histogram to be displayed directly. */
  void SetParameters( ParamsType *params, ResultsType *results, HistType *hist );

  QmitkPartialVolumeAnalysisWidget( QWidget * /*parent = 0 */);
  virtual ~QmitkPartialVolumeAnalysisWidget();

  void DrawGauss();

  void ClearItemModel();

  std::vector< std::vector<double>* > m_Vals;
};

#endif /* QmitkPartialVolumeAnalysisWidget_H_ */
