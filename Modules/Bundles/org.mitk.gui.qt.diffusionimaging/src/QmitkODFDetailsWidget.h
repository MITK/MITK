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
#include <org_mitk_gui_qt_diffusionimaging_Export.h>
#include <itkOrientationDistributionFunction.h>


/**
 * \brief Widget for displaying image histograms based on the vtkQtChart
 * framework
 */
class DIFFUSIONIMAGING_EXPORT QmitkODFDetailsWidget : public QmitkPlotWidget
{
Q_OBJECT

public:

  void SetParameters( itk::OrientationDistributionFunction<double, QBALL_ODFSIZE> odf );

  QmitkODFDetailsWidget( QWidget * /*parent = 0 */);
  virtual ~QmitkODFDetailsWidget();

  std::vector<double> m_Vals;

private:

};

#endif /* QmitkODFDetailsWidget_H_ */
