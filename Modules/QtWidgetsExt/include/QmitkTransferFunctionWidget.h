/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKTRANSFERFUNCTIONWIDGET_H
#define QMITKTRANSFERFUNCTIONWIDGET_H

#include "MitkQtWidgetsExtExports.h"
#include "ui_QmitkTransferFunctionWidget.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

#include <QPushButton>
#include <QSlider>

#include <QmitkTransferFunctionWidget.h>

namespace mitk
{
  class BaseRenderer;
}

class MITKQTWIDGETSEXT_EXPORT QmitkTransferFunctionWidget : public QWidget, public Ui::QmitkTransferFunctionWidget
{
  Q_OBJECT

public:
  QmitkTransferFunctionWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkTransferFunctionWidget() override;

  void SetDataNode(mitk::DataNode *node, const mitk::BaseRenderer *renderer = nullptr);

  void SetScalarLabel(const QString &scalarLabel);

  void ShowScalarOpacityFunction(bool show);
  void ShowColorFunction(bool show);
  void ShowGradientOpacityFunction(bool show);

  void SetScalarOpacityFunctionEnabled(bool enable);
  void SetColorFunctionEnabled(bool enable);
  void SetGradientOpacityFunctionEnabled(bool enable);

public slots:

  void SetXValueScalar(const QString text);
  void SetYValueScalar(const QString text);
  void SetXValueGradient(const QString text);
  void SetYValueGradient(const QString text);
  void SetXValueColor(const QString text);

  void OnUpdateCanvas();
  void UpdateRanges();
  void OnResetSlider();

  void OnSpanChanged(int lower, int upper);

protected:
  mitk::TransferFunctionProperty::Pointer tfpToChange;

  int m_RangeSliderMin;
  int m_RangeSliderMax;

  mitk::SimpleHistogramCache histogramCache;
};

#endif
