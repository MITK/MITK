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

#ifndef QmitkIVIMWidget_H_
#define QmitkIVIMWidget_H_

#include "QmitkPlotWidget.h"

#include "QmitkHistogram.h"
#include "QmitkExtExports.h"
#include "mitkImage.h"
#include "mitkPlanarFigure.h"

#include <qlayout.h>
#include <qtextedit.h>
#include <qdialog.h>

#include <itkHistogram.h>

#include <vtkQtChartWidget.h>
#include <vtkQtBarChart.h>

#include <QStandardItemModel>

#include "itkDiffusionIntravoxelIncoherentMotionReconstructionImageFilter.h"


/**
 * \brief Widget for displaying image histograms based on the vtkQtChart
 * framework
 */
class QmitkIVIMWidget : public QmitkPlotWidget
{

public:

  typedef itk::DiffusionIntravoxelIncoherentMotionReconstructionImageFilter<short, short> IVIMFilterType;

  typedef mitk::Image::HistogramType HistogramType;
  typedef mitk::Image::HistogramType::ConstIterator HistogramConstIteratorType;

  void SetParameters( IVIMFilterType::IVIMSnapshot snap );

  QmitkIVIMWidget( QWidget * /*parent = 0 */);
  virtual ~QmitkIVIMWidget();

  void DrawGauss();

  void ClearItemModel();

  std::vector< std::vector<double>* > m_Vals;

private:

  std::vector<double> vec(vnl_vector<double> vector);

};

#endif /* QmitkIVIMWidget_H_ */
