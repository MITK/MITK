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


#ifndef QMITKKURTOSISWIDGET_H
#define QMITKKURTOSISWIDGET_H

#include "QmitkPlotWidget.h"

#include "itkDiffusionKurtosisReconstructionImageFilter.h"


class QmitkKurtosisWidget : public QmitkPlotWidget
{
public:

  typedef itk::DiffusionKurtosisReconstructionImageFilter<short, float> KurtosisFilterType;

  QmitkKurtosisWidget( QWidget* /* parent */);
  virtual ~QmitkKurtosisWidget();

  void SetData( KurtosisFilterType::KurtosisSnapshot snap );

  std::vector< std::vector<double>* > m_Vals;

private:

  std::vector<double> toStdVec(const vnl_vector<double>& vector);

};

#endif // QMITKKURTOSISWIDGET_H
