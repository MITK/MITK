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

   void SetIntegerMode(bool intMode);

   void SetScalarLabel(const QString& scalarLabel);

   void ShowScalarOpacityFunction(bool show);
   void ShowColorFunction(bool show);
   void ShowGradientOpacityFunction(bool show);

   void SetScalarOpacityFunctionEnabled(bool enable);
   void SetColorFunctionEnabled(bool enable);
   void SetGradientOpacityFunctionEnabled(bool enable);

  public slots:

    void SetXValueScalar();
    void SetYValueScalar();
    void SetXValueGradient();
    void SetYValueGradient();
    void SetXValueColor();

    void OnUpdateCanvas();
    void UpdateRanges();
    void OnResetSlider();

   void OnSpanChanged (double lower, double upper);


  protected:
  
    mitk::TransferFunctionProperty::Pointer tfpToChange;
    
    double m_RangeSliderMin;
    double m_RangeSliderMax;

    mitk::SimpleHistogramCache histogramCache;
    
};
#endif
