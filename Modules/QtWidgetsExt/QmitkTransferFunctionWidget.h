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

#ifndef QMITKTRANSFERFUNCTIONWIDGET_H
#define QMITKTRANSFERFUNCTIONWIDGET_H

#include "ui_QmitkTransferFunctionWidget.h"
#include "MitkQtWidgetsExtExports.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

#include <QSlider>
#include <QPushButton>

#include <QmitkTransferFunctionWidget.h>

namespace mitk {
class BaseRenderer;
}

class MitkQtWidgetsExt_EXPORT QmitkTransferFunctionWidget : public QWidget, public Ui::QmitkTransferFunctionWidget
{

  Q_OBJECT

  public:

    QmitkTransferFunctionWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkTransferFunctionWidget () ;

    void SetDataNode(mitk::DataNode* node, const mitk::BaseRenderer* renderer = NULL);

   void SetScalarLabel(const QString& scalarLabel);

   void ShowScalarOpacityFunction(bool show);
   void ShowColorFunction(bool show);
   void ShowGradientOpacityFunction(bool show);

   void SetScalarOpacityFunctionEnabled(bool enable);
   void SetColorFunctionEnabled(bool enable);
   void SetGradientOpacityFunctionEnabled(bool enable);

  public slots:

    void SetXValueScalar( const QString text );
    void SetYValueScalar( const QString text );
    void SetXValueGradient( const QString text );
    void SetYValueGradient( const QString text );
    void SetXValueColor( const QString text );

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

