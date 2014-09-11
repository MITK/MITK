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

#ifndef QMITKTRANSFERFUNCTIONGENERATORWIDGET_H
#define QMITKTRANSFERFUNCTIONGENERATORWIDGET_H

#include "ui_QmitkTransferFunctionGeneratorWidget.h"
#include "MitkQtWidgetsExtExports.h"

#include <mitkCommon.h>

#include <QWidget>

#include <mitkDataNode.h>
#include <mitkTransferFunctionProperty.h>

class MitkQtWidgetsExt_EXPORT QmitkTransferFunctionGeneratorWidget : public QWidget, public Ui::QmitkTransferFunctionGeneratorWidget
{

  Q_OBJECT

  public:

    QmitkTransferFunctionGeneratorWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    ~QmitkTransferFunctionGeneratorWidget () ;

    void SetDataNode(mitk::DataNode* node);

    int AddPreset(const QString& presetName);

    void SetPresetsTabEnabled(bool enable);
    void SetThresholdTabEnabled(bool enable);
    void SetBellTabEnabled(bool enable);

  public slots:

    void OnSavePreset( );
    void OnLoadPreset( );

    void OnDeltaLevelWindow( int dx, int dy );
    void OnDeltaThreshold( int dx, int dy );

  signals:

    void SignalTransferFunctionModeChanged( int );
    void SignalUpdateCanvas();

  protected slots:

    void OnPreset( int mode );

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

    std::string ReduceFileName(std::string fileNameLong );

    double ScaleDelta(int d) const;

};

#endif

