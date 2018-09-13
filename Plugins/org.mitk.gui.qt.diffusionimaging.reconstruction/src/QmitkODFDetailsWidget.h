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

#ifndef QmitkODFDetailsWidget_H_
#define QmitkODFDetailsWidget_H_

#include "QmitkPlotWidget.h"
#include <org_mitk_gui_qt_diffusionimaging_reconstruction_Export.h>
#include <itkOrientationDistributionFunction.h>


/**
 * \brief Widget for displaying image histograms based on the vtkQtChart
 * framework
 */
class DIFFUSIONIMAGING_RECONSTRUCTION_EXPORT QmitkODFDetailsWidget : public QmitkPlotWidget
{
Q_OBJECT

public:

  template< unsigned int ODF_SIZE=ODF_SAMPLING_SIZE >
  void SetParameters( itk::OrientationDistributionFunction<float, ODF_SIZE> odf )
  {
    this->Clear();

    std::vector<double> xVals;
    std::vector<double> yVals;
    double max = itk::NumericTraits<double>::NonpositiveMin();
    double min = itk::NumericTraits<double>::max();

    for (unsigned int i=0; i<ODF_SIZE; i++){
      xVals.push_back(i);
      yVals.push_back(odf[i]);
      if (odf[i]>max)
        max = odf[i];
      if (odf[i]<min)
        min = odf[i];
    }

    if(min>0)
      m_Plot->setAxisScale ( 0, 0, max );
    else
      m_Plot->setAxisScale ( 0, min, max );

    auto curveId = this->InsertCurve( "ODF Values" );
    this->SetCurveData( curveId, xVals, yVals );
    this->SetCurvePen( curveId, QPen(Qt::blue, 0.5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin) );
    this->Replot();
  }

  QmitkODFDetailsWidget( QWidget * /*parent = 0 */);
  virtual ~QmitkODFDetailsWidget();

  std::vector<double> m_Vals;

private:

};

#endif /* QmitkODFDetailsWidget_H_ */
