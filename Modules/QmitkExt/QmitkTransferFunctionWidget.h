/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QMITKTRANSFERFUNCTIONWIDGET_H
#define QMITKTRANSFERFUNCTIONWIDGET_H

#include "ui_QmitkTransferFunctionWidget.h"
#include "QmitkExtExports.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

#include <QSlider>
#include <QPushButton>

#include <QmitkTransferFunctionWidget.h>


class QmitkExt_EXPORT QmitkTransferFunctionWidget : public QWidget, public Ui::QmitkTransferFunctionWidget
{

  Q_OBJECT

  public:

    QmitkTransferFunctionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkTransferFunctionWidget () ;

	  void SetDataNode(mitk::DataNode* node);

  public slots:

	  void SetXValueScalar();
	  void SetYValueScalar();
	  void SetXValueGradient();
	  void SetYValueGradient();
	  void SetXValueColor();
	 
	  void OnUpdateCanvas();
	  void UpdateRanges();
	  void OnResetSlider();
	  
	  void OnSpanChanged (int lower, int upper);
	  

  protected:
  
    mitk::TransferFunctionProperty::Pointer tfpToChange;
    
    int m_RangeSliderMin;
    int m_RangeSliderMax;

    mitk::SimpleHistogramCache histogramCache;
    
};
#endif
