/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef QMITKTRANSFERFUNCTIONGENERATORWIDGET_H
#define QMITKTRANSFERFUNCTIONGENERATORWIDGET_H

#include "MitkQtWidgetsExtExports.h"
#include "ui_QmitkTransferFunctionGeneratorWidget.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

class MITKQTWIDGETSEXT_EXPORT QmitkTransferFunctionGeneratorWidget : public QWidget,
                                                                     public Ui::QmitkTransferFunctionGeneratorWidget
{
  Q_OBJECT

public:
  QmitkTransferFunctionGeneratorWidget(QWidget *parent = nullptr, Qt::WindowFlags f = nullptr);
  ~QmitkTransferFunctionGeneratorWidget() override;

  void SetDataNode(mitk::DataNode *node);

  int AddPreset(const QString &presetName);

  void SetPresetsTabEnabled(bool enable);
  void SetThresholdTabEnabled(bool enable);
  void SetBellTabEnabled(bool enable);

public slots:

  void OnSavePreset();
  void OnLoadPreset();

  void OnDeltaLevelWindow(int dx, int dy);
  void OnDeltaThreshold(int dx, int dy);

signals:

  void SignalTransferFunctionModeChanged(int);
  void SignalUpdateCanvas();

protected slots:

  void OnPreset(int mode);

protected:
  mitk::TransferFunctionProperty::Pointer tfpToChange;

  double histoMinimum;
  double histoMaximum;

  double thPos;
  double thDelta;

  double deltaScale;
  double deltaMax;
  double deltaMin;

  const mitk::Image::HistogramType *histoGramm;

  QString presetFileName;

  double ScaleDelta(int d) const;
};

#endif
